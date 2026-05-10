#include <chrono>
#include <thread>
#include "../globals.hpp"
#include "screen_logo.hpp"
#include "../config/config.hpp"
#include "../db/database.hpp"
#include "../ui/objects.hpp"
#include "../ui/drawing.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

// BACKEND DE LA PANTALLA  LOGO

void screenLogoUpdate(Screen& currentScreen, bool& errorConfig, bool& errorUpdating)      // Es función void ya que no retorna nada cuando termina, necesita a currentScreen, errorConfig y errorUpdating para modificar sus valores a nivel global
{
    framesCounter++;              // framesCounter actúa como contador, los FPS (Frames Per Second) del programa son 60, 1 segundo es igual a 60 FPS, entonces 120 son 2 segundos
    if (framesCounter > 300)      // Cuando ya pasan 120 FPS (2 segundos), el programa entra a este if
    {
        framesCounter = 0;        // Reinicia framesCounter, ya que se usará para otros escenarios
        configureData();          // Carga la configuración del archivo de configuración, el cual debe encontrarse en la carpeta actual, llamado ".config"

        if (statusCodeConfig != 0)                // Si el estado del archivo de configuración NO es igual a 0, implicaría que no ocurrió una configuración exitosa
        {
            if (statusCodeConfig == 1) {          // Código de estado 1 significa que el archivo de configuración inicialmente no fue encontrado, pero se crea el predeterminado
                loadConfig();                     // Carga a las barras de entrada de la pantalla CONFIGURATION los argumentos que encontró del archivo de configuración predeterminado
            }
            errorConfig   = true;                 // Indica que hubo un error de configuración
            currentScreen = CONFIGURATION;        // La pantalla del programa será ahora la de CONFIGURATION, para corregir los problemas de la configuración
            return;                               // Retorno de la función backend
        }

        /* Si el bloque de código del if anterior no ocurrió ya que la configuración fue exitosa, no se ejecutó un retorno, entonces el programa puede seguir con estas líneas,
           recordemos que en todas las funciones, siempre que llegan a la línea de código en la que hay un return, se terminará la función sin importar si falta código por ejecutar */

        statusCodeUpdating = updateData();        // updateData() se encarga de actualizar todos los datos que se necesitan cargar de la base de datos, y en caso de que ocurra un error, el código de error queda almacenado
        objectCreation();                         // Se encarga de la creación de absolutamente todos los objetos (los botones, barras de tareas, etc) a partir de sus clases

        if (statusCodeUpdating != 0)              // Si SÍ ocurre algún error en la actualización de los datos cargados desde la base de datos...
        {
            // Otro archivo que se utiliza por parte del programa es logs.txt, el cual se encuentra en la carpeta actual del programa, como .config, este archivo sirve para registrar errores y proporcionar más error sobre ellos
            fs::path pathToLogs = fs::current_path() / "logs.txt";                                      // pathToLogs almacenará la ruta en el sistema para acceder al archivo de los logs
            std::ofstream logFile(pathToLogs, std::ios::app);                                           // "app" del std::ios::app significa "append", esto permite que se pueda escribir al archivo sin que se borre su contenido
            logFile << "[ " << std::chrono::system_clock::now() << " ]  | >>> " << outQuery << "\n";    // Se escribe al archivo de los logs el contenido en formato "[ HORA ] | ERROR \n"
            logFile.close();                                                                            // Se cierra el archivo logs.txt

            loadConfig();                         // Como ocurrió un error, se enviará a la pantalla CONFIGURATION, por lo cual es ideal que se carguen las barras de entrada con el contenido que se cargó del archivo .config
            errorUpdating = true;                 // Indica que hubo un error de configuración
            currentScreen = CONFIGURATION;        // La pantalla del programa será ahora la de CONFIGURATION, para corregir los problemas de la configuración
            return;                               // Retorno de la función backend
        }

        /* Si el programa llega a este punto, significa que ninguno de los dos if anteriores se lograron ejecutar, ya que ambos al final de sus bloques de código, retornan
           Como ninguno de los if anteriores se ejecutaron en este punto, significa que no hubo errores ni en la configuración ni en la carga de los datos de la base de datos, entonces el programa va a la pantalla principal*/
        currentScreen = MAINMENU;                 // La pantalla del programa será ahora la de MAINMENU, esto significaría que ocurrió una configuración y carga de los datos exitosa
    }
}

// FRONTEND DE LA PANTALLA LOGO
void screenLogoDraw(Screen& currentScreen)
{
    if (currentScreen == LOGO) transition("show");
    if (currentScreen != LOGO && alphaIsZero == false)
    {
        transition("hide");
        screenLogoDraw(currentScreen);
    }
}
