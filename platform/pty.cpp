#include "pty.hpp"
#include <string>                     // Llama a la librería string, ya que se necesitan usar diversos formatos de string a continuación, como wstring y funciones para manipular los strings
#include <chrono>
#include <iostream>
#include <cstdlib>                    // Necesario para std::system
#include <format>                     // Para formatear fechas en C++20

// En este código confieso que tuve que pedir ayuda a Claude, estuve 6 horas intentando averiguar por qué en Windows no funcionaba la ptyfunc(), caí en la locura

/* A continuación, ptyfunc() sirve para la ejecución de comandos a través de una pseudoconsola, una pseudoconsola, o también llamada PTY, es una consola/terminal que simula ser una terminal interactiva,
   una terminal interactiva es necesaria para que la respuesta de la base de datos muestre de forma gráfica las tablas usando los pipes |, guiones - y símbolos de suma +, con todo esto se puede ver de manera
   más genuina la información que proviene de la terminal como si fuera una terminal real, ya que si nada más mando los comandos ejecutados por el administrador por medio de la función sendquery() la respuesta
   de la base de datos muestra las respuestas de una manera horrible, ya que mysql automáticamente detecta cuando un comando se ejecuta desde una terminal interactiva o desde un programa, y cuando se ejecutan comandos
   desde un programa el mysql automáticamente envía las respuestas sin que se puedan ver "bonitas", así que la función ptyfunc() al simular una terminal, hace creer a mysql que realmente se encuentra el
   administrador en una terminal, y envía la respuesta de forma bonita, además no solo eso, sino ayuda a poder visualizar mejor datos como errores, o respuestas más específicas, y si hiciera todo eso con
   sendquery() sería demasiado complejo.

   Para que la función ptyfunc() se ejecute de manera correcta en tanto windows como en linux, tuve que definir dos variables de la misma función, una función específica para windows como otra específica para linux.

   En windows, las dependencias que necesita el programa incluyen a los archivos de mariadb (principalmente bin\mariadb.exe) , los cuales se encuentran ya adjuntados al compilar para windows por medio del archivo exe_compiler.sh

   En linux, solo necesita el comando mysql instalado en el sistema, pero eso no se cubrirá en este programa ni en el compilador compiler.sh, ya que no creo que hayan computadoras linux a instalarles este programa
   a parte de las de nosotros, pero ya nosotros tendríamos ese comandos instalado debido a que anteriormente tuvimos que haber montado el servidor MySQL */

#if defined(_WIN32)                   // Rama de compilación para Windows (usa ConPTY: la pseudoconsola nativa de Windows 10+)

/* ConPTY (HPCON, CreatePseudoConsole, ClosePseudoConsole,
   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE) está declarado en los encabezados de
   MinGW solo si NTDDI_VERSION >= 0x0A000006 (Windows 10 1809 / RS5). Por eso,
   además de _WIN32_WINNT, fijamos NTDDI_VERSION ANTES de incluir windows.h.
   Si no se hace, el compilador dirá que 'HPCON' no está declarado. */

#ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0A00
#elif _WIN32_WINNT < 0x0A00
#  undef  _WIN32_WINNT
#  define _WIN32_WINNT 0x0A00
#endif

#ifndef NTDDI_VERSION
#  define NTDDI_VERSION 0x0A000006
#elif NTDDI_VERSION < 0x0A000006
#  undef  NTDDI_VERSION
#  define NTDDI_VERSION 0x0A000006
#endif

#include <windows.h>                  // API de Windows: procesos, pipes, handles, ConPTY
#include <vector>                     // Para el buffer del comando

/* ── Carga dinámica de ConPTY ────────────────────────────────────────────────
   Con MinGW (y especialmente con -static) los símbolos CreatePseudoConsole y
   ClosePseudoConsole no siempre se enlazan bien contra kernel32, lo que puede
   hacer que la pseudoconsola "se cree" sin error pero nunca conecte la salida.
   Por eso los resolvemos en tiempo de ejecución con GetProcAddress, que es la
   forma fiable y recomendada por Microsoft. Funciona con o sin -static. */

