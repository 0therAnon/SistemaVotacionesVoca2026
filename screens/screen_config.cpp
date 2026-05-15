#include "../globals.hpp"
#include "screen_config.hpp"
#include "../platform/ipvalid.hpp"
#include "../config/config.hpp"
#include "../db/database.hpp"
#include "../ui/objects.hpp"
#include "../ui/input.hpp"
#include "../ui/drawing.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <string.h>

// Esta función se encarga del backend de la pantalla CONFIGURATION

void screenConfigUpdate(Screen& currentScreen,            // La función necesita el valor de la pantalla actual
                        bool& errorConfig,                // Necesita la variable que verifica si hubo un error en la función configureData()
                        bool& errorCreating,              // Necesita la variable que verifica si hubo un error en la función objectCreation()
                        bool& errorUpdating,              // Necesita la variable que verifica si hubo un error en la función updateData()
                        bool& invalidIp,                  // Necesita saber si hubo un error con la IP introducida
                        bool& inputEmpty,                 // Necesita saber si hay datos de entrada vacíos
                        bool& fromAdmin)                  // Variable que sirve para saber si se llegó a entrar al panel de configuración desde la pantalla ADMINMENU y no desde LOGO
{
    if (IsWindowResized()) loadConfig();                  /* Si la ventana cambia de tamaño, se llamará a objectCreation() para crear los objetos de nuevo y modificar sus tamaños, así que también se debe cargar la configuración
                                                             en las barras de entrada automáticamente para que no se pierdan los datos */

    if (IsGestureDetected(GESTURE_TAP))                   // Si se percibe un clic...
    {
        // Seleccion de pestaña de configuracion

        for (int i = 0; i < (int)configbuttons.size(); i++)         // Recorrerá todas las pestañas de la pantalla CONFIGURATION
        {
            configbuttons[i]->status = isPressed(configbuttons[i]);                             // Se actualiza el estado de cada pestaña
            if (configbuttons[i]->status == 4) configSelected = configbuttons[i]->name;         // Y si la pestaña actual en el bucle fue la que recibió el clic entonces actualizará a configSelected para que almacene la nueva pestaña seleccionada
        }

        // Botón saveConfig, el cual se encarga de guardar la configuración en el archivo de configuración, y de paso cargar la nueva configuración al programa

        saveConfigPtr->status = isPressed(saveConfigPtr);
        if (saveConfigPtr->status == 4)                  // Si se detecta que el botón saveConfig fue presionado...
        {
            int emptyValues = 0;        // Almacena la cantidad de barras que se encuentran vacías
            for (int v = 0; v < (int)extraBars.size(); v++)             // Recorrerá todas las barras de entrada pertenecientes a extraBars (pestaña "Extra")
                if (extraBars[v]->input32.empty()) emptyValues++;       // Si hay una barra vacía, se sumará un valor a emptyValues
            for (int v = 0; v < (int)pathBars.size(); v++)              // Recorrerá todas las barras de entrada pertenecientes a pathBars (pestaña "Paths")
                if (pathBars[v]->input32.empty()) emptyValues++;        // Si hay una barra vacía, se sumará un valor a emptyValues

             // La comprobación de las barras termBars debe ser manual, ya que solo la barra de password puede estar vacía, a pesar de que las demás no

            if (!termBars[0]->input.empty() && !termBars[1]->input.empty() &&       // Se comprueba de que solo la barra password
                !termBars[2]->input.empty() && !termBars[3]->input.empty() &&       // de termBars sea la única vacía, y que ninguna de las demás no
                !admPasswordBarPtr->input.empty() && emptyValues == 0)              // además de verificar de que emptyValues sea igual a 0, si no es igual a 0, alguna barra estaría vacía y esto no sería correcto para ejecutar lo de este if
            {
                if (validIP(termBars[0]->input))                                    // Se valida de que la IP sea válida, si llega a ser válida, procederá con el if
                {
                    // Se realiza la carga de la configuración digitada en cada barra a las variables del programa
                    free(configurations[0]);  configurations[0]  = strdup(termBars[0]->input.data());         // IP
                    free(configurations[1]);  configurations[1]  = strdup(termBars[3]->input.data());         // Usuario
                    free(configurations[2]);  configurations[2]  = strdup(termBars[4]->input.data());         // Contraseña
                    free(configurations[3]);  configurations[3]  = strdup(termBars[2]->input.data());         // BD
                    free(configurations[4]);  configurations[4]  = strdup(termBars[1]->input.data());         // Puerto
                    free(configurations[5]);  configurations[5]  = strdup(admPasswordBarPtr->input.data());   // Contraseña admin
                    free(configurations[6]);  configurations[6]  = strdup(labNameBarPtr->input.data());       // Nombre de laboratorio
                    free(configurations[7]);  configurations[7]  = strdup(extraBars[0]->input.data());        // Tabla Estudiantes
                    free(configurations[8]);  configurations[8]  = strdup(extraBars[1]->input.data());        // Tabla Partidos
                    free(configurations[9]);  configurations[9]  = strdup(extraBars[4]->input.data());        // Columna Voto
                    free(configurations[10]); configurations[10] = strdup(extraBars[5]->input.data());        // Columna Votos
                    free(configurations[11]); configurations[11] = strdup(extraBars[6]->input.data());        // Columna Laboratorios
                    free(configurations[12]); configurations[12] = strdup(extraBars[2]->input.data());        // Columna Nombre partidos
                    free(configurations[13]); configurations[13] = strdup(extraBars[3]->input.data());        // Partido nulo
                    free(configurations[14]); configurations[14] = strdup(pathBars[0]->input.data());         // Font programa
                    free(configurations[15]); configurations[15] = strdup(pathBars[1]->input.data());         // Font outLogs/terminal
                    free(configurations[16]); configurations[16] = strdup(pathBars[2]->input.data());         // Font PDF
                    free(configurations[17]); configurations[17] = strdup(pathBars[3]->input.data());         // Nombre PDF

                    statusCodeUpdating = updateData();                        // Se actualiza la información de la base de datos y se verifica que la configuración sea correcta, el código de estado de la función se guarda en statusCodeUpdating
                    std::string oldConfigSelected = configSelected;           // Se guarda la última pestaña actual dentro de oldConfigSelected, esto para evitar de que el reinicio de los objetos hagan que la pestaña actual cambie
                    statusCodeCreating = objectCreation();
                    if (statusCodeCreating == 10)                             // Se vuelven a crear los objetos, debido a que deben de ser actualizados dependiendo del código de estado que haya devuelto updateData()
                    {                                                         // Sin embargo, si el código de estado de objectCreation es igual a 10, significa que hubo un error al cargar las banderas de los partidos, entonces...
                        errorCreating = true;
                        fs::path pathToLogs = fs::current_path() / "logs.txt";                                                                    // Se busca el archivo logs.txt para escribir el error que hubo
                        std::ofstream logFile(pathToLogs, std::ios::app);                                                                         // El archivo logs.txt se abre en modo de escritura y se guarda en logFile
                        logFile << "[ " << std::chrono::system_clock::now() << " ]  | >>> Error al cargar las banderas de los partidos\n";        // Se escribe en el siguiente formato el error: "[ hora ] | >>> error"
                        logFile.close();                                                                                // Se cierra el archivo
                        loadConfig();                                                                                   // Se procede a cargar a las barras de entrada la configuraación previa
                    }
                    configSelected = oldConfigSelected;                       // Se reasigna configSelected al valor que tenía previamente

                    if (statusCodeUpdating == 0 && statusCodeCreating == 0)                           // Si el updateData() NO tuvo errores, procederá a escribir en el archivo de configuración la configuración que está usando el programa
                    {
                        fs::path pathToConfig = fs::current_path() / ".config";                       // Busca al archivo .config en la carpeta actual, hay que recordar que este archivo es el archivo de configuración
                        std::ofstream cfgFile(pathToConfig);                                          // Se procede a guardar en modo escritura la variable que almacena la ruta del archivo de confiuración
                        std::string outCredentials =                                                  // Se empiezan a crear los strings que se escribirán en el archivo de configuración
                            "[Credentials]\nserver="s + *server + "\nuser="s + *user +
                            "\npassword="s + *password + "\ndatabase="s + *database +
                            "\nport="s + *port + "\nadmPassword="s + *admPassword + "\nlabName="s + *labName + "\n"s;
                        std::string outExtra =
                            "\n[Extra]\nnameColumnPartidosNombre="s + *nameColumnPartidosNombre +
                            "\nnameNuloOpcion="s + *nameNuloOpcion +
                            "\nnameColumnVotosNombre="s + *nameColumnVotosNombre +
                            "\nnameColumnVotoNombre="s + *nameColumnVotoNombre +
                            "\nnameColumnLabsNombre="s + *nameColumnLabsNombre +
                            "\nnameTableEstudiantes="s + *nameTableEstudiantes +
                            "\nnameTablePartidos="s + *nameTablePartidos + "\n"s;
                        std::string outPaths =
                            "\n[Paths]\npathProgramFont="s + *pathProgramFont +
                            "\npathLogsFont=" + *pathLogsFont +
                            "\npathPdfFont="s + *pathPdfFont +
                            "\ninformeName="s + *informeName + "\n"s;
                        cfgFile << outCredentials + outExtra + outPaths << std::endl;                 // Se escriben los strings dentro del archivo de configuracion
                        cfgFile.close();                                                              // Se cierra el archivo
                        currentScreen = fromAdmin ? ADMINMENU : MAINMENU;                             // Y si la pantalla CONFIGURATION se cargó desde ADMINMENU, procederá a nombrar a currentScreen como ADMINMENU, sino, como MAINMENU
                        fromAdmin = false;                                                            // fromAdmin se declara false, ya que se debe de resetear la variable por que no se necesita más
                    }
                    else                                                    // En caso de que updateData() SÍ tuvo errores...
                    {
                        errorUpdating = true;                                                                           // Cargará a errorUpdating como true, esta variable verifica si hubo errores en la actualización de los datos
                        fs::path pathToLogs = fs::current_path() / "logs.txt";                                          // Se busca el archivo logs.txt para escribir el error que hubo
                        std::ofstream logFile(pathToLogs, std::ios::app);                                               // El archivo logs.txt se abre en modo de escritura y se guarda en logFile
                        logFile << "[ " << std::chrono::system_clock::now() << " ]  | >>> " << outQuery << "\n";        // Se escribe en el siguiente formato el error: "[ hora ] | >>> error"
                        logFile.close();                                                                                // Se cierra el archivo
                        loadConfig();                                                                                   // Se procede a cargar a las barras de entrada la configuraación previa
                    }
                }
                else { invalidIp = true; }            // Si la IP digitada era inválida, declarará a invalidIp como verdadero, para posteriormente enviar un mensaje sobre esto en el frontend
            }
            else { inputEmpty = true; }               // Si alguna barra se encuentra vacía, inputEmpty se declara como verdadero, para posteriormente enviar un mensaje sobre esto en el frontend

        }
    }

    // Procesamiento de barras de entrada segun pestaña activa

    if (configSelected == configbuttons[0]->name)   // Si la pestaña activa es "Credenciales"...
    {
        for (int b = 0; b < (int)termBars.size(); b++)      // Recorrerá cada barra de la pestaña actual
        {
            if (IsKeyPressed(KEY_TAB))                      // Si se presiona la tecla TAB, procederá a cambiar entre barras | El funcionamiento es casi idéntico a como las funciones TAB en la pestaña "Terminal" de screen_adminmenu.cpp
            {
                if (termBars[b]->status > 1 && b + 1 < (int)termBars.size())            // Si la barra actual tiene un estado mayor a 1, es decir, se está interactuando con la barra directamente, y la barra es menor al total de barras, entonces...
                {
                    termBars[b]->status = 0;                                            // La barra actual pasa a tener un estado igual a cero, ya que el punto de usar TAB es cambiar de barras
                    termBars[b + 1]->status = 2;                                        // Y la barra siguiente pasa a tener un estado igual a 2, lo suficiente como para poder recibir datos de entrada
                    beam = 0; break;                                                    // Ahora, el parpadeo de la barra parpadeante se reinicia a cero, y se rompe el bucle
                }
                else if (termBars[b]->status > 1 && b + 1 == (int)termBars.size())      // En caso de que la barra actual tenga un estado mayor a 1, es decir, se está interactuando directamente, y también es igual a la última barra, entonces...
                {
                    termBars[b]->status = 0;                                            // La barra actual procede a tener un estado igual a cero, para no seguir interactuando más con ella
                    admPasswordBarPtr->status = 2;                                      // Y, ahora se interactuará con admPasswordBar, tiene que pasar a esta barra por que en este caso la última barra está fuera del vector termBars
                    beam = 0; break;                                                    // El parpadeo se reinicia a cero y se rompe el bucle
                }
                else if (admPasswordBarPtr->status > 1)                                 // En caso de que admPasswordBar tenga un estado mayor a 1, es decir, se está interactuando directamente, entonces...
                {
                    admPasswordBarPtr->status = 0;                                      // La barra admPasswordBar pasa a estado cero para dejar de interactuar con ella
                    labNameBarPtr->status = 2;                                          // La barra labNameBar pasa a ser la barra activa
                    beam = 0; break;                                                    // El parpadeo se reinicia a cero y se rompe el bucle
                }
                else if (labNameBarPtr->status > 1)                                     // En caso de que labNameBar tenga un estado mayor a 1, es decir, se está interactuando directamente, entonces...
                {
                    labNameBarPtr->status = 0;                                          // La barra labNameBar pasa a estado cero para dejar de interactuar con ella
                    termBars[0]->status = 2;                                            // La primera barra del vector pasa a ser la barra activa
                    beam = 0; break;                                                    // El parpadeo se reinicia a cero y se rompe el bucle
                }
            }
            else termBars[b]->status = isPressed(termBars[b]);                          // En caso de que no se presione la tecla TAB, entonces solo estará vigilando el estado de la barra actual

            if (termBars[b]->status > 1)                                                // Y si la barra actual está interactuandose directamente...
            {
                if (termBars[b]->name != termBars[1]->name)                                         // Si la barra NO es igual al índice uno de termBars (termBars[1] es el puerto) entonces...
                    inputfunc("backend", termBars[b], 45, "allchars", littleFontSize, BLANCO);       // El modo de entrada será allchars
                else                                                                                // En caso de que la barra sí sea igual al índice uno de termBars, entonces...
                    inputfunc("backend", termBars[b], 45, "numberonly", littleFontSize, BLANCO);     // Solo recibirá números con el modo de entrada numberonly
            }
        }
        admPasswordBarPtr->status = isPressed(admPasswordBarPtr);         // Como admPasswordBar está fuera del vector termBars, hay que verificar su estado fuera del bucle
        labNameBarPtr->status = isPressed(labNameBarPtr);                 // labNameBar está fuera del vector termBars, hay que verificar su estado fuera del bucle
        if (admPasswordBarPtr->status > 1)                                // Si se está interactuando con la barra, entonces...
            inputfunc("backend", admPasswordBarPtr, 25, "allchars", littleFontSize, BLANCO);       // Se recibirán datos de entrada de la barra actual
        if (labNameBarPtr->status > 1)                                // Si se está interactuando con la barra, entonces...
            inputfunc("backend", labNameBarPtr, 25, "allchars", littleFontSize, BLANCO);       // Se recibirán datos de entrada de la barra actual
    }
    else if (configSelected == configbuttons[1]->name)  // Si la pestaña activa es "Extra"...
    {
        for (int b = 0; b < (int)extraBars.size(); b++)
        {
            if (IsKeyPressed(KEY_TAB))
            {
                if (extraBars[b]->status > 1 && b + 1 < (int)extraBars.size())          // Si la barra actual tiene un estado mayor a 1, es decir, se está interactuando con la barra directamente, y la barra es menor al total de barras, entonces...
                {
                    extraBars[b]->status = 0;                                           // La barra actual pasa a tener un estado igual a cero, ya que el punto de usar TAB es cambiar de barras
                    extraBars[b + 1]->status = 2;                                       // Y la barra siguiente pasa a tener un estado igual a 2, lo suficiente como para poder recibir datos de entrada
                    beam = 0; break;                                                    // El parpadeo se reinicia a cero y rompe el bucle
                }
                else if (extraBars[b]->status > 1 && b + 1 == (int)extraBars.size())    // En caso de que la barra actual tenga un estado mayor a 1, es decir, se está interactuando directamente, y también es igual a la última barra, entonces...
                {
                    extraBars[b]->status = 0;                                           // La barra actual procede a tener un estado igual a cero, para no seguir interactuando más con ella
                    extraBars[0]->status = 2;                                           // El estado de la siguiente barra cambiará a 2, suficiente para activarse y recibir datos de entrada
                    beam = 0; break;                                                    // El parpadeo se reinicia a cero y rompe el bucle
                }
            }
            else extraBars[b]->status = isPressed(extraBars[b]);                        // En caso de que NO se presione la tecla TAB, solo estará vigilando el estado de cada barra

            if (extraBars[b]->status > 1)                                               // Si el estado de la barra actual es superior a 1, es decir, se está interactuando con esa barra
                inputfunc("backend", extraBars[b], 45, "allchars", littleFontSize, BLANCO);        // Se recibirán datos de entrada de la barra actual
        }
    }
    else if (configSelected == configbuttons[2]->name)  // Si la pestaña activa es "Paths"...
    {
        for (int b = 0; b < (int)pathBars.size(); b++)
        {
            if (IsKeyPressed(KEY_TAB))
            {
                if (pathBars[b]->status > 1 && b + 1 < (int)pathBars.size())            // Si la barra actual tiene un estado mayor a 1, es decir, se está interactuando con la barra directamente, y la barra es menor al total de barras, entonces...
                {
                    pathBars[b]->status = 0;                                            // La barra actual pasa a tener un estado igual a cero, ya que el punto de usar TAB es cambiar de barras
                    pathBars[b + 1]->status = 2;                                        // Y la barra siguiente pasa a tener un estado igual a 2, lo suficiente como para poder recibir datos de entrada
                    beam = 0; break;                                                    // El parpadeo se reinicia a cero y rompe el bucle
                }
                else if (pathBars[b]->status > 1 && b + 1 == (int)pathBars.size())      // En caso de que la barra actual tenga un estado mayor a 1, es decir, se está interactuando directamente, y también es igual a la última barra, entonces...
                {
                    pathBars[b]->status = 0;                                            // La barra actual procede a tener un estado igual a cero, para no seguir interactuando más con ella
                    pathBars[0]->status = 2;                                            // El estado de la siguiente barra cambiará a 2, suficiente para activarse y recibir datos de entrada
                    beam = 0; break;                                                    // El parpadeo se reinicia a cero y rompe el bucle
                }
            }
            else pathBars[b]->status = isPressed(pathBars[b]);                          // En caso de que NO se presione la tecla TAB, solo estará vigilando el estado de cada barra

            if (pathBars[b]->status > 1)                                                // Si el estado de la barra actual es superior a 1, es decir, se está interactuando con esa barra
                inputfunc("backend", pathBars[b], 45, "allchars", mediumFontSize, BLANCO);         // Se recibirán datos de entrada de la barra actual
        }
    }
}

