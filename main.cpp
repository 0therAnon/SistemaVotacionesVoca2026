// Llamada a todas las funciones y recursos necesarios
#include "globals.hpp"
#include "db/database.hpp"
#include "ui/objects.hpp"
#include "screens/screen_logo.hpp"
#include "screens/screen_config.hpp"
#include "screens/screen_mainmenu.hpp"
#include "screens/screen_adminmenu.hpp"
#include "screens/screen_confirmation.hpp"
#include "screens/screen_votation.hpp"
#include "screens/screen_ending.hpp"

int main(void)
{
    // FLAG_WINDOWS_TOPMOST:    Hace que la ventana siempre esté encima de todas las demás
    // FLAG_WINDOW_UNDECORATED: Hace que el titulo, y los bordes de la ventana se eliminen
    // FLAG_WINDOW_RESIZABLE:   Permite al usuario poder modificar el tamaño de la ventana
    SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_RESIZABLE);      // Configura esas opciones mencionadas anteriormente para la parte gráfica
    InitWindow(GetScreenWidth(), GetScreenHeight(), "Programa de Votaciones VOCA 2026");        // GetScreenWidth() y GetScreenHeight() obtienen el tamaño de la pantalla en alto y ancho, y crean una ventana de ese tamaño con ese título
    SetWindowMinSize(GetMonitorWidth(0) / 2, GetMonitorHeight(0) / 2);  // Configura el tamaño mínimo de la ventana, el cual es la mitad del tamaño del monitor en ancho y alto
    SetWindowMaxSize(GetMonitorWidth(0), GetMonitorHeight(0));          // Configura el tamaño máximo de la ventana, el cual es el tamaño completo del monitor en ancho y alto

    Screen currentScreen = LOGO;                // Asigna a la variable currentScreen el valor igual a LOGO al inicio del programa, para que comience con la ventana de bienvenida y mostrando el logo del VOCA, el cual aún no ha sido asignado
    Screen oldCurrentScreen = currentScreen;    // Es igual a la variable currentScreen, solo que se actualiza después del backend y frontend de las pantallas para almacenar la ultima pantalla
    SetTextLineSpacing(nlSpacing);              // Se encarga de la cantidad de espacio entre newlines en el texto que se muestre en el frontend
    SetTargetFPS(60);                           // Especifica la cantidad de FPS a la que el programa se estará ejecutando, esta cantidad será 60 Frames Per Second

    screenWidth  = GetScreenWidth();  // La variable screenWidth tendrá el valor del ancho de la pantalla
    screenHeight = GetScreenHeight(); // La variable screenHeight tendrá el valor del alto de la pantalla
    fontSize       = screenHeight / 24; if ((int)fontSize       % 2 != 0) fontSize--;         // fontSize es el valor del tamaño del font (letra) del frontend, si el valor es impar, se le restará uno para que sea par
    mediumFontSize = fontSize   / 1.5;  if ((int)mediumFontSize % 2 != 0) mediumFontSize--;   // mediumFontSize es un font mediano, si el valor es impar, se le resta uno para que sea par
    littleFontSize = fontSize   / 2;                                                          // littleFontSize es un font pequeño

    fontTtf = LoadFontEx("./fonts/Inter_24pt-Regular.ttf", fontSize, 0, 250); // Es la variable que tendrá el font almacenado, busca la ruta del archivo del font a usar, se le pasa el tamaño (fontSize) como argumento
    monoTtf = LoadFontEx("./fonts/GoMonoNerdFont-Regular.ttf", fontSize, 0, 250); // Es la variable que tendrá el font almacenado, busca la ruta del archivo del font a usar, se le pasa el tamaño (fontSize) como argumento

    ImageFormat(&vocaLogo, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);    // Modifico el formato del logo del Voca, esto para permitir la transparencia de la imagen en las transiciones
    vocaLogoTexture = LoadTextureFromImage(vocaLogo);             // La imagen se convierte a textura, para que raylib la procese mejor y aplique su efecto de desaparecer
    UnloadImage(vocaLogo);                                        // Luego, se procede a descargar la imagen vocaLogo, ya que no se necesita más

    // ── Per-loop state ────────────────────────────────────────────────────────
    std::string outResultsMode      = "percentages";   // Verifica si el modo de salida de la pestaña "Resultados" tiene que ser en porcentajes o sino en cantidades
    std::string explorarFinalOutput = "";   // Almacena la salida de la pestaña "Explorar" en el panel del administrador, esto luego se mostrará en el frontend
    std::string strresultados       = "";   // Es un string que se usará en la pestaña "Resultados"
    std::string votes               = "";   // Es un string que almacena la cantidad de votos que posteriormente se usará en la pestaña "Resultados"
    std::string outputTerm          = "";   // Almacena la salida de la pestaña "Terminal" luego de ejecutar queries como administrador
    std::string modeInput           = "";   // Especifica el modo de entrada que se usan para la función inputfunc(), hay cuatro tipos de modos: allchars, regexponly, numberonly y boolean
    std::string studentName         = "";   // Guarda el nombre del estudiante en una variable, para luego mostrarlo en el frontend

    bool restartExplorar        = true;     // Verifica si la pestaña "Explorar" en el panel del administrador debe ser reiniciada, esto para no mostrar datos desactualizados
    bool restartResultados      = true;     // Verifica si la pestaña "Resultados" en el panel del administrador deber ser reiniciada, esto para no mostrar datos desactualizados
    bool restartTerminal        = true;     // Verifica si el administrador se había salido del panel luego de haber iniciado sesión, esto para cerrarla y obligarlo a volver a digitar las credenciales de acceso
    bool invalidIp              = false;    // Verifica si una IP digitada es válida o no, en algún campo de la pantalla CONFIGURATION o ADMINMENU, esto sirve para mostrar un mensaje en el frontend de que la IP es inválida
    bool inputEmpty             = false;    // Verifica si un valor en CONFIGURATION o ADMINMENU en la pestaña "Terminal" está vacío, esto sirve para mostrar un mensaje en el frontend de que hay datos vacíos o faltantes
    bool invalidCredentials     = false;    // Verifica si los valores en la pestaña "Terminal" del panel del administrador son credenciales válidas o correctas, o no
    bool successfulPdfCreation  = false;    // Verifica si el PDF se pudo crear correctamente con la funcion inform()
    bool existstudent           = false;    // Verifica si el estudiante que se intentó buscar con la cédula en MAINMENU existe o no
    bool errorUpdating          = false;    // Verifica si la función updateData() tuvo algun error o no
    bool errorConfig            = false;    // Verifica si la función configureData() tuvo algun error o no
    bool votoBlanco             = false;    // Verifica si el estudiante seleccionó un partido o un voto nulo, esto lo obliga a votar por algo, y no seleccionar nada | Es el bug descubierto por Josué Torres
    bool fromAdmin              = false;    // Verifica si la última pantalla en la que se encontraba el usuario era ADMINMENU o no, esto para devolverse a ADMINMENU si se accedió desde esa pantalla, ya que también se puede acceder por LOGO
    bool correctstudent         = false;    // Verifica si el estudiante que se encontró en la base de datos es el correcto, esto desde la ventana CONFIRMATION
    bool rqst                   = false;    // Verifica si se tiene que hacer una petición/request en la pestaña "Explorar" para actualizar los datos

    int   verifyvote        = 0;            // Verifica si el partido por el que se desea votar es el correcto, esto también desde la ventana CONFIRMATION

    // ── Main loop ─────────────────────────────────────────────────────────────
    while (!WindowShouldClose())    // Mientras no se haga una petición para cerrar la ventana (presionar tecla ESC), se ejecutará el siguiente código hasta la línea 144...
    {
        // Handle resize
        if (IsWindowResized())  // Si la ventana cambió de tamaño...
        {
            SetWindowMinSize(screenHeight * 1.5, screenHeight * 0.7);   // Dice de que el tamaño mínimo no puede ser menor a esto | La verdad ni yo entiendo por qué esta operación, pero dejenla ahí
            screenWidth  = GetScreenWidth();    // Actualiza el valor de screenWidth
            screenHeight = GetScreenHeight();   // Actualiza el valor de screenHeight
            while (screenHeight < GetMonitorWidth(0) / 2) {screenHeight++;}   // Si llega a ocurrir de que screenHeight es menor al tamaño del monitor dividido entre 2, aumentará su tamaño 1 vez por cada vez que se repita el while
            SetWindowSize(screenWidth, screenHeight);   // Se asigna el tamaño de la ventana a los valores de screenWidth y screenHeight manualmente
            fontSize       = screenHeight / 24; if ((int)fontSize       % 2 != 0) fontSize--;         // Se vuelven a
            mediumFontSize = fontSize   / 1.5;  if ((int)mediumFontSize % 2 != 0) mediumFontSize--;   // actualizar los
            littleFontSize = fontSize   / 2;                                                          // valores de los fonts
            objectCreation();   // Se llama a la funcion objectCreation() la cual se encarga de la creación de los objetos, como el tamaño de la ventana cambia, los objetos también deben de cambiar su eje x y eje y, como tamaños de ancho y alto
        }
        timer++;
        if (currentScreen != LOGO && timer > 900)  verifyConnection(currentScreen);  // Verifica la conexión con la base de datos
        // ── Backend switch ────────────────────────────────────────────────────
        switch (oldCurrentScreen)    // Busca cuál es la ventana actual
        {
            case LOGO:            // En caso de que la ventana sea LOGO
                screenLogoUpdate(currentScreen, errorConfig, errorUpdating);
                break;
            case CONFIGURATION:   // En caso de que la ventana sea CONFIGURATION
                screenConfigUpdate(currentScreen, errorConfig, errorUpdating,
                                   invalidIp, inputEmpty, fromAdmin);
                break;
            case MAINMENU:        // En caso de que la ventana sea MAINMENU
                screenMainmenuUpdate(currentScreen, existstudent, studentName);
                break;
            case ADMINMENU:       // En caso de que la ventana sea ADMINMENU
                screenAdminmenuUpdate(currentScreen,
                                      restartExplorar, restartResultados, restartTerminal,
                                      invalidIp, inputEmpty, invalidCredentials,
                                      successfulPdfCreation, fromAdmin, rqst, correctstudent,
                                      explorarFinalOutput, strresultados,
                                      votes, outputTerm, modeInput,
                                      outResultsMode, opcSize);
                break;
            case CONFIRMATION:    // En caso de que la ventana sea CONFIRMATION
                screenConfirmationUpdate(currentScreen, existstudent, correctstudent, verifyvote);
                break;
            case VOTATION:        // En caso de que la ventana sea VOTATION
                screenVotationUpdate(currentScreen, votoBlanco);
                break;
            case ENDING:          // En caso de que la ventana sea ENDING
                screenEndingUpdate(currentScreen, verifyvote, correctstudent);
                break;
            default: break;       // Por defecto, si no es ninguna de estas ejecuta un break, pero esto no pasará nunca, aunque igual es solo por defecto
        }

        // ── Frontend (drawing) ────────────────────────────────────────────────
        BeginDrawing();                       // Función que inicia el "dibujado" en el frontend
        if (darkMode)                                         // Si el modo oscuro está activo...
            ClearBackground({45, 45, 48, 255});               // Limpia el background y lo "pinta" de un color oscuro
        else
            ClearBackground({250, 244, 228, 255});            // Limpia el background y lo "pinta" de un color claro
        screenWidth  = GetScreenWidth();      // El tamaño de screenWidth es actualizado
        screenHeight = GetScreenHeight();     // El tamaño de screenHeight es actualizado

        switch (oldCurrentScreen)    // Vuelve a ejecutar un switch para buscar cual es la ventana actual
        {
            case LOGO:            // En caso de que la ventana sea LOGO
                screenLogoDraw(currentScreen);
                break;
            case CONFIGURATION:   // En caso de que la ventana sea CONFIGURATION
                screenConfigDraw(currentScreen, inputEmpty, invalidIp, errorUpdating, errorConfig);
                break;
            case MAINMENU:        // En caso de que la ventana sea MAINMENU
                screenMainmenuDraw(currentScreen);
                break;
            case ADMINMENU:       // En caso de que la ventana sea ADMINMENU
                screenAdminmenuDraw(currentScreen,
                                    invalidCredentials, inputEmpty, invalidIp,
                                    adminAuthenticated, successfulPdfCreation,
                                    explorarFinalOutput, modeInput, outResultsMode);
                break;
            case CONFIRMATION:    // En caso de que la ventana sea CONFIRMATION
                screenConfirmationDraw(currentScreen, existstudent, correctstudent, studentName);
                break;
            case VOTATION:        // En caso de que la ventana sea VOTATION
                screenVotationDraw(currentScreen, votoBlanco);
                break;
            case ENDING:          // En caso de que la ventana sea ENDING
                screenEndingDraw(currentScreen, verifyvote);
                break;
            default: break;
        }
        EndDrawing();             // Termina el "dibujado"
        oldCurrentScreen = currentScreen;       /* Se actualiza el valor de oldCurrentScreen con el valor real actual | Se utiliza esta variable para crear transiciones entre las pantallas, ya que si se usa la variable currentScreen y no
                                                   oldCurrentScreen el programa llegará a este switch e inmediatamente mostrará el frontend de la pantalla a la que se ha cambiado, sin permitir mostrar la transición de la pantalla anteior  */
    }

    // ── Cleanup ───────────────────────────────────────────────────────────────
    for (char* ptr : configurations) free(ptr);   // Libera los punteros en el vector de configurations para evitar memory leaks
    UnloadFont(fontTtf);      // Libera la memoria a la que apuntaba la variable fontTtf
    CloseWindow();            // Cierra la ventana de raylib
    mysql_close(conn);        // Cierra la conexión MySQL primaria del programa
    mysql_close(auth);        // Cierra la conexión MySQL de la pestaña "Terminal" del panel del administrador
    conn = nullptr;           // Declara valores nulos a los punteros de MySQL
    auth = nullptr;           // para limpiarlos y evitar memory leaks
    mysql_library_end();      // Llama a esta función de MySQL para también limpiar la mayor cantidad de memoria posible
#ifdef _WIN32           // En caso de que sea windows...
    WSACleanup();       // Limpia el uso de winsock2.h y liberar los recursos que utilizó el programa
#endif
    return 0;   // Devuelve 0 | Código de estado exitoso | Fín ideal del programa
}