typedef HRESULT (WINAPI *PFN_CreatePseudoConsole)(COORD, HANDLE, HANDLE, DWORD, HPCON*);
typedef void    (WINAPI *PFN_ClosePseudoConsole)(HPCON);

static PFN_CreatePseudoConsole pCreatePseudoConsole = nullptr;   // Puntero a la CreatePseudoConsole real del sistema
static PFN_ClosePseudoConsole  pClosePseudoConsole  = nullptr;   // Puntero a la ClosePseudoConsole real del sistema

static bool loadConPTY()                                         // Carga ambas funciones desde kernel32.dll una sola vez
{
    if (pCreatePseudoConsole && pClosePseudoConsole) return true; // Ya cargadas: no repetir
    HMODULE hK = GetModuleHandleW(L"kernel32.dll");               // kernel32 siempre está presente en todo proceso Windows
    if (!hK) return false;
    pCreatePseudoConsole = (PFN_CreatePseudoConsole)GetProcAddress(hK, "CreatePseudoConsole");
    pClosePseudoConsole  = (PFN_ClosePseudoConsole) GetProcAddress(hK, "ClosePseudoConsole");
    return (pCreatePseudoConsole && pClosePseudoConsole);         // true solo si Windows soporta ConPTY (10 1809+)
}

/* ── Directorio del ejecutable ───────────────────────────────────────────────
   Devuelve la carpeta donde reside el .exe (sin barra final). Se usa como
   directorio de trabajo (lpCurrentDirectory) al lanzar mariadb.exe, de modo que
   éste encuentre sus DLLs en .\bin\ sin importar desde dónde se abra el programa
   (doble clic, acceso directo, etc.). Sin esto el proceso hijo falla con el
   error 0xc0000142 ("la aplicación no se pudo iniciar correctamente"). */

static std::wstring getExeDir()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(NULL, path, MAX_PATH);        // Ruta completa del .exe en ejecución
    std::wstring full(path);
    auto pos = full.rfind(L'\\');                    // Última barra invertida
    if (pos != std::wstring::npos) full.resize(pos); // Recorta el nombre del archivo: queda solo la carpeta
    return full;
}

/* ── Conversión UTF-8 (std::string) → UTF-16 (std::wstring) ──────────────────
   La API W de Windows trabaja en UTF-16, así que convertimos los argumentos. */

static std::wstring to_wstring(const std::string& str)
{
    if (str.empty()) return L"";
    int need = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring w(need, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &w[0], need);
    return w;
}

/* ── Filtro de secuencias de escape VT (CSI y OSC) ───────────────────────────
   ConPTY emite la tabla de MariaDB con secuencias ANSI. La mayoría son de color
   y se descartan. Pero algunas representan ESPACIADO mediante movimiento de
   cursor en lugar de espacios literales; si las descartáramos sin más, las
   columnas de la tabla quedarían desalineadas (un '|' corrido, datos pegados).

   Caso importante: CSI 'C' (ESC[<n>C) = "mover cursor n columnas a la derecha".
   ConPTY lo usa para saltar espacios en blanco. Lo convertimos en n espacios
   reales para conservar la alineación de la tabla.

   La máquina de estados conserva su estado entre llamadas (se pasa por
   referencia) para ser robusta ante secuencias partidas entre dos lecturas. */

enum class VTState { NORMAL, ESC_SEEN, CSI, OSC };

