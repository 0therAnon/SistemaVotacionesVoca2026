#include "pty.hpp"
#include <string>                     // Llama a la librería string, ya que se necesitan usar diversos formatos de string a continuación, como wstring y funciones para manipular los strings
#include <chrono>
#include <iostream>

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

#if defined(_WIN32)                   // A continuación, verifica si se compilará para windows, compilará el código hasta alrededor de la línea 173

#ifndef _WIN32_WINNT                  // ConPTY (CreatePseudoConsole / ClosePseudoConsole) requiere Windows 10+
#  define _WIN32_WINNT 0x0A00
#elif _WIN32_WINNT < 0x0A00
#  undef  _WIN32_WINNT
#  define _WIN32_WINNT 0x0A00
#endif

#include <winsock2.h>                 // Llama a la librería winsock2.h
#include <ws2tcpip.h>                 // Llamada a la librería de la comunicación TCP/IP
#pragma comment(lib, "Ws2_32.lib")    // La librería para su compilación la buscará como "Ws2_32.lib"
#include <windows.h>                  // Llamada a la librería para comunicarse con la API de Windows

static std::wstring to_wstring(const std::string& str)      // Función para convertir de string a wstring
{
    if (str.empty()) return L"";      // Si el string de entrada está vacío, devolverá un string en widechar vacío
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);   // Declara el tamaño necesitado para almacenar el string
    std::wstring wstr(size_needed, 0);                                                          // Declara a wstr como tipo widechar string, con el tamaño ya calculado en size_needed
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);       // Y procede a convertir el string recibido a un tipo widechar string
    return wstr;                      // Retorna el string convertido
}

// Para que ptyfunc() pueda enviar los comandos a la base de datos necesita las credenciales de acceso, las cuales son sus argumentos