// Frontend de CONFIGURATION

void screenConfigDraw(Screen &currentScreen,
                      bool &inputEmpty,                 // Estas variables sirven principalmente
                      bool &invalidIp,                  // para saber si ocurrieron errores
                      bool &errorUpdating,              // en alguna parte del backend y mostrar
                      bool &errorCreating,              // los errores que ocurrieron y
                      bool &errorConfig)                // algun mensaje en este frontend
{
    if (currentScreen == CONFIGURATION) transition("show");
    DrawRectangle(configPanel[0], adminPanel[1],
                  configPanel[2], configPanel[3], VOCADORADOSUAVE);    // Usa configPanel para dibujarse independientemente del panel de administración
    DrawLineEx((Vector2){configPanel[0], adminPanel[1]},
               (Vector2){(configPanel[0]+configPanel[2]), adminPanel[1]},
                4.0f, DORADO_BORDE);
    DrawTextEx(fontTtf, "PANEL DE CONFIGURACIÓN"s.data(),                                         // Procede a escribir el título "Panel de Configuración"
               (Vector2){(float)(centertext("PANEL DE CONFIGURACIÓN"s, screenWidth, fontSize)),
                          (float)(screenHeight * 0.03)},
               fontSize, 2, COLORTEXTO);                                                               // BLANCO porque el título flota sobre el fondo negro

    // Se actualiza el estado de cada botón de pestaña cada frame para detectar hover correctamente
    for (int i = 0; i < (int)configbuttons.size(); i++)
        configbuttons[i]->status = isPressed(configbuttons[i]);

    drawSelected(configbuttons, littleFontSize, configSelected);                     // Dibuja las pestañas de CONFIGURATION
    if (configSelected == configbuttons[0]->name)                                     // Si la pestaña actual es "Credenciales"
    {
        for (int b = 0; b < (int)termBars.size(); b++)                                // Recorrerá cada barra de datos de entrada...
        {
            DrawTextEx(fontTtf, termBars[b]->name.data(),                             // Dibujará el nombre de la barra
                       (Vector2){(float)screenWidth * 0.15f,
                                  (float)termBars[b]->yloc + ((termBars[b]->ysize * 0.5f) - (littleFontSize * 0.5f))},
                       littleFontSize, 0, NEGRO);
            PrettyDrawRectangle(termBars[b]);                                         // Y dibujará la barra misma
            inputfunc("frontend", termBars[b], 0, "allchars", littleFontSize);        // Además del contenido que tiene
        }
        DrawTextEx(fontTtf, admPasswordBarPtr->name.data(),                           // Como admPasswordBar es una barra independiente, hay que dibujar su nombre fuera del bucle
                   (Vector2){(float)screenWidth * 0.15f,
                              (float)admPasswordBarPtr->yloc + ((admPasswordBarPtr->ysize * 0.5f) - (littleFontSize * 0.5f))},
                   littleFontSize, 0, NEGRO);
        PrettyDrawRectangle(admPasswordBarPtr);                                       // Llamar a PrettyDrawRectangle para que dibuje a la barra misma
        inputfunc("frontend", admPasswordBarPtr, 0, "allchars", littleFontSize);      // Y llamar a inputfunc() para que dibuje su contenido
        DrawTextEx(fontTtf, labNameBarPtr->name.data(),                               // Como labNameBar es una barra independiente, hay que dibujar su nombre fuera del bucle
                   (Vector2){(float)screenWidth * 0.15f,
                              (float)labNameBarPtr->yloc + ((labNameBarPtr->ysize * 0.5f) - (littleFontSize * 0.5f))},
                   littleFontSize, 0, NEGRO);
        PrettyDrawRectangle(labNameBarPtr);                                           // Llamar a PrettyDrawRectangle para que dibuje a la barra misma
        inputfunc("frontend", labNameBarPtr, 0, "allchars", littleFontSize);          // Y llamar a inputfunc() para que dibuje su contenido
    }
    else if (configSelected == configbuttons[1]->name)                                // Si la pestaña actual es "Extra"
    {
        for (int b = 0; b < (int)extraBars.size(); b++)                               // Recorrerá cada barra de datos de entrada...
        {
            DrawTextEx(fontTtf, extraBars[b]->name.data(),                            // Dibujará el nombre de cada barra
                       (Vector2){(float)screenWidth * 0.15f,
                                  (float)extraBars[b]->yloc + ((extraBars[b]->ysize * 0.5f) - (littleFontSize * 0.5f))},
                       littleFontSize, 0, NEGRO);
            PrettyDrawRectangle(extraBars[b]);                                        // Y dibujará la barra misma
            inputfunc("frontend", extraBars[b], 0, "allchars", littleFontSize);       // Además del contenido que tiene
        }
    }
    else if (configSelected == configbuttons[2]->name)                                // Si la pestaña actual es "Paths"
    {
        for (int b = 0; b < (int)pathBars.size(); b++)                                // Recorrerá cada barra de datos de entrada...
        {
            DrawTextEx(fontTtf, pathBars[b]->name.data(),                             // Dibujará el nombre de cada barra
                       (Vector2){(float)screenWidth * 0.15f,
                                  (float)pathBars[b]->yloc + ((pathBars[b]->ysize * 0.5f) - (mediumFontSize * 0.5f))},
                       mediumFontSize, 0, NEGRO);
            PrettyDrawRectangle(pathBars[b]);                                         // Y dibujará la barra misma
            inputfunc("frontend", pathBars[b], 0, "allchars", mediumFontSize);        // Además del contenido que tiene
        }
    }
    saveConfigPtr->status = isPressed(saveConfigPtr);                                 // Se actualiza el estado del botón cada frame para detectar hover correctamente
    PrettyDrawRectangle(saveConfigPtr);                                               // Dibuja al final el botón de guardar la configuración
    DrawTextEx(fontTtf, saveConfigPtr->name.data(),                                   // Y dibuja el nombre de ese botón también
               (Vector2){saveConfigPtr->xloc + (float)centertext(saveConfigPtr->name, saveConfigPtr->xsize, mediumFontSize),
                          saveConfigPtr->yloc + (float)((saveConfigPtr->ysize - mediumFontSize) / 2)},
               mediumFontSize, 0, NEGRO);

    // Estas líneas sirven para mostrar los mensajes en caso de algún error

    if (inputEmpty) {shortmessage("Los datos se encuentran vacios", fontSize, inputEmpty);}           // Si inputEmpty se activa, es por que una barra se encuentra vacía, entonces llamará a shortmessage() para mostrar el mensaje escrito
    else if (invalidIp) {shortmessage("La IP digitada es invalida", fontSize, invalidIp);}            // Si invalidIp se activa, es por que una IP digitada es inválida, entonces llamará a shortmessage() para mostrar el mensaje escrito
    else if (errorUpdating || errorConfig || errorCreating)            // Si errorUpdating se activa, es por algun error en la configuracion, como también errorConfig, entonces...
    {
        std::string errorMessage = "";    // Define a errorMessage, que será el string que almacenará alguno de los siguientes mensajes

        /* Se comparan los códigos de estado tanto los de statusCodeUpdating como los de statusCodeConfig, estos códigos de estado son los que la función de cada uno retorna,
           si no son iguales a cero, significa que hubo un error, y aquí  está el significado de cada código de estado de cada función */

        if      (statusCodeUpdating == -1)  errorMessage = "Por favor, escriba el nombre del laboratorio actual";
        else if (statusCodeUpdating == 1)   errorMessage = "Error en la conexión a la base de datos";
        else if (statusCodeUpdating == 3)   errorMessage = "Error con el nombre de la base de datos";
        else if (statusCodeUpdating == 6)   errorMessage = "Error en el nombre de la tabla de los estudiantes";
        else if (statusCodeUpdating == 7)   errorMessage = "Error en el nombre de la tabla de los partidos";
        else if (statusCodeUpdating == 8)   errorMessage = "Error en el nombre de la columna de los votos de los estudiantes";
        else if (statusCodeUpdating == 9)   errorMessage = "Error en el nombre de la columna de los votos de los partidos";
        else if (statusCodeUpdating == 10)  errorMessage = "Error en el nombre de la columna que almacena el nombre de los partidos";
        else if (statusCodeUpdating == 11)  errorMessage = "Error en el nombre de la opcion a votar nulo";
        else if (statusCodeUpdating == 12)  errorMessage = "Error en el nombre de la columna de los laboratorios de los estudiantes";
        else if (statusCodeUpdating == 13)  errorMessage = "Error en la ruta de la fuente de la letra del programa";
        else if (statusCodeUpdating == 14)  errorMessage = "Error en la ruta de la fuente de la letra monoespaciada";
        else if (statusCodeUpdating == 15)  errorMessage = "Error en la ruta de la fuente de la letra para el PDF";
        else if (statusCodeUpdating == 20)  errorMessage = "Error desconocido";
        else if (statusCodeUpdating == 127) errorMessage = "Error | La IP del servidor no existe";
        else if (statusCodeCreating == 10)  errorMessage = "Error al cargar las banderas de los partidos";
        else if (statusCodeConfig == 1)     errorMessage = "Error | No se encontro el archivo de configuracion";
        else if (statusCodeConfig == 2)     errorMessage = "Error | Faltaron parametros en el archivo de configuracion";
        if (errorConfig)   shortmessage(errorMessage, mediumFontSize, errorConfig, 450);          // Si errorConfig era el que se encontraba en true, llamará a la función shortmessage con errorConfig entre los argumentos
        else if(errorUpdating)  shortmessage(errorMessage, mediumFontSize, errorUpdating, 450);   // en caso de que sea errorUpdating, llamará a errorUpdating
        else shortmessage(errorMessage, mediumFontSize, errorCreating, 450);                      // Como última opción, entonces llamará a errorCreating
    }
    if (currentScreen != CONFIGURATION && alphaIsZero == false)
    {
        transition("hide");
        screenConfigDraw(currentScreen, inputEmpty, invalidIp, errorUpdating, errorCreating, errorConfig);
    }
}