static void filterVT(const char* data, DWORD len,
                     std::string& out, VTState& st, bool& oscEsc, std::string& csiParams)
{
    for (DWORD i = 0; i < len; i++)
    {
        unsigned char c = (unsigned char)data[i];
        switch (st)
        {
            case VTState::NORMAL:
                if (c == 0x1B) st = VTState::ESC_SEEN;          // ESC: posible inicio de secuencia
                else if (c != '\r') out += (char)c;             // Texto normal (descartamos \r)
                break;

            case VTState::ESC_SEEN:
                if      (c == '[') { st = VTState::CSI; csiParams.clear(); }  // ESC[ → secuencia CSI
                else if (c == ']') st = VTState::OSC;            // ESC] → secuencia OSC
                else {                                          // ESC seguido de otra cosa: no era secuencia
                    st = VTState::NORMAL;
                    out += (char)0x1B;
                    if (c != '\r') out += (char)c;
                }
                break;

            case VTState::CSI:
                if (c >= 0x40 && c <= 0x7E)                      // Byte final de la secuencia CSI (0x40..0x7E)
                {
                    if (c == 'C')                               // 'C' = avanzar cursor N columnas → equivale a N espacios
                    {
                        int n = csiParams.empty() ? 1 : atoi(csiParams.c_str());  // Sin parámetro = 1
                        if (n < 0) n = 0;
                        if (n > 4096) n = 4096;                 // Límite de seguridad
                        out.append((size_t)n, ' ');            // Reinsertamos el espaciado como espacios reales
                    }
                    // Cualquier otro CSI (colores 'm', cursor 'H', borrado 'K', etc.) se descarta
                    st = VTState::NORMAL;
                }
                else
                {
                    // Acumula los parámetros numéricos/intermedios de la secuencia (ej. "12" en ESC[12C)
                    csiParams += (char)c;
                }
                break;

            case VTState::OSC:                                  // OSC termina con BEL (0x07) o con ESC '\'
                if (c == 0x07) { st = VTState::NORMAL; oscEsc = false; }
                else if (c == 0x1B) oscEsc = true;
                else if (c == '\\' && oscEsc) { st = VTState::NORMAL; oscEsc = false; }
                else oscEsc = false;
                break;
        }
    }
}

// ── Función principal ────────────────────────────────────────────────────────