std::string ptyfunc(std::string sqlinput,         // Comando a ejecutar
                    std::string inputuser,        // Usuario
                    std::string inputpass,        // Contraseña
                    std::string inputserver,      // IP de servidor
                    std::string inputport,        // Puerto de MySQL
                    std::string inputdatabase)    // Nombre de la base de datos
{
    std::string outTerm = "";                           // Declaración de outTerm, el cual será el valor que se reciba como respuesta del comando
    std::wstring sqliwstr = to_wstring(sqlinput);       // Convierte los valores
    std::wstring userwstr = to_wstring(inputuser);      // de las credenciales de
    std::wstring passwstr = to_wstring(inputpass);      // acceso a strings de tipo
    std::wstring srvrwstr = to_wstring(inputserver);    // widechar, ya que la API de
    std::wstring portwstr = to_wstring(inputport);      // Windows trabaja con UTF-16,
    std::wstring basewstr = to_wstring(inputdatabase);  // y widechar es lo mismo a UTF-16

    std::wstring cmd;

    if (sqlinput != "backup")                           // Si el comando a ejecutar NO es "backup", el cual es un comando personalizado del sistema de votaciones, entonces armará una query normalmente
        cmd = L".\\bin\\mariadb.exe";      // Empieza a armar el comando a ejecutar en la pseudoconsola: .\bin\mariadb.exe
    else                                                // En caso de que el comando SÍ sea "backup", procederá a realizar un backup de la base de datos
        cmd = L"if (!(Test-Path \".\\backups\")) { New-Item -ItemType Directory -Path \".\\backups\" } ; .\\bin\\mariadb-dump.exe"; // Empieza a armar la query para el backup

    cmd += L" -u"; cmd += userwstr;                     // .\bin\mariadb.exe -u usuario
    cmd += L" -p"; cmd += passwstr;                     // .\bin\mariadb.exe -u usuario -p contraseña
    if (sqlinput != "backup")                           // Si el comando a ejecutar NO es "backup", el cual es un comando personalizado del sistema de votaciones, entonces armará una query normalmente
        {cmd += L" -D "; cmd += basewstr;}              // .\bin\mariadb.exe -u usuario -p contraseña -D basededatos
    else                                                // Si el comando a ejecutar SÍ es "backup", cambiará la forma de llamar al argumento para especificar el nombre de la base de datos, ya que mariadb-dump usa otro argumento para esto
        {cmd += L" --databases "; cmd += basewstr;}     // Procede a llamar a la base de datos con "--databases" en vez de "-D"
    cmd += L" -h "; cmd += srvrwstr;                    // .\bin\mariadb.exe -u usuario -p contraseña -D basededatos -h ipservidor
    cmd += L" -P "; cmd += portwstr;                    // .\bin\mariadb.exe -u usuario -p contraseña -D basededatos -h ipservidor -P puerto

    if (sqlinput != "backup")                           // Si el comando NO es "backup", el cual es un comando personalizado del sistema de votaciones, entonces seguirá armando la query normalmente
    {
        cmd += L" -e \""; cmd += sqliwstr;              // .\bin\mariadb.exe -u usuario -p contraseña -D basededatos -h ipservidor -P puerto -e "comando
        cmd += L"\"";                                   // .\bin\mariadb.exe -u usuario -p contraseña -D basededatos -h ipservidor -P puerto -e "comando"
    }
    else                                                // Si el comando SÍ es "backup", entonces procederá a crear una redirección a un archivo dentro de la carpeta backups
    {
        auto now = std::chrono::system_clock::now();
        std::wstring nowWstr = std::format(L"{:%Y-%m-%d-%H-%M-%S}", now);
        cmd += L" > \".\\backups\\backup_" + nowWstr;
        cmd += L".sql\"";
    }

    std::wcout<<cmd<<"\n";

    /* Se crean los pipes para la comunicación, los cuales sirven para enviar datos de entrada o salida como recibir datos de entrada o salida

       Se puede explicar de esta forma:

       [Este programa] -- hPipeInWrite -- > --hPipeInRead -- > [MariaDB/MySQL]   (enviar datos)
       [Este programa] < -- hPipeOutRead -- < -- hPipeOutWrite -- [MariaDB/MySQL]  (recibir datos)  */

    HANDLE hPipeInRead, hPipeInWrite;                           // Se declaran los pipes de entrada de lectura y escritura
    HANDLE hPipeOutRead, hPipeOutWrite;                         // Se declaran los pipes de salida de lectura y escritura
    SECURITY_ATTRIBUTES sa{ sizeof(sa), NULL, TRUE };           // Se declara de que los pipes se pueden heredar al proceso hijo, el cual sería el proceso que se crea al invocar la pseudoconsola a ejecutar el comando armado
    CreatePipe(&hPipeInRead, &hPipeInWrite, &sa, 0);            // Se crean los pipes de entrada con el sa anteriormente especificado
    CreatePipe(&hPipeOutRead, &hPipeOutWrite, &sa, 0);          // Se crean los pipes de salida con el sa anteriormente especificado

    HPCON hPC;                                                            // Se declara la variable de la pseudoconsola
    COORD size = { 120, 30 };                                             // Se crea el tamaño visual de la pseudoconsola, aunque será invisible, influye en la respuesta de la base de datos
    CreatePseudoConsole(size, hPipeInRead, hPipeOutWrite, 0, &hPC);       // Se crea la pseudoconsola, con los pipes donde la terminal recibe datos de entrada, y escribe su salida

    STARTUPINFOEXW si{};                                // Crea una estructura que contendrá todas las propiedades y configuraciones de la pseudoconsola, además de que la estructura acepta texto en UTF-16 (por eso el W final en STARTUPINFOEXW)
    si.StartupInfo.cb = sizeof(STARTUPINFOEXW);         // Se le indica a windows el tamaño de la estructura, el cual será el tamaño de STARTUPINFOEXW

    SIZE_T attrListSize;                                                                                    // Se declara attrListSize, el cual guardará cuanta memoria se necesita por InitializeProcThreadAttributeList
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);                                           // Se llama a InitializeProcThreadAttributeList para inicializar un proceso NULL, el cual no hace nada pero calcula la memoria necesaria
    si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrListSize);         // Reserva la cantidad de memoria calculada anteriormente pidiendo el tamaño de attrListSize, y reserva esa memoria en el heap
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrListSize);                             // Llama a InitializeProcThreadAttributeList nuevamente pero con los atributos en si.lpAttributeList, con eso inicializa un proceso real
    UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,                   // Ahora, actualiza los atributos del proceso recién iniciado, diciendole que debe de usar la pseudoconsola declarada como hPC
                              hPC, sizeof(hPC), NULL, NULL);

    PROCESS_INFORMATION pi{};                           // Crea una estructura que contendrá la información del proceso actual
    CreateProcessW(NULL, cmd.data(), NULL, NULL, FALSE,                                 // Ahora con esta línea, ejecuta el comando armado previamente al incio de la función
                   EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &si.StartupInfo, &pi);
    CloseHandle(hPipeInWrite);                          // Ahora, se cierran los pipes que eran de la pseudoconsola
    CloseHandle(hPipeInRead);                           // ya que si estos pipes no se cierran debidamente
    CloseHandle(hPipeOutWrite);                         // el programa estará esperando recibir datos de entrada eternamente

    char buffer[4096];                          // Se declara un buffer de tipo char que almacenará como máximo 4096 carácteres, este buffer sirve para almacenar poco a poco los carácteres de la respuesta de la base de datos
    DWORD bytesAvailable = 0;                   // Se declara a bytesAvailable que almacenará la cantidad de bytes restantes por leer de la respuesta de la base de datos por el comando ejecutado anteriormente

    /* Al supuestamente encontrarnos en una terminal virtual, Windows formateará la respuesta de la base de datos usando OSC y CSI:

       CSI: Sirve para controlar el cursor de la terminal, los colores de las respuestas y de la terminal, y la edición de la visualización de los datos en la terminal
       OSC: Sirve para configurar el título de la ventana y definir links

       El inicio de las secuencias de CSI comienza con ESC (0x1B) y un [ seguido de otros parámetros y un carácter final como por ejemplo 1;2m
       El inicio de las secuencias de OSC comienza con ESC (0x1B) y un ] y finaliza con ST ESC\

       Al ser carácteres que windows formatea en la salida de los textos de la terminal, deben de ser eliminados, entonces en el bucle siguiente, se detectarán los carácteres de los dos tipos y se filtrarán */

    enum class VTState { NORMAL, OSC, CSI };          // Declara tres estados de la terminal: NORMAL para texto normal, OSC para secuencias OSC, y CSI para secuencias CSI
    VTState vtState = VTState::NORMAL;                // Asigna a VTState inicialmente como NORMAL

    Sleep(500);                               // Dá un tiempo de reposo del programa de 500 milisegundos para que mariadb.exe se inicialice y haga la consulta al servidor, y después de esto...
    while (true)                              // Inicio del bucle de lectura de la respuesta del servidor
    {
        if (!PeekNamedPipe(hPipeOutRead, NULL, 0, NULL, &bytesAvailable, NULL)) break;                    // Si se intenta mirar dentro del pipe de lectura por medio de PeekNamedPipe y falla, se sale del bucle de lectura
        if (bytesAvailable > 0)                                                                           // Si hay más de 0 bytes dentro del pipe disponibles, entonces...
        {
            DWORD bytesRead;                                                                              // Se declara bytesRead como el valor que almacenará la cantidad de bytes leídos
            ReadFile(hPipeOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL);                         // Y se empieza a leer el pipe byte por byte, el contenido que se lea se guarda en el buffer de 4096 bytes
            for (DWORD i = 0; i < bytesRead; i++)                                                         // Se empieza a recorrer los bytes leídos
            {
                unsigned char c = buffer[i];                                                              // Se declara c como el carácter que se está leyendo actualmente del buffer
                switch (vtState)                                                                          // Empieza un switch, que verificará a cual bloque de código ir dependiendo del modo que almacene vtState
                {
                    case VTState::NORMAL:                                                                 // Si el modo es NORMAL...
                        if (c == 0x1B)                                                                    // Si se detecta que el byte actual es igual a ESC (0x1B), lo cual coincide con el inicio de CSI o OSC, entonces...
                        {
                            if (i + 1 < bytesRead)                                                        // Si se le suma un valor más a i y sigue siendo menor que bytesRead (para prevenir leer algo fuera del límite)...
                            {
                                if (buffer[i + 1] == ']') { vtState = VTState::OSC; i++; break; }         // Se verificará si el siguiente byte es igual al carácter ], si fuese así coincidiría con el inicio de OSC, entonces cambia el modo vtState
                                if (buffer[i + 1] == '[') { vtState = VTState::CSI; i++; break; }         // En cambio, si el siguiente byte es igual al carácter [, coincidiría con el inicio de CSI, entonces cambia el modo de vtState
                            }
                        }
                        if (c == '\r') break;                                                             // Si se detecta que el byte actual es un retorno de carro, hace un break para descartarlo, ya que no es un carácter válido
                        outTerm += c;                                                                     // En caso de que el carácter actual NO sea igual a 0x1B ni a un \r, entonces lo agrega a outTerm
                        break;
                    case VTState::OSC:                                                                    // Si el modo es OSC...
                        if (c == 0x07 || (c == '\\' && i > 0 && buffer[i - 1] == 0x1B))                   // Si se detecta que el byte actual es igual a 0x07 o a un \, y que el carácter anterior fue un ESC (0x1B), entonces significa que terminó CSI
                            vtState = VTState::NORMAL;                                                    // Regresa al modo NORMAL
                        break;
                    case VTState::CSI:                                                                    // Si el modo es CSI...
                        if (c >= 0x40 && c <= 0x7E) vtState = VTState::NORMAL;                            // Si detecta que el carácter actual es mayor a 0x40 o es menor a 0x7E, significará el fin de CSI, entonces regresará al modo NORMAL
                        break;
                }
            }
        }
        else      // En caso de que hayan menos de 0 bytes disponibles en el pipe, entonces...
        {
            DWORD exitCode;                                     // Se declara exitCode para verificar si mariadb.exe terminó su ejecución en la siguiente línea
            GetExitCodeProcess(pi.hProcess, &exitCode);         // Se llama GetExitCodeProcess() con el parámetro pi.Process para tener la información del proceso y guardará el código de estado que responda GetExitCodeProcess() dentro de exitCode
            if (exitCode != STILL_ACTIVE) break;                // Si el proceso responde con un código de salida NO IGUAL a STILL_ACTIVE, significa que el proceso ya terminó, en ese caso hace un break del bucle de lectura
        }
    }

    WaitForSingleObject(pi.hProcess, INFINITE);                 // Espera a que mariadb.exe termine completamente por un tiempo INFINITE, quiere decir que esperará a mariadb.exe a terminar el proceso hasta que mariadb.exe lo comunique
    CloseHandle(pi.hProcess);                                   // Se cierra el PROCESS_INFORMATION del proceso actual
    CloseHandle(pi.hThread);                                    // Se cierra el PROCESS_INFORMATION del hilo de mariadb.exe
    CloseHandle(hPipeOutRead);                                  // Se cierra el pipe de lectura
    ClosePseudoConsole(hPC);                                    // Se cierra la pseudoconsola
    DeleteProcThreadAttributeList(si.lpAttributeList);          // Elimina los atributos y configuraciones del proceso actual
    HeapFree(GetProcessHeap(), 0, si.lpAttributeList);          // Y por último, libera la memoria anteriormente guardada

    outTerm += "\n";      // Se le agrega un newline a outTerm por pura estética
    return outTerm;       // Y retorna la respuesta que haya recibido de la base de datos que fue guardada en outTerm
}

#else         // Si se detecta que NO se está compilando para windows...

#include <pty.h>          // Se necesita para la función forkpty() para ejecutar un comando con un proceso hijo
#include <unistd.h>       // Sirve para leer los datos de salida del proceso hijo, ejecutr en el proceso hijo y salir del proceso hijo
#include <sys/wait.h>     // Sirve para la función wait() para esperar a la finalización correcta del proceso hijo
#include <cstdlib>

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
        execlp("/usr/bin/mysql", "/usr/bin/mysql",                                        // Ejecutará a mysql con los argumentos que se armaron previamente
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
