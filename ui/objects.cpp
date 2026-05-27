#include "../globals.hpp"
#include "objects.hpp"
#include "../db/database.hpp"
#include "../ui/drawing.hpp"
#include <memory>
#include <string>
#include <fstream>                  // Llamada a la librería fstream, la cual permite la lectura de archivos

// Esta función a continuación, tiene como propósito la creación de todos los objetos del programa, desde objetos simples como los botones, hasta objetos más especiales como los objetos de tipo column, que se cargan desde la base de datos

int objectCreation()
{

    // Procedo a limpiar todos los vectores que almacenan los objetos del programa, esto para prevenir de que si la llamaba de esta función ocurre más de una vez, no guarde objetos duplicados en cada vector

    while (!lastLogs.empty())    lastLogs.pop_back();                   // Es el vector que almacena los últimos logs de cada botón, ya que en esta función los botones serán reseteados, entonces almacenará los registros de cada uno
    if (!adminButtons.empty())                                          // En caso de que el vector adminButtons NO esté vacío, es decir, se llamó a objectCreation para actualizar los objetos, entonces...
        for (auto btn : adminButtons)                                   // Procederá a recorrer todos los botones del panel de administración
            lastLogs.push_back(btn->outLog);                            // y almacenará todos los logs de cada botón
    else                                                                // En caso de que el vector adminButtons SÍ esté vacío, es decir, es la primera vez en la que se van a crear los botones en el programa, entonces...
        for (int i = 0; i < 7; i++)                                     // Procederá a usar un bucle que correrá 7 veces
            lastLogs.push_back("");                                     // y el propósito de ese bucle será añadir simplemente un string vacío, ya que los strings de lastLogs se usarán para nombrar las propiedades outLog de cada botón

    while (!adminObj.empty())    adminObj.pop_back();                   // Es el vector padre, este vector almacena absolutamente todos los objetos del programa
    while (!tablesVec.empty())   tablesVec.pop_back();                  // Es el vector que almacena las tablas cargadas de la base de datos
    while (!partidosVec.empty()) partidosVec.pop_back();                // Es el vector que almacena los partidos cargados de la base de datos
    while (!columnsVec.empty())  columnsVec.pop_back();                 // Es el vector que almacena todas las columnas cargadas de la base de datos
    while (!adminButtons.empty())     adminButtons.pop_back();          // Este vector tiene todas las pestañas del panel de administracion
    while (!configbuttons.empty()) configbuttons.pop_back();            // Este vector tiene todas las pestañas del panel de configuracion
    while (!termBars.empty())    termBars.pop_back();                   // Este vector almacena todas las barras que aparecen en "Terminal", también aparecen en la pestaña "Credenciales" del panel de configuracion
    while (!extraBars.empty())   extraBars.pop_back();                  // Este vector almacena todas las barras que aparecen en la pestaña "Extra" del panel de configuracion
    while (!pathBars.empty())    pathBars.pop_back();                   // Este vector almacena todas las barras que aparecen en la pestaña "Paths" del panel de configuracion
    while (!colorsVec.empty())   colorsVec.pop_back();                  // Este vector almacena todos los colores del programa
    while (!partidosVec.empty()) partidosVec.pop_back();      // Este vector almacena las banderas de los partidos

    colorsVec.push_back(&DORADO_BORDE);
    colorsVec.push_back(&VOCAVERDE);
    colorsVec.push_back(&VOCAVERDESUAVE);
    colorsVec.push_back(&VOCAAMARILLO);
    colorsVec.push_back(&VOCAAMARILLOSUAVE);
    colorsVec.push_back(&VOCADORADO);
    colorsVec.push_back(&VOCADORADOSUAVE);
    colorsVec.push_back(&NEGRO);
    colorsVec.push_back(&BLANCO);
    colorsVec.push_back(&COLORTEXTO);

    // warn es el cuadro de advertencia para cada vez que se ejecuta una query en la pestaña Agregar, Actualizar y Borrar en el panel de administración
    warn[0] = screenWidth / 3;                            // Declaro la ubicación en el eje X de la pantalla para posicionar el cuadro
    warn[1] = screenHeight * 0.30f;                       // Ubicación del eje Y
    warn[2] = (screenWidth  - warn[0]) * 0.5f;            // Declaro el tamaño en el eje X del cuadro
    warn[3] = (screenHeight - warn[1]) * 0.5f;            // Tamaño en eje Y

    // explorarSquare es el cuadro de fondo de la pestaña "Explorar" del panel de administracion, aquí defino las dimensiones del cuadro
    explorarSquare[0] = screenWidth  * 0.12;
    explorarSquare[1] = screenHeight * 0.24;
    explorarSquare[2] = screenWidth  * 0.76;
    explorarSquare[3] = screenHeight * 0.66;

    // adminPanel es el cuadro del panel de administracion, aquí defino las dimensiones del cuadro
    adminPanel[0] = 0.0;
    adminPanel[1] = screenHeight * 0.1;
    adminPanel[2] = screenWidth;
    adminPanel[3] = screenHeight * 0.9;

    // Panel de configuración con posición independiente del panel de administración
    configPanel[0] = 0.0;
    configPanel[1] = screenHeight * 0.13;  // Más arriba que adminPanel, se puede ajustar independientemente
    configPanel[2] = screenWidth;
    configPanel[3] = screenHeight * 0.9;

    /* A continuación, se iniciará la creación de los objetos, pero hay que primero entender esto:
       Siempre se le tiene que asignar un tipo auto a la hora de crear el objeto, y después de escribir auto, se le pasa como argumento el nombre del objeto
       Además, los objetos se deben de crear con un puntero, los punteros se crean por medio de std::make_unique y se le pasa de argumento el tipo de objeto que se desea crear, por ejemplo, si quiero crear un botón, iniciaría su creación así:

       auto botonCreado = std::make_unique<button>();

       Luego de esto, se le asignan las propiedades, que son las variables que tiene el objeto dependiendo de su clase, para tener más información sobre las clases, pueden leer el archivo ./ui/widgets.hpp
       Si la clase, por ejemplo, es de tipo button, el objeto button tiene las siguientes propiedades:

        name
        xloc
        yloc
        xsize
        ysize
        status
        highColor
        normalColor
        selfquery
        outLog
        oType

       Para yo modificar alguna propiedad, como por ejemplo la propiedad name, solo tengo que escribir esto:

       botonCreado->name = "Botón Creado";

       Se escribe con una flecha -> por que se apunta a la propiedad de un objeto que accede por medio de un puntero, en este caso, el objeto puntero sería botonCreado, y la propiedad sería name, entonces se accede por medio de una flecha

       Otro punto importante a considerar es el hecho de que al final de las declaraciones se nombra un puntero extra, el cual sirve para usarse en el resto del código para acceder al objeto, ya que el puntero original se guardará en adminObj
       La razón por la que todos los objetos se guardan en adminObj es por que ayuda a que todos los objetos sin necesidad de llamarlos uno por uno puedan ser eliminados en caso de que se tengan que actualizar propiedades como sus tamaños, y
       los objetos al guardarse en adminObj, la propiedad sobre ellos ahora la tiene adminObj, entonces se necesita otro puntero para acceder a ellos, el cual es el puntero extra, que siempre, por razones de orden, debe tener el siguiente formato:

       <nombre original del objeto>Ptr

        Ejemplo: botonCreadoPtr

       Ese puntero es preferible que se declare en globals.cpp para que el puntero pueda ser accedido de manera global, ya que la mayoría de estos objetos se necesitan acceder de forma global

       Después, se procederá a almacenar el objeto dentro de adminObj, comentaré algunas partes del código para aclarar esto en caso de que no haya quedado muy bien explicado  */

    // butnames es la lista que contiene los nombres de las pestañas del panel de administracion
    butnames[0] = "Consultar";
    butnames[1] = "Agregar";
    butnames[2] = "Actualizar";
    butnames[3] = "Borrar";
    butnames[4] = "Explorar";
    butnames[5] = "Resultados";
    butnames[6] = "Terminal";

    // Botones de las pestañas del panel de administracion
    for (int i = 0; i < 7; i++)                                     // Este es un bucle para crear varios botones que comparten ancho, alto y ubicación en el eje Y (altura) | Para no tener que crear todos uno por uno, uso un bucle
    {
        auto btn = std::make_unique<button>();                      // Declaro a btn, el puntero temporal para cada botón
        btn->name        = butnames[i];                             // Declaro que el nombre de btn, será según el índice en el bucle actual pero usandolo para acceder a un índice en butnames, por ejemplo, si i vale 0, el nombre será "Consultar"
        btn->xloc        = 10+i + (adminPanel[2] / 7.34) * i + (10 * i);   // Declaro la ubicación en X (ubicación en el ancho de la pantalla)
        btn->yloc        = screenHeight * 0.08;                     // Declaro la posición en el eje Y (altura)
        btn->xsize       = screenWidth / 7.25;                      // Declaro el tamaño del ancho del botón
        btn->ysize       = adminPanel[3] * 0.05;                    // Declaro el tamaño de la altura del botón
        btn->outLog      = lastLogs[i];                             // Esta variable almacenará los resultados de las queries según el botón en el panel de administracion, para más información revisar la función logfunction() en ui/drawing.cpp
        btn->status      = 0;                                       // Se declara el estado del botón, el cual por defecto es 0 (inactivo)
        btn->highColor   = &VOCADORADO;                              // Se declara el color del botón cuando tenga que ser resaltado
        btn->normalColor = &VOCAAMARILLOSUAVE;                       // Se declara el color del botón cuando no debe ser resaltado
        adminButtons.push_back(btn.get());                          // Se introduce el botón al vector adminButtons, pero solo el puntero del botón
        adminObj.push_back(std::move(btn));                         // Ahora, el botón se introduce a adminObj, y ahora adminObj tiene su propiedad
    }

    // Botones de las pestañas de configuracion
    std::string confignames[] = {"Credenciales", "Extra", "Paths"};
    for (int i = 0; i < 3; i++)
    {
        auto btn = std::make_unique<button>();
        btn->name        = confignames[i];
        btn->xloc        = configPanel[2] / 7.22 * (i + 2) + (7.22 * (i + 2));    // Usa configPanel para posicionarse independientemente del panel de administración
        btn->yloc        = screenHeight * 0.08;
        btn->xsize       = configPanel[2] / 7.22;
        btn->ysize       = configPanel[3] * 0.05;
        btn->outLog      = "";
        btn->status      = 0;
        btn->highColor   = &VOCADORADO;                              // Se declara el color del botón cuando tenga que ser resaltado
        btn->normalColor = &VOCAAMARILLOSUAVE;                       // Se declara el color del botón cuando no debe ser resaltado
        configbuttons.push_back(btn.get());
        adminObj.push_back(std::move(btn));
    }

    // opcionAct — botón de la lista desplegable de la pestaña "Actualizar"
    auto opcionAct = std::make_unique<button>();    // Declaro a opcionAct, el puntero original del botón
    opcionAct->name   = "...";                      // Declaro su nombre predeterminado
    opcionAct->xloc   = screenWidth * 0.12;         // Declaro su ubicación en el eje X (ancho de la pantalla)
    opcionAct->yloc   = 0;                          // Declaro su ubiación en el alto de la pantalla (es cero, ya que se declarará más tarde en la pestaña "Actualizar", para más informacion, revisar screens/screen_adminmenu.cpp)
    opcionAct->xsize  = 0;                          // Declaro su ancho (es cero, ya que se declarará después)
    opcionAct->ysize  = adminPanel[3] * 0.05;       // Declaro la altura del botón
    opcionAct->normalColor = &BLANCO;                 // Fondo blanco en reposo para distinguirlo visualmente de los demás botones
    opcionAct->highColor   = &VOCAVERDE;              // Beige pastel al tener hover o estar activo
    opcionActPtr = opcionAct.get();                 // opcionActPtr es el puntero que apuntará al puntero original del botón
    adminObj.push_back(std::move(opcionAct));       // Se introduce el botón a adminObj, y ahora adminObj tiene su propiedad

    // cedula button
    auto cedula = std::make_unique<button>();
    cedula->name      = "Ingresar";
    cedula->xloc      = (screenWidth * 0.5) - ((screenWidth * 0.16) * 0.5);
    cedula->yloc      = screenHeight * 0.7;
    cedula->xsize     = screenWidth * 0.16;
    cedula->ysize     = screenHeight * 0.1;
    cedula->selfquery = "SELECT * FROM "s + *nameTableEstudiantes + " WHERE Cedula = '"s;
    cedulaPtr = cedula.get();
    adminObj.push_back(std::move(cedula));

    // cedulaBar
    auto cedulaBar = std::make_unique<inputBar>();
    cedulaBar->name    = "Barra de Cedula";
    cedulaBar->xloc    = (screenWidth * 0.5) - ((screenWidth * 0.5) / 2);
    cedulaBar->yloc    = screenHeight * 0.5;
    cedulaBar->xsize   = screenWidth * 0.50;
    cedulaBar->ysize   = fontSize * 2;
    cedulaBar->status  = 4;
    cedulaBar->input   = "";
    cedulaBar->input32 = U"";
    cedulaBarPtr = cedulaBar.get();
    adminObj.push_back(std::move(cedulaBar));

    // votar button
    auto votar = std::make_unique<button>();
    votar->name      = "Votar";
    votar->xloc      = cedulaPtr->xloc;
    votar->yloc      = screenHeight * 0.8;
    votar->xsize     = screenWidth * 0.16;
    votar->ysize     = screenHeight * 0.1;
    votar->status    = 0;
    votar->normalColor = &VOCAAMARILLO;
    votar->highColor   = &VOCADORADO;
    votar->selfquery = "UPDATE "s + *nameTableEstudiantes + " SET "s + *nameColumnVotoNombre + " = '"s;
    votarPtr = votar.get();
    adminObj.push_back(std::move(votar));

    auto btnSi = std::make_unique<button>();
    btnSi->name   = "SI";
    btnSi->xloc   = ((warn[0]+warn[2]) / 3) + warn[0]/2 - (screenWidth * 0.01);            // Declaro la ubicación en el eje X del botón para seleccionar "SI"
    btnSi->yloc   = (warn[1]+warn[3]) * 0.9;                                               // Ubicación en eje Y
    btnSi->xsize  = screenWidth * 0.02;                                                    // Tamaño eje X
    btnSi->ysize  = screenWidth * 0.02;                                                    // Tamaño eje Y
    btnSi->status = 0;
    btnSiPtr = btnSi.get();
    adminObj.push_back(std::move(btnSi));

    auto btnNo = std::make_unique<button>();
    btnNo->name   = "NO";
    btnNo->xloc   = ((warn[0]+warn[2]) / 1.5) + warn[0]/2 - (screenWidth * 0.01);          // Declaro la ubicación en el eje X del botón para seleccionar "NO"
    btnNo->yloc   = (warn[1]+warn[3]) * 0.9;                                               // Ubicación en eje Y
    btnNo->xsize  = screenWidth * 0.02;                                                    // Tamaño eje X
    btnNo->ysize  = screenWidth * 0.02;                                                    // Tamaño eje Y
    btnNo->status = 0;
    btnNoPtr = btnNo.get();
    adminObj.push_back(std::move(btnNo));

    // continuar button
    auto continuar = std::make_unique<button>();
    continuar->name   = "Continuar";
    continuar->xloc   = cedulaPtr->xloc * 1.3;
    continuar->yloc   = screenHeight * 0.6;
    continuar->xsize  = screenWidth * 0.16;
    continuar->ysize  = screenHeight * 0.1;
    continuar->status = 0;
    continuarPtr = continuar.get();
    adminObj.push_back(std::move(continuar));

    // regresar button
    auto regresar = std::make_unique<button>();
    regresar->name   = "Regresar";
    regresar->xloc   = cedulaPtr->xloc * 0.7;
    regresar->yloc   = screenHeight * 0.6;
    regresar->xsize  = screenWidth * 0.16;
    regresar->ysize  = screenHeight * 0.1;
    regresar->status = 0;
    regresarPtr = regresar.get();
    adminObj.push_back(std::move(regresar));

    // backup button
    auto backup = std::make_unique<button>();
    backup->name      = "Backup";
    backup->xloc      = screenWidth * 0.01;
    backup->yloc      = screenHeight * 0.93;
    backup->xsize     = screenWidth * 0.03;
    backup->ysize     = screenWidth * 0.03;
    backup->status    = 0;
    backupPtr = backup.get();
    adminObj.push_back(std::move(backup));

    // resetData button
    auto resetData = std::make_unique<button>();
    resetData->name      = "resetData";
    resetData->xloc      = screenWidth * 0.08;
    resetData->yloc      = screenHeight * 0.01;
    resetData->xsize     = screenWidth * 0.03;
    resetData->ysize     = screenWidth * 0.03;
    resetData->status    = 0;
    resetDataPtr = resetData.get();
    adminObj.push_back(std::move(resetData));

    // cambiarFrontend button
    auto cambiarFrontend = std::make_unique<button>();
    cambiarFrontend->name      = "Claro";
    cambiarFrontend->xloc      = screenWidth* 0.89;
    cambiarFrontend->yloc      = screenHeight * 0.01;
    cambiarFrontend->xsize     = screenWidth * 0.03;
    cambiarFrontend->ysize     = screenWidth * 0.03;
    cambiarFrontend->status    = 0;
    cambiarFrontendPtr = cambiarFrontend.get();
    adminObj.push_back(std::move(cambiarFrontend));

    // exitAdmin button
    auto exitAdmin = std::make_unique<button>();
    exitAdmin->name   = "Salir";
    exitAdmin->xloc   = screenWidth * 0.01;
    exitAdmin->yloc   = screenHeight * 0.01;
    exitAdmin->xsize  = screenWidth * 0.03;
    exitAdmin->ysize  = screenWidth * 0.03;
    exitAdmin->status = 0;
    exitAdminPtr = exitAdmin.get();
    adminObj.push_back(std::move(exitAdmin));

    // enterConfig button
    auto enterConfig = std::make_unique<button>();
    enterConfig->name   = "Configuracion";
    enterConfig->xloc   = screenWidth * 0.96;
    enterConfig->yloc   = screenHeight * 0.01;
    enterConfig->xsize  = screenWidth * 0.03;
    enterConfig->ysize  = screenWidth * 0.03;
    enterConfig->status = 0;
    enterConfigPtr = enterConfig.get();
    adminObj.push_back(std::move(enterConfig));

    // refresh button
    auto refresh = std::make_unique<button>();
    refresh->name   = "Refrescar";
    refresh->xloc   = screenWidth  * 0.96;
    refresh->yloc   = screenHeight * 0.93;
    refresh->xsize  = screenWidth * 0.03;
    refresh->ysize  = screenWidth * 0.03;
    refresh->status = 0;
    refreshPtr = refresh.get();
    adminObj.push_back(std::move(refresh));

    // resToggle button
    auto resToggle = std::make_unique<button>();
    resToggle->name   = "#";
    resToggle->xloc   = screenWidth * 0.14;
    resToggle->yloc   = screenHeight * 0.17;
    resToggle->xsize  = littleFontSize * 1.5;
    resToggle->ysize  = littleFontSize * 1.5;
    resToggle->status = 0;
    resTogglePtr = resToggle.get();
    adminObj.push_back(std::move(resToggle));

    // informe button
    auto informe = std::make_unique<button>();
    informe->name   = "Informe";
    informe->xloc   = screenWidth * 0.8;
    informe->yloc   = screenHeight * 0.4;
    informe->xsize  = littleFontSize * (float)informe->name.length();
    informe->ysize  = littleFontSize * 1.5;
    informe->status = 0;
    informePtr = informe.get();
    adminObj.push_back(std::move(informe));

    // saveConfig button
    auto saveConfig = std::make_unique<button>();
    saveConfig->name   = "Guardar";
    saveConfig->xloc   = screenWidth * 0.5 - ((littleFontSize * saveConfig->name.length()) / 2);
    saveConfig->yloc   = screenHeight * 0.85;
    saveConfig->xsize  = littleFontSize * saveConfig->name.length();
    saveConfig->ysize  = littleFontSize * 1.5;
    saveConfig->status = 0;
    saveConfigPtr = saveConfig.get();
    adminObj.push_back(std::move(saveConfig));

    // terminalBar (barra de entrada de comandos de terminal)
    auto terminalBar = std::make_unique<inputBar>();
    terminalBar->name        = "TerminalBar";
    terminalBar->xloc        = adminPanel[0];
    terminalBar->yloc        = (adminPanel[1] + adminPanel[1] * 0.4 + adminPanel[3] - adminPanel[3] * 0.05) - 1;
    terminalBar->xsize       = adminPanel[2];
    terminalBar->ysize       = 1 + (0.5 * (adminPanel[3] - (adminPanel[3] - adminPanel[3] * 0.1)));
    terminalBar->status      = 0;
    terminalBar->highColor   = &NEGRO;
    terminalBar->normalColor = &NEGRO;
    terminalBar->input       = "";
    terminalBar->input32     = U"";
    terminalBarPtr = terminalBar.get();
    adminObj.push_back(std::move(terminalBar));

    // Barras de credenciales del terminal de administrador
    std::string nameTermCred[] = {"IP de BD:", "Puerto de BD:", "Nombre de BD:",
                                  "Usuario de BD:", "Contraseña de BD:"};
    for (int b = 0; b < 5; b++)
    {
        auto credBar = std::make_unique<inputBar>();
        credBar->name    = nameTermCred[b];
        credBar->xloc    = (screenWidth * 0.15) + ((credBar->name.length() * littleFontSize) / 2) + (screenWidth * 0.022);
        credBar->yloc    = screenHeight * 0.22 + ((screenHeight * 0.09) * b);
        credBar->xsize   = littleFontSize * 30;
        credBar->ysize   = littleFontSize * 2;
        credBar->status  = 0;
        credBar->input   = "";
        credBar->input32 = U"";
        termBars.push_back(credBar.get());
        adminObj.push_back(std::move(credBar));
    }

    // Barras de configuracion "Extra"
    std::string nameExtra[] = {
        "Nombre de la tabla 'Estudiantes':",
        "Nombre de la tabla 'Partidos':",
        "Nombre de la columna 'Nombre' de la tabla 'Partidos':",
        "Nombre de la opcion para votar nulo:",
        "Nombre de la columna 'Voto' de la tabla 'Estudiantes':",
        "Nombre de la columna 'Votos' de la tabla 'Partidos':",
        "Nombre de la columna 'Laboratorio' de la tabla 'Estudiantes':"
    };
    for (int b = 0; b < 7; b++)
    {
        auto extraBar = std::make_unique<inputBar>();
        extraBar->name    = nameExtra[b];
        extraBar->xloc    = (screenWidth * 0.15) + ((extraBar->name.length() * littleFontSize) / 2) + (screenWidth * 0.022);
        extraBar->yloc    = screenHeight * 0.22 + ((screenHeight * 0.09) * b);
        extraBar->xsize   = littleFontSize * 30;
        extraBar->ysize   = littleFontSize * 2;
        extraBar->status  = 0;
        extraBar->input   = "";
        extraBar->input32 = U"";
        extraBars.push_back(extraBar.get());
        adminObj.push_back(std::move(extraBar));
    }

    // Barras de configuracion "Paths"
    std::string namePaths[] = {"Path del font del programa:",
                               "Path del font mono del programa:",
                               "Path del font del informe PDF:",
                               "Nombre del informe de salida:"};
    for (int b = 0; b < 4; b++)
    {
        auto pathBar = std::make_unique<inputBar>();
        pathBar->name    = namePaths[b];
        pathBar->xloc    = (screenWidth * 0.15) + ((pathBar->name.length() * mediumFontSize) / 2) + (screenWidth * 0.022);
        pathBar->yloc    = screenHeight * 0.22 + ((screenHeight * 0.12) * b);
        pathBar->xsize   = mediumFontSize * 30;
        pathBar->ysize   = mediumFontSize * 2;
        pathBar->status  = 0;
        pathBar->input   = "";
        pathBar->input32 = U"";
        pathBars.push_back(pathBar.get());
        adminObj.push_back(std::move(pathBar));
    }

    // admPasswordBar es la barra en la pantalla CONFIGURATION en la que se puede configurar la contraseña para entrar al panel de administracion
    auto admPasswordBar = std::make_unique<inputBar>();
    admPasswordBar->name    = "Contraseña del panel de administracion:";
    admPasswordBar->xloc    = (screenWidth * 0.15) + ((admPasswordBar->name.length() * littleFontSize) / 2) + (screenWidth * 0.022);
    admPasswordBar->yloc    = screenHeight * 0.22 + ((screenHeight * 0.09) * (int)termBars.size());
    admPasswordBar->xsize   = termBars[0]->xsize / 2;
    admPasswordBar->ysize   = termBars[0]->ysize;
    admPasswordBar->status  = 0;
    admPasswordBar->input   = "";
    admPasswordBar->input32 = U"";
    admPasswordBarPtr = admPasswordBar.get();
    adminObj.push_back(std::move(admPasswordBar));

    // labNameBar es la barra en la pantalla CONFIGURATION en la que se configura el nombre del laboratorio actual
    auto labNameBar = std::make_unique<inputBar>();
    labNameBar->name    = "Nombre del laboratorio actual:";
    labNameBar->xloc    = (screenWidth * 0.15) + ((labNameBar->name.length() * littleFontSize) / 2) + (screenWidth * 0.022);
    labNameBar->yloc    = screenHeight * 0.22 + ((screenHeight * 0.09) *( (int)termBars.size()+1));
    labNameBar->xsize   = termBars[0]->xsize / 2;
    labNameBar->ysize   = termBars[0]->ysize;
    labNameBar->status  = 0;
    labNameBar->input   = "";
    labNameBar->input32 = U"";
    labNameBarPtr = labNameBar.get();
    adminObj.push_back(std::move(labNameBar));

    // adminTerminal (cuadro de salida de la terminal)
    auto adminTerminal = std::make_unique<inputBar>();
    adminTerminal->name        = "Cuadro de Terminal de Administrador";
    adminTerminal->xloc        = screenWidth * 0.12;
    adminTerminal->yloc        = screenHeight * 0.17;
    adminTerminal->xsize       = screenWidth * 0.76;
    adminTerminal->ysize       = screenHeight * 0.72;
    adminTerminal->status      = 0;
    adminTerminal->highColor   = &NEGRO;
    adminTerminal->normalColor = &NEGRO;
    adminTerminal->input       = "";
    adminTerminal->input32     = U"";
    adminTerminalPtr = adminTerminal.get();
    adminObj.push_back(std::move(adminTerminal));

    // barAdminTerminal (barra de entrada de comandos de la terminal de administrador)
    auto barAdminTerminal = std::make_unique<inputBar>();
    barAdminTerminal->name        = "Terminal de Administrador";
    barAdminTerminal->xloc        = adminTerminalPtr->xloc;
    barAdminTerminal->yloc        = adminTerminalPtr->yloc + adminTerminalPtr->ysize*0.99;
    barAdminTerminal->xsize       = adminTerminalPtr->xsize;
    barAdminTerminal->ysize       = littleFontSize * 2;
    barAdminTerminal->status      = 0;
    barAdminTerminal->highColor   = &NEGRO;
    barAdminTerminal->normalColor = &NEGRO;
    barAdminTerminal->input       = "";
    barAdminTerminal->input32     = U"";
    barAdminTerminalPtr = barAdminTerminal.get();
    adminObj.push_back(std::move(barAdminTerminal));

    // actBar (barra de la pestaña "Actualizar")
    auto actBar = std::make_unique<inputBar>();
    actBar->xloc    = 0;
    actBar->yloc    = opcionActPtr->yloc;
    actBar->xsize   = 0;
    actBar->ysize   = adminPanel[3] * 0.05;
    actBar->status  = 0;
    actBar->input   = "";
    actBar->input32 = U"";
    actBarPtr = actBar.get();
    adminObj.push_back(std::move(actBar));

    // opcSelected (la opción seleccionada de la pestaña "Actualizar")
    auto opcSelected = std::make_unique<sqlobject>();
    opcSelected->name        = opcionActPtr->name;
    opcSelected->normalColor = &VOCAVERDE;
    opcSelected->highColor   = &VOCADORADO;
    opcSelectedPtr = opcSelected.get();
    adminObj.push_back(std::move(opcSelected));

    if (adminSelected == "") adminSelected  = butnames[0];             // Define la pestaña seleccionada en el panel de administracion como "Consultar" de manera predeterminada
    if (configSelected == "") configSelected = confignames[0];          // Define la pestaña seleccionada en el panel de configuración como "Credenciales" de manera predeterminada

    if (statusCodeUpdating == 0)              // Si la función updateData() se ejecutó con exito (esta función se encarga de actualizar los datos y comprobar que todo esté bien), entonces se crearán los objetos relacionados a la base de datos
    {
        partidosVec.reserve(quanpartidos*2);
        tablesVec.reserve(quantables);
        columnsVec.reserve(quancolumns);
        percentages.reserve(quanpartidos);

        // Tablas
        for (int i = 0; i < quantables; i++)
        {
            auto table = std::make_unique<sqlobject>();
            table->name        = nametables[i];
            table->xloc        = (screenWidth * 0.12) + (screenWidth * 0.1 * i) + (screenWidth * 0.02 * i);
            table->yloc        = screenHeight * 0.16;
            table->xsize       = screenWidth * 0.1;
            table->ysize       = screenHeight * 0.05;
            table->status      = 0;
            table->normalColor = &VOCAVERDE;
            table->highColor   = &VOCADORADO;
            tablesVec.push_back(table.get());
            adminObj.push_back(std::move(table));
        }

        if (tableSelected == "") tableSelected = tablesVec[0]->name;

        adminButtons[0]->selfquery = "SELECT * FROM "s + tableSelected + " WHERE ";   // Se resetea la query de la pestaña "Consultar"
        adminButtons[1]->selfquery = "INSERT INTO "s  + tableSelected + " (";         // Se resetea la query de la pestaña "Agregar"
        adminButtons[2]->selfquery = "UPDATE "s       + tableSelected + " SET ";      // Se resetea la query de la pestaña "Actualizar"
        adminButtons[3]->selfquery = "DELETE FROM "s  + tableSelected + " WHERE ";    // Se resetea la query de la pestaña "Borrar"

        /* Columnas | La creación de las columnas es la más compleja, esta, necesita el tipo de dato que almacena la columna, el tamaño máximo que admite como dato, y también este objeto tiene un ID para identificarlo de otras columnas con
           el mismo nombre, todos estos datos se cargan haciendo una query a la base de datos con el uso del método DESC propio de SQL, lo que hace DESC es mostrar toda la información de las columnas de una tabla mas o menos de esta manera:

           DESC Estudiantes;              <- Esa es la query que se le realiza a la base de datos, se entra en un bucle que recorra las tablas, y se arma la query, luego la respuesta se guarda en strnamecolumns
                                             Se puede apreciar que la respuesta viene sin simbolos de "|", "+" o "-" para decorar el gráfico, esto es por que mysql detecta cuando se realiza una query desde un programa
           Cedula            varchar(20)     // Como podemos apreciar, el primer valor que aparece es el nombre de la columna, en este caso, ese valor es Cedula
           Primer_Apellido   varchar(18)     // El segundo valor es el tipo de dato, en este caso ese tipo de dato es un varchar (string), ese valor se almacena en typecolumn y en el objeto column se guarda en column->type
           Segundo_Apellido  varchar(18)     // Y el tercer valor que necesitamos es la cantidad maxima de datos que admite esta columna, el cual ese valor se almacena en maxlencolumn, y en el objeto en column->maxlen
           Nombre            varchar(30)
           Voto              tinyint(1)

          */

        bool named                   = false;         // Verifica si ya se nombró la columna actual con su nombre correspondiente
        bool typed                   = false;         // Verifica si ya se le atribuyó un tipo de datoa la columna actual
        int identifier               = 0;             // identifier es un contador que almacenará el id de cada columna dependiendo del bucle for de abajo, este id almacenado luego se guardará el la propiedad .id de cada columna
        std::string typecolumn       = "";            // Almacenará el tipo de columna
        std::string varNameColumn    = "";            // Almacenará el nombre de la columna
        std::string maxlencolumn     = "";            // Almacenará máximo de datos de entrada de la columna
        std::string descOutputString = "";            // Almacenará la output de la query con el método DESC para recibir los datos de todas las columnas de la tabla actual que se esté leyendo
        column* namecolumnPtr        = nullptr;       // Vamos a declarar un puntero nulo el cual almacenará cada columna en el bucle for de abajo de manera temporal, para ser así agregado a adminObj y a columnsVec

        for (int table = 0; table < (int)nametables.size(); table++)                      // Empieza a recorrer cada tabla, almacena el nombre de cada tabla en table
        {
            if (sendquery(("DESC " + nametables[table]).data(), 0, 4)) {return 21;}       // Realiza la query con el método DESC de SQL para recibir todos los datos de todas las columnas SQL, y si hay un error retorna 21
            if (outQuery.empty()) {return 21;}                                            // En caso de que no ocurriera un error, pero la respuesta está vacía, también retornará 21
            descOutputString = outQuery;                                                  // En caso de que todo haya salido bien, se almacenará la respuesta en descOutputString

            for (int c = 0; c < (int)descOutputString.size(); c++)                        // Este bucle recorrerá TODOS los carácteres de descOutputString
            {
                if (descOutputString[c] != '\n')                                          // Si el carácter actual NO es un newline...
                {
                    if (descOutputString[c] != ' ' && !named)                             // Si el carácter actual que se está recorriendo de descOutputString NO es un espacio y la columna NO ha sido nombrada, entonces
                        varNameColumn.push_back(descOutputString[c]);                     // Se almacenará en el string del nombre temporal el carácter
                    else if (descOutputString[c] == ' ' && !named)                        // En caso de que SÍ sea un espacio el carácter actual y la columna NO ha sido nombrada, significa que ya terminó de leer el nombre, entonces...
                    {
                        auto namecolumn = std::make_unique<column>();                     // Se declara un puntero único que sea el correspondiente a la columna actual
                        namecolumn->name      = varNameColumn;                            // Se le asigna como nombre a la columna, el nombre temporal que se acumuló con varNameColumn
                        namecolumn->fromTable = nametables[table];                        // Estamos en un bucle que recorre todas las tablas, la variable table almacena la tabla actual, así que le asignamos que la columna pertenece a table
                        namecolumn->status  = 0;                                          // Declara el estado de la columna
                        namecolumn->id      = ++identifier;                               // Aumenta a identifier y al mismo tiempo declara que el id actual es el valor de identifier
                        namecolumn->type    = "";                                         // Declara el tipo de dato actual como vacío, luego se le asignará el tipo de dato, ya que en este momento aún no se ha llegado a leer
                        namecolumn->maxlen  = "";                                         // También declara a maxlen como vacío, por que aún no se ha logrado leer
                        named               = true;                                       // Declara a named verdadero, es decir, ya se nombró la columna
                        namecolumnPtr = namecolumn.get();                                 // El puntero que previamente habíamos declarado como vacío, ahora apunta a la columna actual, esto sirve para luego almacenar los valores de type e input
                        columnsVec.push_back(namecolumn.get());                           // Se introduce la columna actual en el vector columnsVec, con su respectivo puntero
                        adminObj.push_back(std::move(namecolumn));                        // Se la columna en adminObj, ahora la columna solo es accesible por medio del puntero temporal namecolumnPtr
                    }
                    else if (descOutputString[c] != '(' && !typed)                        // Si el carácter NO es un paréntesis (el paréntesis es el inicio del máximo de datos, osea, maxlen)) y NO se ha especificado su tipo, entonces...
                        typecolumn.push_back(descOutputString[c]);                        // Procederá a acumular los carácteres en el string typecolumn
                    else if (descOutputString[c] == '(' && !typed)                        // Si el carácter SI es un paréntesis y NO se ha especificado su tipo, entonces
                    {
                        namecolumnPtr->type += typecolumn;                                // Se procederá a guardar el string que almacenó el tipo de dato a la propiedad type de la columna actual por medio de su puntero temporal
                        if (namecolumnPtr->type == "tinyint")                             // En el caso especial de que la columna actual solo maneje valores booleanos (1 y 0)...
                        {
                            namecolumnPtr->input    = "0";                                  // Se indicará de que de manera predeterminada en input, se introduzca un cero
                            namecolumnPtr->input32  = U"0";                                 // Y lo mismo en input32
                        }
                        else
                        {
                            namecolumnPtr->input   = "";                                         // input tiene que estar vacío ya que no puede almacenar ningun dato de entrada, por que no se ha digitado nada en la columna
                            namecolumnPtr->input32 = U"";                                        // Lo mismo que con input
                        }
                        typed = true;                                                     // Y ahora se declara que su tipo fue especificado
                    }
                    else if (descOutputString[c] != ')' && typed)                         // Si el carácter NO es un paréntesis final (el paréntesis final es el final de la cantidad de carácteres que se pueden introducir) entonces...
                        maxlencolumn.push_back(descOutputString[c]);                      // Se procederán a acumular los carácteres en maxlencolumn
                    else if (descOutputString[c] == ')' && typed)                         // Si ya se llega a encontrar el paréntesis final, entonces...
                    {
                        namecolumnPtr->maxlen += maxlencolumn;                            // Se procederá a guardar el string que almacenó la cantidad máxima de datos de entrada a la propiedad maxlen de la columna actual
                        named = false; typed = false;                                     // Se resetean los activadores
                        varNameColumn = ""; typecolumn = ""; maxlencolumn = "";           // Se resetean los strings que se usaron
                    }
                }
            }
            descOutputString = "";
        }

        // Texturas de las banderas, logos y representantes de los partidos
        for (int i = 0; i < quanpartidos; i++)
        {
            // ── Bandera (obligatoria) ─────────────────────────────────────────
            std::string fileFlag = "bandera"s + namepartidos[i] + ".jpg"s;
            fs::path rutaFlag = fs::current_path() / "assets" / fileFlag;
            if (!fs::exists(rutaFlag)) return 10;                                           // Si la bandera no existe, retorna error 10

            Image flag = LoadImage(((std::string)rutaFlag).data());

            auto party = std::make_unique<parties>();
            party->name   = namepartidos[i];

            party->xloc   = ((screenWidth / quanpartidos) * i) + (screenWidth / (quanpartidos * 2) / 2);
            party->yloc   = (screenHeight * 0.6) * 0.6;
            party->xsize  = screenWidth / (quanpartidos * 2);
            party->ysize  = screenHeight * 0.1;

            party->flagxsize = party->xsize;
            party->flagysize = party->ysize;
            party->flagxloc  = party->xloc;
            party->flagyloc  = party->yloc;

            // Colores derivados de la bandera
            party->mainColor = GetImageColor(flag, flag.width / 2, 1);
            party->subColor  = { (unsigned char)(party->mainColor.r / 2),
                                 (unsigned char)(party->mainColor.g / 2),
                                 (unsigned char)(party->mainColor.b / 2), 255 };
            party->fadeColor = Fade(party->mainColor, 0.25f);

            // Cargar textura de bandera
            ImageFormat(&flag, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            ImageResize(&flag, (int)party->flagxsize, (int)party->flagysize);
            party->flag = LoadTextureFromImage(flag);
            UnloadImage(flag);

            // ── Logo
            std::string fileLogo = "logo"s + namepartidos[i] + ".png"s;
            fs::path rutaLogo = fs::current_path() / "assets" / fileLogo;
            if (fs::exists(rutaLogo))
            {
                Image logo = LoadImage(((std::string)rutaLogo).data());
                party->logoW = logo.width * 0.5f;
                party->logoH = logo.height * 0.5f;
                ImageFormat(&logo, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                ImageResize(&logo, (int)party->logoW, (int)party->logoH);
                party->logo    = LoadTextureFromImage(logo);
                party->hasLogo = true;
                UnloadImage(logo);
            }
            else return 11;                                                                 // Si el logo no existe, retorna error 11

            // ── Representante (opcional, pero si no existe retorna error 11) ──
            std::string fileRep = "representante"s + namepartidos[i] + ".png"s;
            fs::path rutaRep = fs::current_path() / "assets" / fileRep;
            if (fs::exists(rutaRep))
            {
                Image rep = LoadImage(((std::string)rutaRep).data());
                party->presidentW = rep.width * 0.5f;
                party->presidentH = rep.height * 0.5f;
                ImageFormat(&rep, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                ImageResize(&rep, (int)party->presidentW, (int)party->presidentH);
                party->president    = LoadTextureFromImage(rep);
                party->hasPresident = true;
                UnloadImage(rep);
            }
            else return 11;                                                                 // Si el representante no existe, retorna error 11

            party->imageAlpha = 0;
            party->status     = 0;
            partidosVec.push_back(party.get());
            adminObj.push_back(std::move(party));
        }

        // Opcion nula (si existe)
        if (nullOption)
        {
            std::string fileFlag = "opcion"s + *nameNuloOpcion + ".jpg"s;
            fs::path rutaFlag = fs::current_path() / "assets" / fileFlag;
            std::cout << rutaFlag << "\n";
            if (!fs::exists(rutaFlag)) return 10;

            Image banner = LoadImage(((std::string)rutaFlag).data());
            auto nullPartido = std::make_unique<parties>();
            nullPartido->name   = *nameNuloOpcion;
            nullPartido->xsize  = votarPtr->xsize * 0.9;
            nullPartido->ysize  = votarPtr->ysize * 0.9;
            nullPartido->xloc   = screenWidth/2-(nullPartido->xsize/2);
            nullPartido->yloc   = votarPtr->yloc * 0.8;

            nullPartido->flagxloc  = nullPartido->xloc;
            nullPartido->flagyloc  = nullPartido->yloc;
            nullPartido->flagxsize = nullPartido->xsize;
            nullPartido->flagysize = nullPartido->ysize;

            nullPartido->mainColor = GetImageColor(banner, banner.width / 2, 1);
            nullPartido->subColor  = { (unsigned char)(nullPartido->mainColor.r / 2),
                                       (unsigned char)(nullPartido->mainColor.g / 2),
                                       (unsigned char)(nullPartido->mainColor.b / 2), 255 };
            nullPartido->fadeColor = Fade(nullPartido->mainColor, 0.25f);
            ImageFormat(&banner, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            ImageResize(&banner, (int)nullPartido->flagxsize, (int)nullPartido->flagysize);
            nullPartido->flag = LoadTextureFromImage(banner);
            UnloadImage(banner);

            // Logo de la opción nula (opcional)
            std::string fileLogoN = "logo"s + *nameNuloOpcion + ".png"s;
            fs::path rutaLogoN = fs::current_path() / "assets" / fileLogoN;
            if (fs::exists(rutaLogoN))
            {
                Image logoN = LoadImage(((std::string)rutaLogoN).data());
                nullPartido->logoW = logoN.width * 0.5f;
                nullPartido->logoH = logoN.height * 0.5f;
                ImageResize(&logoN, (int)nullPartido->logoW, (int)nullPartido->logoH);
                ImageFormat(&logoN, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                nullPartido->logo    = LoadTextureFromImage(logoN);
                nullPartido->hasLogo = true;
                UnloadImage(logoN);
            }
            else return 11;

            nullPartido->imageAlpha = 0;
            nullPartido->status = 0;
            partidosVec.push_back(nullPartido.get());
            adminObj.push_back(std::move(nullPartido));
        }

        // sinVotar (para estadisticas es el valor ABS. el cual representa el abstencionismo)
        auto sinVotar = std::make_unique<parties>();
        sinVotar->name   = "ABS.";
        sinVotar->xloc   = screenWidth  * 2;
        sinVotar->yloc   = screenHeight * 2;
        sinVotar->xsize  = 0;
        sinVotar->ysize  = 0;
        sinVotar->status = 0;
        sinVotarPtr = sinVotar.get();
        partidosVec.push_back(sinVotar.get());
        adminObj.push_back(std::move(sinVotar));
    }
    return 0;
}