std::string ptyfunc(std::string sqlinput,         // Comando SQL a ejecutar (o "backup")
                    std::string inputuser,         // Usuario
                    std::string inputpass,         // Contraseña
                    std::string inputserver,       // IP del servidor
                    std::string inputport,         // Puerto de MySQL/MariaDB
                    std::string inputdatabase)     // Nombre de la base de datos
{
    // ── Caso especial "backup": vuelca la base de datos a un .sql con fecha ──
    if (sqlinput == "backup")
    {
        auto now = std::chrono::system_clock::now();
        std::string nowStr = std::format("{:%Y-%m-%d-%H-%M-%S}", now);

        std::system("mkdir .\\backups 2>nul");      // Crea la carpeta backups si no existe

        std::string cmd = ".\\bin\\mariadb-dump.exe"
                          " --user=" + inputuser +
                          " --password=" + inputpass +
                          " --host=" + inputserver +
                          " --port=" + inputport +
                          " --databases " + inputdatabase +
                          " > .\\backups\\backup_" + nowStr + ".sql";

        int stateCode = std::system(cmd.data());
        if (stateCode != 0) return "Hubo un error al generar el backup de la base de datos";
        return "Se hizo el backup con éxito";
    }

    // ── Construcción del comando para mariadb.exe ──
    std::wstring cmd = L".\\bin\\mariadb.exe";
    cmd += L" -u";  cmd += to_wstring(inputuser);
    cmd += L" -p";  cmd += to_wstring(inputpass);
    cmd += L" -D "; cmd += to_wstring(inputdatabase);
    cmd += L" -h "; cmd += to_wstring(inputserver);
    cmd += L" -P "; cmd += to_wstring(inputport);
    cmd += L" -e \""; cmd += to_wstring(sqlinput); cmd += L"\"";

    std::vector<wchar_t> cmdBuffer(cmd.begin(), cmd.end());  // CreateProcessW necesita un buffer modificable
    cmdBuffer.push_back(0);

    std::string outTerm;   // Acumula la salida ya filtrada

    // ── Carga de ConPTY ──
    if (!loadConPTY())
        return "Error | Este Windows no soporta ConPTY (se requiere Windows 10 1809 o superior).";

    /* ── Creación de los pipes ────────────────────────────────────────────────
       hPipeInRead/hPipeInWrite  → entrada hacia mariadb.exe (no la usamos, el
                                    comando va en -e, pero ConPTY los exige).
       hPipeOutRead/hPipeOutWrite → salida desde mariadb.exe (lo que leemos). */

    HANDLE hPipeInRead = NULL,  hPipeInWrite = NULL;
    HANDLE hPipeOutRead = NULL, hPipeOutWrite = NULL;
    SECURITY_ATTRIBUTES sa{ sizeof(sa), NULL, TRUE };

    if (!CreatePipe(&hPipeInRead, &hPipeInWrite, &sa, 0) ||
        !CreatePipe(&hPipeOutRead, &hPipeOutWrite, &sa, 0))
        return "Error | No se pudieron crear los pipes de comunicación.";

    // ── Creación de la pseudoconsola ──
    HPCON hPC = NULL;
    COORD size = { 120, 30 };                    // Tamaño "visual" de la consola: influye en el formato de la tabla
    HRESULT hr = pCreatePseudoConsole(size, hPipeInRead, hPipeOutWrite, 0, &hPC);
    if (FAILED(hr)) {
        CloseHandle(hPipeInRead);  CloseHandle(hPipeInWrite);
        CloseHandle(hPipeOutRead); CloseHandle(hPipeOutWrite);
        return "Error | No se pudo crear la PseudoConsola. HRESULT: " + std::to_string(hr);
    }

    /* CRÍTICO: ConPTY ya duplicó internamente hPipeInRead y hPipeOutWrite.
       Cerramos NUESTRAS copias ahora mismo. Si no lo hiciéramos, quedaría un
       escritor extra en el pipe de salida y el EOF nunca llegaría, dejando el
       bucle de lectura sin recibir nada. Conservamos hPipeInWrite y hPipeOutRead. */
    CloseHandle(hPipeInRead);
    CloseHandle(hPipeOutWrite);

    // ── Configuración del proceso con el atributo de pseudoconsola ──
    STARTUPINFOEXW si{};
    si.StartupInfo.cb = sizeof(STARTUPINFOEXW);

    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrSize);                 // Calcula la memoria necesaria
    si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrSize);
    if (!si.lpAttributeList ||
        !InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize) ||
        !UpdateProcThreadAttribute(si.lpAttributeList, 0,
                                   PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                                   hPC, sizeof(hPC), NULL, NULL))
    {
        if (si.lpAttributeList) HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        CloseHandle(hPipeInWrite); CloseHandle(hPipeOutRead);
        pClosePseudoConsole(hPC);
        return "Error | No se pudo configurar el proceso de la pseudoconsola.";
    }

    // ── Directorio de trabajo = carpeta del .exe (clave para evitar 0xc0000142) ──
    std::wstring exeDir = getExeDir();

    // ── Lanzamiento de mariadb.exe ──
    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(
        NULL, cmdBuffer.data(),
        NULL, NULL,
        TRUE,                                     // bInheritHandles = TRUE (requerido por ConPTY)
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        exeDir.empty() ? NULL : exeDir.c_str(),   // directorio de trabajo = carpeta del .exe
        &si.StartupInfo, &pi);

    if (!ok) {
        DWORD err = GetLastError();
        DeleteProcThreadAttributeList(si.lpAttributeList);
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
        CloseHandle(hPipeInWrite); CloseHandle(hPipeOutRead);
        pClosePseudoConsole(hPC);
        return "Error al ejecutar MariaDB. Código: " + std::to_string(err) +
               ". Asegúrate de que .\\bin\\mariadb.exe existe en la ubicación correcta de la app.";
    }

    /* Cerramos el extremo de escritura de stdin: no enviamos nada por pipe (el
       comando va en -e), así mariadb.exe ve EOF en su entrada y termina limpio. */
    CloseHandle(hPipeInWrite);

    /* ── Bucle de lectura NO BLOQUEANTE ───────────────────────────────────────
       NO usamos un ReadFile bloqueante: ConPTY mantiene su propio escritor en el
       pipe de salida hasta que se cierra la pseudoconsola (que cerramos DESPUÉS
       de este bucle), así que un ReadFile bloqueante esperaría datos para siempre
       y colgaría la app de un solo hilo (el síntoma "no responde").

       En su lugar sondeamos con PeekNamedPipe:
         - Si hay bytes disponibles, los leemos y filtramos.
         - Si no hay y el proceso aún vive, esperamos un poco (sin quemar CPU).
         - Cuando el proceso ya terminó Y el pipe lleva un rato vacío, salimos.
       Así garantizamos capturar toda la tabla sin bloquearnos jamás. */

    VTState st = VTState::NORMAL;
    bool oscEsc = false;
    std::string csiParams;   // Acumula los parámetros numéricos de las secuencias CSI (para interpretar ESC[<n>C)
    char buffer[4096];
    DWORD bytesRead = 0;
    DWORD bytesAvailable = 0;

    bool processActive = true;          // ¿mariadb.exe sigue corriendo?
    int  idleRounds = 0;                 // Vueltas consecutivas con el pipe vacío tras terminar el proceso

    while (true)
    {
        // Mira cuántos bytes hay en el pipe sin bloquear. Si falla, el pipe se cerró.
        if (!PeekNamedPipe(hPipeOutRead, NULL, 0, NULL, &bytesAvailable, NULL))
            break;

        if (bytesAvailable > 0)
        {
            if (ReadFile(hPipeOutRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
                filterVT(buffer, bytesRead, outTerm, st, oscEsc, csiParams);
            idleRounds = 0;              // Llegaron datos: reinicia el contador de inactividad
        }
        else
        {
            if (processActive)
            {
                // Espera hasta 15ms a que el proceso termine (no quema CPU)
                if (WaitForSingleObject(pi.hProcess, 15) == WAIT_OBJECT_0)
                    processActive = false;
            }
            else
            {
                /* El proceso ya terminó y el pipe está vacío en esta vuelta.
                   ConPTY puede tardar en depositar los últimos bytes, así que
                   esperamos varias vueltas de silencio antes de salir. Si entra
                   un solo byte, idleRounds se reinicia arriba. */
                Sleep(10);
                if (++idleRounds >= 100)   // ~1 segundo de silencio total → fin seguro
                    break;
            }
        }
    }

    // ── Limpieza ──
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hPipeOutRead);
    pClosePseudoConsole(hPC);
    DeleteProcThreadAttributeList(si.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, si.lpAttributeList);

    outTerm += "\n";        // Newline final por estética
    return outTerm;
}

#else         // Si NO se compila para Windows (Linux): se conserva la implementación original con forkpty()

#include <pty.h>          // Se necesita para la función forkpty() para ejecutar un comando con un proceso hijo
#include <unistd.h>       // Sirve para leer los datos de salida del proceso hijo, ejecutr en el proceso hijo y salir del proceso hijo
#include <sys/wait.h>     // Sirve para la función wait() para esperar a la finalización correcta del proceso hijo

std::string ptyfunc(std::string sqlinput,           // Pide como argumento el comando a ejecutar con el servidor MySQL
                    std::string inputuser,          // Usuario a autenticar
                    std::string inputpass,          // Contraseña
                    std::string inputserver,        // IP de servidor
                    std::string inputport,          // Puerto de servidor
                    std::string inputdatabase)      // Nombre de base de datos
{
    using namespace std::string_literals;                               // Se usa para concatenar los strings usando ""s

    if (sqlinput == "backup")     // En caso de que sqlinput (el comando ingresado) sea igual al comando personalizado para generar backups...
    {
        auto now = std::chrono::system_clock::now();                                          // now almacena la hora actual
        std::string nowStr = std::format("{:%Y-%m-%d-%H-%M-%S}", now);                        // Se convierte a un formato en el que pueda separarse todo con guiones, para evitar usar ":" los cuales no pueden usarse como títulos de archivos
        int stateCode;                                                                        // Almacena el código de estado de cada comando, esto para recibir información en caso de errores
        std::string cmd;                                                                      // Almacena el comando a ejecutar
        cmd = "/usr/bin/mkdir -p ./backups/";                                                 // Crea la carpeta backups si NO existe
        stateCode = std::system(cmd.data());                                                  // El valor del código de estado del comando se almacena en stateCode
        if (stateCode != 0) return "Hubo un error al crear la carpeta de backups";            // Si el código de estado NO es igual a cero significa que hubo un error, entonces informará que no se creo la carpeta correctamente
        cmd = "/usr/bin/mysqldump "s +                                                        // Si no hubo un error con el comando anterior, prepara el comando para dumpear la base de datos
              " --user "s + inputuser +
              " --password="s + inputpass +
              " --host "s + inputserver +
              " --port "s + inputport +
              " --databases "s + inputdatabase +
              " > ./backups/backup_"s + nowStr;                                               // Además de que redirige la salida a ./backups/backup_HORAACTUAL
        stateCode = std::system(cmd.data());                                                  // Ejecuta el comando y almacena el código de estado
        if (stateCode != 0) return "Hubo un error al generar el backup de la base de datos";  // Si el código de estado es distinto a cero significa que hubo un error al generar el backup
        return "Se hizo el backup con éxito";                                                 // En caso de que el comando anterior haya ocurrido bien, retornará que más bien todo se creó con éxito, e inmediatamente saldrá de la función
    }

    /* "backup" es muy util ya que sirve tanto en la terminal como también como función desde el botón de backup en el panel de administración.
        En caso de que sqlinput NO sea "backup", se ejecutará lo siguiente como si hubiese sido un comando común: */

    std::string outTerm = "";                                           // Se declara outTerm, string que servirá para recibir la respuesta de la base de datos
    std::string userarg = "--user="s + inputuser;                                   // --user=usuario
    std::string passarg = "--password="s + inputpass;                               // --password=contraseña
    std::string srvrarg = "--host="s + inputserver;                                 // --host=ipservidor
    std::string portarg = "--port="s + inputport;                                   // --port=puerto
    std::string basearg = "--database="s + inputdatabase;                           // --database=basededatos
    std::string query   = "-e "s + sqlinput;                                        // --e comando | Esta línea para abajo ocurre en caso de que el comando "backup" no haya sido ingresado

    int master;                                                         // Declaración de master, que será la dirección en la memoria donde se hará el fork al proceso hijo
    pid_t pid = forkpty(&master, nullptr, nullptr, nullptr);            // Se ejecuta el proceso hijo, se almacenará el PROCESS IDENTIFIER del proceso en la variable pid
    if (pid == 0)                                                       // Si el pid es igual a 0, significa que la variable ahora almacena el proceso hijo con éxito, entonces...
    {
        execlp("/usr/bin/mariadb", "/usr/bin/mariadb",                                        // Ejecutará a mysql/mariadb con los argumentos que se armaron previamente
               userarg.data(), passarg.data(), srvrarg.data(),
               basearg.data(), query.data(), nullptr);                  // Y después de escribir el último argumento hay que escribir un nullptr al final, esto para indicarle a execlp() que ya no se necesitan más argumentos
        _exit(1);                                                       /* Al ejecutar execlp() en el proceso hijo, el proceso hijo nunca debería retornar, y si llega a retornar sería en caso de que haya ocurrido un error
                                                                           así que en caso de que retorne, llegará a esta línea y el proceso hijo será matado con _exit() */
    }

    // Cuando el programa llega a esta línea, empieza a leer la salida del proceso hijo

    char buffer[4096];                                                  // Prepara un buffer de 4096 bytes, donde almacenará 4096 carácteres conforme vaya leyendo la salida
    while (true)                                                        // Empieza el bucle de lectura
    {
        ssize_t ln = read(master, buffer, sizeof(buffer) - 1);          /* ln almacenará la cantidad de bytes leídos desde master, el cual es la dirección en memoria que estaría recibiendo la salida del proceso hijo, y los almacena en buffer,
                                                                           puede leer como máximo 4095 carácteres (sizeof(buffer) - 1) para no desbordar el buffer*/
        if (ln <= 0) break;                                             // Si ln almacena una cantidad menor o igual a 0, romperá el bucle de lectura, 0 implicaría que leyó correctamente todo master, si es -1, ocurrió un error de lectura
        for (int w = 0; w < ln; w++)                                    // Se recorrerá el valor total de ln, y como lo leído de master se almacenó en buffer, se leerá cada caracter del buffer hasta que el contador llegue a ln
            if (buffer[w] != '\r') outTerm += buffer[w];                // Si el carácter del índice actual dentro de buffer NO es un retorno de carro, almacenará el carácter en outTerm
    }
    wait(nullptr);                                                      // Llama a wait para que el proceso padre (master) espere a que el proceso hijo termine debidamente
    return outTerm;       // Retorna outTerm, el string con toda la respuesta del comando hacia la base de datos MySQL
}

#endif
