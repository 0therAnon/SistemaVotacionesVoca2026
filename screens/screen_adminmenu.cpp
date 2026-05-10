#include <chrono>
#include <thread>
#include <memory>
#include "../globals.hpp"
#include "screen_adminmenu.hpp"
#include "../config/config.hpp"
#include "../db/database.hpp"
#include "../ui/input.hpp"
#include "../ui/drawing.hpp"
#include "../ui/objects.hpp"
#include "../reports/inform.hpp"
#include "../reports/htmlreport.hpp"
#include "../platform/pty.hpp"
#include "../platform/ipvalid.hpp"

void screenAdminmenuUpdate(Screen &currentScreen,
                           bool &restartExplorar,                   // Verifica si se tiene que reiniciar la pestaña "Explorar" para actualizar sus datos
                           bool &restartResultados,                 // Verifica si se tiene que reiniciar la pestaña "Resultados" para actualizar sus datos
                           bool &restartTerminal,                   // Verifica si se tiene que reiniciar la pestaña "Terminal" para volver a pedir las credenciales
                           bool &invalidIp,                         // Sirve para saber si la IP digitada en la pestaña "Terminal" es válida
                           bool &inputEmpty,                        // Verifica si las barras de entrada en la pestaña "Terminal" están vacías
                           bool &invalidCredentials,                // Verifica si las credenciales para la pestaña "Terminal" son válidas
                           bool &successfulPdfCreation,             // Verifica si la función inform() en ./reports/inform.cpp tuvo un resultado exitoso
                           bool &fromAdmin,                         // Verifica si se va a acceder a la pantalla CONFIGURATION desde el panel de administración
                           bool &rqst,                              // Verifica si hay que realizar otra vez una query/request al servidor de base de datos en la pestaña "Explorar"
                           bool &correctstudent,                    // Sirve para verificar el estudiante actual
                           std::string &explorarFinalOutput,        // Muestra la salida de los datos de la pestaña "Explorar"
                           std::string &strresultados,              // Almacena datos la pestaña "Resultados"
                           std::string &votes,                      // Almacena los votos cargados desde la base de datos pedidos desde la pestaña "Resultados"
                           std::string &outputTerm,                 // Muestra la salida de los datos de la pestaña "Terminal"
                           std::string &modeInput,                  // Especifica el modo de entrada de datos en las barras de entrada (los modos se encuentran en inputfunc())
                           std::string &outResultsMode,             // Especifica el modo de salida de los datos de la gráfica (si se desean porcentajes o cantidades)
                           int &opcSize)
{
    int conCnt = 0, agrCnt = 0, actCnt = 0, borCnt = 0, cnt = 0;    // Declara todos estos contadores a 0
    bool added = false;                                             // Declara a added, verifica si se armó la query de la pestaña "Actualizar" con los valores de la barra de entrad
    std::string agregarVal = "", agregarCol = "";                   // Estos strings se usarán para la pestaña "Agregar", servirán para guardar los valores y posteriormente usarse para armar la query a la base de datos

    // Detectar la interacción con algun botón o pestaña

    if (IsGestureDetected(GESTURE_TAP))                             // Si se llega a presionar la pantalla, se ejecutará el siguiente bloque de código
    {

        exitAdminPtr->status        = isPressed(exitAdminPtr);           // Se empieza a verificar el estado
        enterConfigPtr->status      = isPressed(enterConfigPtr);         // de los botones de salir, actualizar datos
        refreshPtr->status          = isPressed(refreshPtr);             // o ingresar a la pantalla CONFIGURATION
        cambiarFrontendPtr->status  = isPressed(cambiarFrontendPtr);     // o cambiar el modo del frontend
        backupPtr->status           = isPressed(backupPtr);              // o ejecutar un backup
        resetDataPtr->status        = isPressed(resetDataPtr);           // o reiniciar la base de datos

        if (exitAdminPtr->status == 4 ||
            enterConfigPtr->status == 4 ||
            refreshPtr->status == 4 ||
            cambiarFrontendPtr->status == 4 ||
            backupPtr->status == 4 ||
            resetDataPtr->status == 4
            )          // Si alguno de esos cuatro botones anteriores llega a recibir el estado 4 (fue presionado) se ejecutará el siguiente bloque de código
        {
            restartTerminal = true;                                                                       // Se activa el reinicio de las credenciales de la pestaña "Terminal"
            columnSelected  = columnsVec[0]->id;                                                          // La columna seleccionada ahora será la primera en el vector columnsVec

            for (int z = 0; z < (int)adminButtons.size(); z++) {adminButtons[z]->outLog = "";}            // Este bucle recorre el atributo outLog de cada pestaña, para que en caso de volver a ingresar al panel, se encuentre limpio

                if (exitAdminPtr->status == 4)                          // Si el botón presionado fue el de salir del panel de administración...
                {
                    currentScreen = MAINMENU;                           // La pantalla actual ahora será MAINMENU
                    cedulaBarPtr->status = 4;                           // El estado de la barra de entrada cedulaBar, pasará a estado 3, para automáticamente recibir datos de entrada
                    return;                                             // Retorno de la función
                }
                else if (refreshPtr->status == 4)                       // Si el botón presionado fue el de refrescar los datos...
                {
                    updateData();                                       // En caso de que el botón presionado fue el de actualizar los datos, procederá a ejecutar updateData()
                    objectCreation();                                   // Como se actualizaron los datos, existe la posibilidad de que botones, tablas o columnas ya no existan, entonces se vuelven a crear los objetos
                    return;                                             // Retorno de la función
                }
                else if (cambiarFrontendPtr->status == 4)               // Si el botón para cambiar el modo del frontend fue presionado...
                {
                    darkMode = darkMode ? false : true;                 // Cambia el valor de darkMode, si era verdadero lo cambia a falos, y viceversa
                    COLORTEXTO = darkMode ? BLANCO : NEGRO;              // Cambia el color del texto de los textos que deben de ser cambiados para su correcta visualización
                    return;                                             // Retorno de la función
                }
                else if (backupPtr->status == 4)                        // Si el botón para hacer backups fue presionado...
                {
                    backupRetString = ptyfunc("backup", *user, *password, *server, *port, *database);     // Ejecuta un backup de la base de datos y guarda la respuesta acá
                    backupReleased = true;                              // Informa que se realizó un backup
                    return;                                             // Retorno de la función
                }
                else if (resetDataPtr->status == 4)                     // Si el botón para reiniciar la base de datos fue presionado...
                {
                    int errors = 0;                                     // Declara un entero llamado errors, el cual almacenará la suma de los códigos de estado de cada query, si ocurre un error la función sendquery devolverá un número mayor a 0
                    errors += (int)sendquery(("UPDATE "s + *nameTableEstudiantes + " SET "s + *nameColumnVotoNombre + " = '0'; "s).data(),0,0,0);   // Suma el código de estado de sendquery() y manda la query | EXITOSO = 0 | ERROR = MAYOR A 0
                    errors += (int)sendquery(("UPDATE "s + *nameTableEstudiantes + " SET " + *nameColumnLabsNombre + " = '0000';"s).data(),0,0,0);  // Reinicia los labs de cada estudiante, la línea anterior reiniciaba los votos de los estudiantes
                    errors += (int)sendquery(("UPDATE "s + *nameTablePartidos + " SET " + *nameColumnVotosNombre + " = '0';"s).data(),0,0,0);       // Reinicia los votos de los partidos
                    if ((int)errors == 0)                               // Si el valor de errors es igual a cero (es decir, ningun sendquery() devolvió un código mayor a cero) entonces procederá con la actualización de los datos
                    {
                        updateData();                                   // Actualiza la información cargada de la base de datos
                        objectCreation();                               // Vuelve a crear los objetos
                        successReset = true;                            // Indica que el reinicio de los datos fue verdadero/exitoso
                        return;                                         // Retorno de la función
                    }
                    else                                                // En caso de que SÍ haya ocurrido un error...
                    {
                        errorReset = true;                              // Se indica que hubo un error al reiniciar los datos
                        return;                                         // Retorno de la función
                    }
                }
                else                                                    // Si el botón presionado no fue ni el de salir ni el de actualizar, entonces significa que fue el de entrar a la configuración, así que procederá a ejecutar lo siguiente
                {
                    fromAdmin = true;                                   // fromAdmin pasará a true, dando a entender al programa de que se quiso entrar a la pantalla de configuración desde el panel de administración
                    loadConfig();                                       // loadConfig() procederá a cargar los datos que se pudieron cargar del archivo de configuración a las barras de entrada, para no tener que escribir toda la configuración
                    currentScreen = CONFIGURATION;                      // La pantalla actual ahora será CONFIGURATION
                    configSelected = configbuttons[0]->name;            // La pestaña seleccionada será la primera en el vector configButtons ("Credenciales") para aparecer en esta pestaña automáticamente
                    return;                                             // Retorno de la función
                }
        }

        for (int i = 0; i < (int)adminButtons.size(); i++)          // Si el programa llega hasta esta línea, es por que no se tocó ningun botón, entonces procederá a recorrer las pestañas
        {
            adminButtons[i]->status = isPressed(adminButtons[i]);   // Verifica el estado de cada pestaña

            if (adminButtons[i]->status == 4)                       // Si la pestaña actual que está recorriendo el bucle tiene estado 3 (fue presionada), procederá a ejecutar el siguiente bloque de código...
            {
                add = 0;                                            // Declara a add igual a 0, es una variable que sirve para sobre el cuadro de las respuestas que reciben cada pestaña, más información en ./ui/drawing.cpp -> logfunction()
                adminSelected = adminButtons[i]->name;              // Declara a adminSelected igual al nombre de la pestaña seleccionada
                if      (adminSelected == butnames[4]) { restartExplorar  = true; rqst = true; }          // Si la pestaña presionada fue "Explorar" (butnames[4]) restartExplorar y rqst los declara true, para que actualice los datos
                else if (adminSelected == butnames[5]) { restartResultados = true; }                      // Si la pestaña presionada fue "Resultados" (butnames[5]) restartExplorar lo declara true, para que actualice los datos
                else if (adminSelected == butnames[6]) { tabRestart = true; }                             // Si la pestaña presionada fue "Terminal" (butnames[6]) tabRestart se pasa a true, no pasa como con las pestañas anteriores
                else { columnsVec[0]->status = 4; }                                                       // En caso de que ninguno de esos botones fue el presionado, procede a declarar a la primera columna del vector columnsVec como activa
            }

        }
    }

    // ── Pestañas Consultar / Agregar / Actualizar / Borrar ────────────────────

    if (adminSelected != butnames[4] && adminSelected != butnames[5] && adminSelected != butnames[6])     // Si la pestaña presionada no es "Explorar", "Resultados" ni "Terminal", entonces ejecutará este bloque hasta la línea 297 aprox
    {
        for (int co = 0; co < (int)columnsVec.size(); co++)         // A este bucle llamemoslo CICLO FOR PADRE, se encarga de recorrer absolutamente todas las columnas de todas las tablas
        {
            // Hay columnas en la base de datos que solo admiten valores booleanos/tinyint, esos valores son iguales a 1 o iguales a 0, en caso de que se presione la tecla SPACE, esos valores cambiarán, SPACE sirve como un atajo
            if (IsKeyPressed(KEY_SPACE) && columnsVec[co]->id == columnSelected &&          // Si se presiona la tecla space, y la columna actual en el bucle es igual a la columna seleccionada y...
                columnsVec[co]->type == "tinyint" && actBarPtr->status == 0)                // el tipo de datos de la columna actual es booleano y el estado de la barra de entrada de la barra de actualizaciones es igual a 0 (para prevenir conflictos)
              // Se verifica si la barra de actualizaciones (actBar) tiene un estado 0, por que si el administrador se encuentra en la pestaña de actualizaciones y toca la tecla SPACE pueden ocurrir errores inesperados
            {
                if (columnsVec[co]->input == "0") {columnsVec[co]->input = "1";}        // Llenará la input con un 1 si la input tenía un 0
                else                                   {columnsVec[co]->input = "0";}         // Si la input tenía un cero, la llenará con un 1
            }

            // Al presionar la tecla TAB, se puede pasar de una columna a otra columna, es muy útil

            if (IsKeyPressed(KEY_TAB))                    // Si se presiona la tecla TAB...
            {
                if (adminSelected != butnames[6])         // Si la pestaña actual NO es la pestaña "Terminal"...
                {
                    if (tabRestart)                                                         // tabRestart es true cuando se hace click a una columna, y por ende debe actualizar a tabCnt la nueva columna en uso, sino seguiría con un valor viejo
                    {
                        tabCnt = 0;                                                                                     // Se reiniciará el contador de los tabs
                        for (int clm = 0; clm < (int)columnsVec.size(); clm++)                                          // Este bucle, recorrerá todas las columnas disponibles para averiguar la nueva columna activa
                        {
                            if (columnsVec[clm]->fromTable == tableSelected) {tabCnt++;}                                // Si la columna actual es de la tabla seleccionada, aumentará el contador por uno
                            if (columnsVec[clm]->id == columnSelected) { tabCnt--; tabRestart = false; break; }         // Si la columna actual en el bucle es la columna seleccionada, resta uno a tabCnt ya que se sumará en el siguiente if
                        }
                    }
                    if (columnsVec[co]->id == columnSelected && tabCnt < quancolumns - 1)   // Se verifica que tabCnt no sea mayor que la cantidad de columnas totales, se le resta un valor a quancolumns por que en las líneas siguientes el valor de
                    {                                                                       // tabCnt se suma, y no debe sobrepasar a quancolumns
                        tabCnt++;                                         // Se le suma un valor a tabCnt, indicando la nueva posición de la columna actual
                        columnsVec[co]->status = 0;                       // Pone en estado 0 la columna actual del bucle for de la línea 93, la cual debe ser la que se estaba usando para escribir
                        columnsVec[co + 1]->status = 2;                   // Cambia a 2 la columna siguiente, ahora sí permite escribir, esto logra el cambio entre las columnas
                        columnSelected = columnsVec[co + 1]->id;          // Ahora nombra como columna seleccionada al id de la columna siguiente
                        beam = 0; break;                                  // beam es la variable para que el cursor parpadee, lo resetea a 0, y rompe el bucle for de la línea 93
                    }
                    else if (columnsVec[co]->id == columnSelected && tabCnt == quancolumns - 1)               // Si tabCnt es mayor o igual a quancolumns, significa que ya está en la última columan que aparece en pantalla, entonces...
                    {
                        columnsVec[co]->status = 0;                                                           // Pasa el estado de la columna actual en uso, proveniente del bucle for de la línea 93, a 0 (no permite escribir)
                        columnsVec[co + 1 - quancolumns]->status = 2;                                         // Pasa a estado 1 la primera columna en las columnas de la tabla actual
                        columnSelected = columnsVec[co + 1 - quancolumns]->id;                                // Ahora, la columna seleccionada es la primera de la tabla actual
                        beam = 0; tabCnt = 0; break;                                                          // Resetea a beam y rompe el bucle for padre
                    }
                }
            }

            // ── Consultar ────────────────────────────────────────────────────
            if (adminSelected == butnames[0])                                 // Si adminSelected almacena el nombre del primer índice de butnames, el cual sería "Consultar", entonces...
            {
                if (columnsVec[co]->fromTable == tableSelected)               // Si la columna que se está recorriendo desde el bucle for padre es de la tabla actual, entonces...
                {
                    if (execConfirmation == 100)                              // Llega a este if si aún se está confirmando realizar la query
                        alert(adminSelected, "backend");
                    else if (execConfirmation == 1)                           // Llega a este if si SÍ SE CONFIRMA la query
                    {
                        if (adminButtons[0]->outLog != "") {adminButtons[0]->outLog += "\n";}     // Si el valor de outLog NO está vacío, se le agregará un newline, esto por pura estética con la función logfunction() de ui/drawing.cpp
                        adminButtons[0]->outLog += adminButtons[0]->selfquery + ";\n"s;           // Ahora, al outLog, se le agrega la query que se armó para enviar a la base de datos
                        sendquery(adminButtons[0]->selfquery.data(), 0, 0, 1, " | ");             // Se procede a enviar la query, y se pide como separador entre las columnas al símbolo pipe |
                        adminButtons[0]->outLog    += outQuery;                                   // Se le agrega al outLog del botón actual la respuesta de la query
                        adminButtons[0]->selfquery  = "SELECT * FROM "s + tableSelected + " WHERE "s;       // Se reinicia la query de la pestaña "Consultar", esto para prepararse en caso de otra query
                        conCnt = 0;                                                                         // Reinicia el contador a cero
                        execConfirmation = 0;                                                           // Se reinicia el valor de execConfirmation para volver a ejecutar otra confirmación en caso de ser necesario
                    }
                    else if (execConfirmation == -1)                          // Llega a este if si se desea cancelar la query
                    {
                        adminButtons[0]->selfquery  = "SELECT * FROM "s + tableSelected + " WHERE "s;       // Se reinicia la query de la pestaña "Consultar", esto para prepararse en caso de otra query
                        conCnt = 0;                                                                         // Reinicia el contador a cero
                        execConfirmation = 0;
                    }
                    if (IsKeyPressed(KEY_ENTER))                              // Si se presiona la tecla ENTER, procederá a realizar la query
                    {
                        conCnt++;                                             // Aumenta el contador de "Consultar"
                        adminButtons[0]->selfquery += columnsVec[co]->name + " REGEXP '"s + columnsVec[co]->input + "'"s;   // Empieza a armar la query, introduciendo el nombre de la columna actual, la palabra "REGEXP" y su valor a buscar
                        if (conCnt < quancolumns)                             // Si el contador es menor a la cantidad de columnas actuales (quancolumns es la variable global que almacena las columnas de la tabla actual) entonces agregará un AND
                        {                                                     // Esto sirve para saber si la columna actual es la última o no
                            adminButtons[0]->selfquery += " AND ";            // Se le agrega un AND para en el siguiente recorrido del bucle for padre, se almacene la siguiente columna
                        }
                        else                                                  // En caso de el contador SÍ iguale a la cantidad de columnas actuales...
                        {
                            alert(adminSelected, "backend");                  // Llamará a la función alert(), para verificar si es necesario una advertencia
                        }
                    }
                }
            }
            // ── Agregar ──────────────────────────────────────────────────────
            else if (adminSelected == butnames[1])                          // Si el botón seleccionado es igual a "Agregar"...
            {
                if (columnsVec[co]->fromTable == tableSelected)             // Si la columna que se está recorriendo desde el bucle for padre es de la tabla actual, entonces...
                {
                    if (execConfirmation == 100)                              // Llega a este if si aún se está confirmando realizar la query
                        alert(adminSelected, "backend");
                    else if (execConfirmation == 1)                           // Llega a este if si SÍ SE CONFIRMA la query
                    {
                        if (adminButtons[1]->outLog != "") adminButtons[1]->outLog += "\n";       // Si outLog NO está vacío, se le agrega un newline, esto para razones de estética
                        adminButtons[1]->outLog    += adminButtons[1]->selfquery + ";\n"s;        // Se le agrega la query ya armada al outLog de la pestaña "Agregar"
                        sendquery(adminButtons[1]->selfquery.data(), 0, 0, 2);                    // Se envía la query
                        adminButtons[1]->outLog    += outQuery;                                   // Se introduce el resultado de la query a outLog, para visualizar la información de respuesta de la base de datos
                        adminButtons[1]->selfquery  = "INSERT INTO "s + tableSelected + " (";     // Se reinicia la query de la pestaña "Agregar", esto para prepararse para otra futura query
                        agrCnt = 0;       // Reinicia el contador a cero
                        updateData();                                                             // Se actualiza la información, ya que fue modificada
                        objectCreation();                                                         // Se vuelven a crear los objetos, pero solo los relacionados a la base de datos
                        execConfirmation = 0;                                                           // Se reinicia el valor de execConfirmation para volver a ejecutar otra confirmación en caso de ser necesario
                    }
                    else if (execConfirmation == -1)                          // Llega a este if si se desea cancelar la query
                    {
                        adminButtons[1]->selfquery  = "INSERT INTO "s + tableSelected + " (";     // Se reinicia la query de la pestaña "Agregar", esto para prepararse para otra futura query
                        agrCnt = 0;       // Reinicia el contador a cero
                        execConfirmation = 0;                                                           // Se reinicia el valor de execConfirmation para volver a ejecutar otra confirmación en caso de ser necesario
                    }
                    if (IsKeyPressed(KEY_ENTER))                            // Si se presiona la tecla ENTER, procederá a realizar la query
                    {
                        if (agrCnt < quancolumns)                           // Si el contador de la pestaña "Agregar" es menor a la cantidad de columnas de la tabla actual, es decir, no es la última columna de la tabla actual
                        {
                            agrCnt++;                                       // El contador aumentará un valor
                            agregarCol += columnsVec[co]->name;             // Al string agregarCol se le sumará el nombre de la columna actual
                            agregarVal += columnsVec[co]->input;            // Al string agregarVal se le sumará el valor que se haya introducido en su barra
                            if (agrCnt != quancolumns) { agregarCol += ", "; agregarVal += "', '"; }    // Si aún la columna actual NO es la última, entonces se le agregarán comas, para recibir un siguiente valor
                        }
                        if (agrCnt == quancolumns)                          // Si el contador es igual a la cantidad de columnas, es decir, ya se llegó a la última columna, entonces...
                        {
                            agregarCol += ") VALUES ('";                    // Al string agregarCol se le agregará ") VALUES (", esto para unir este string con agregarVal en la query
                            agregarVal += "')";                             // agregarVal finaliza su contenido
                            adminButtons[1]->selfquery += agregarCol + agregarVal;                    // Ahora, se unirán los strings para armar la query
                            alert(adminSelected, "backend");
                        }
                    }
                }
            }
            // ── Actualizar ───────────────────────────────────────────────────
            else if (adminSelected == butnames[2])                                                                          // Si la pestaña actual, es la de "Actualizar", se ejecutará este bloque de código hasta la línea 257 aprox
            {
                // Este if a continuación, sirve para que no se repita de nuevo el armado de estos datos a la query de "Actualizar", ya que hay que recordar que estamos en un bucle desde la línea 29 aprox, y no se deben repetir datos en la query

              if (IsKeyPressed(KEY_ENTER) && !added && opcSelectedPtr->name != opcionActPtr->name)                        // Si se presiona la tecla ENTER y NO se ha añadido este pedazo para la query a la base de datos (para eso !added)
                {
                    if (!((opcSelectedPtr->type == "tinyint" && actBarPtr->input.empty()) ||                                // Si el campo NO es uno de tipo tinyint vacío o... (apreciar el símbolo ! en todo el OR ||, el cual niega todo)
                          (opcSelectedPtr->type == "int"     && actBarPtr->input.empty())))                                 // Si el campo NO es uno de tipo int vacío
                    {
                        adminButtons[2]->selfquery += opcSelectedPtr->name + " = '"s + actBarPtr->input + "' WHERE "s;      // Se le añade a la query de la pestaña "Actualizar" el nombre de la columna a actualizar y lo que se ingresó a actBar
                        added = true;                                                                                       // added pasa a ser true
                    }
                }

                // Ahora, los objetos opcionAct y actBar deben ser actualizados, ya que no se les nombró en objectCreation() ciertos valores, estos valores se nombran dependiendo de los tamaños de las columnas de la tabla actual y sus nombres

                opcionActPtr->xsize = maxLenName;                                                       // opcionAct es el botón de la lista desplegable, su tamaño se calcula dependiendo del nombre más largo de las columnas de la tabla actual
                opcionActPtr->yloc  = screenHeight * 0.23 + (littleFontSize * 2) * quancolumns +        // La ubicación de la altura del botón se calcula dependiendo de la ubicación de la última columna
                                      screenHeight * 0.02 * quancolumns;
                actBarPtr->yloc     = opcionActPtr->yloc;                                               // actBar es la barra de entrada de la columna que se quiera actualizar, su ubicación de la altura será la misma a opcionAct
                actBarPtr->xloc     = opcionActPtr->xloc + opcionActPtr->xsize + screenWidth * 0.02;    // Su ubicación en el eje x depende de la ubicación en ancho del botón opcionAct, mas el ancho del botón, mas un espacio extra enre el botón

                if (columnsVec[co]->fromTable == tableSelected)                                         // Ahora, si recordamos que estamos en un bucle desde la línea 29, podemos usarlo para buscar cuales son las columnas de la tabla actual
                {
                    opcionActPtr->status = isPressed(opcionActPtr);                                     // Se verifica el estado de opcionAct
                    if (opcionActPtr->status > 1 && (int)opcionesAct.size() < quancolumns)              // En caso de que se llegue a presionar opcionAct, para abrir la lista desplegable, y el vector opcionesAct es menor a la cantidad de columnas...
                    {
                        if (opcionActPtr->status == 4) {                                                // Si se presiona el botón para escoger una opción...
                            actBarPtr->input = "";                                                      // Se procederá a vaciar la barra actBar
                            actBarPtr->input32 = U"";                                                   // para que se limpie automáticamente al cambiar opciones
                        }
                        cnt++;                                                                          // Aumenta el contador, se usa para que opcionesAct no se llene infinitamente
                        auto opc = std::make_unique<sqlobject>();                                       // Se declara un objeto temporal llamado opc, el cual servorá para guardar entre las opciones a la columna actual del ciclo for padre
                        opc->name   = columnsVec[co]->name;                                             // El nombre será el de la columna actual del ciclo for padre
                        opc->xloc   = opcionActPtr->xloc;                                               // Su ubicación en el ancho de la pantalla será la de opcionAct
                        opc->yloc   = opcionActPtr->yloc + opcionActPtr->ysize * cnt;                   // Su ubicación en la altura de la pantalla será la de opcionAct mas su tamaño en altura, multiplicado por cnt para que no se sobrepongan
                        opc->xsize  = opcionActPtr->xsize;                                              // Su ancho será el mismo que opcionAct
                        opc->ysize  = opcionActPtr->ysize;                                              // Su altura será la misma que opcionAct
                        opc->status = 0;                                                                // Su estado será 0, ya que no debe de haber recibido alguna interacción aún
                        opc->type   = columnsVec[co]->type;                                             // Su tipo de dato será el mismo que el de la columna correspondiente
                        opc->maxlen = std::stoi(columnsVec[co]->maxlen);                                // Su tamaño máximo de datos de entrada será el mismo que el de la columna correspondiente
                        opc->id     = columnsVec[co]->id;                                               // Su id será el mismo que el de la columna correspondiente
                        opc->normalColor = &BLANCO;                                                     // Fondo blanco en reposo para que sea consistente con el botón principal del desplegable
                        opcionesAct.push_back(opc.get());                                               // El objeto se empuja a opcionesAct
                        adminObj.push_back(std::move(opc));                                             // Su puntero se almacena en adminObj
                    }

                    // Ahora verifica si se presionó alguna opción de la lista desplegable, con opcionesActPtr->status == 0 significa que ya el botón que abrió la lista desplegable, sino que ahora se presionó alguna de sus opciones

                    else if (opcionActPtr->status == 0 && (int)opcionesAct.size() == quancolumns)       // Si las opciones aún no han sido vaciadas, osea de que opcionesAct aún sigue teniendo el tamaño de quancolumns
                    {
                        for (auto& opt : opcionesAct)                                                   // Por cada opción en el vector opcionesAct...
                        {
                            opt->status = isPressed(opt);                                               // Busca la opción que haya sido presionada
                            if (opt->status == 4)                                                       // Si la opción en el bucle actual fue presionada...
                            {
                                opcSize      = opcionesAct.size();                                      // opcSize almacenará la cantidad total de opciones en opcionesAct
                                opcSelectedPtr = opt;                                                   // La opción seleccionada será la que está usando el bucle ahora mismo
                                while (!opcionesAct.empty()) opcionesAct.pop_back();                    // Empezará a vaciar todo el vector con las opciones, pero esto no afecta a opcSelected por que opcSelected guardó a opt como una copia
                            }
                            actBarPtr->xsize = opcSelectedPtr->maxlen * littleFontSize;                 // El tamaño de actBar se calcula por medio del ancho de las letras de littleFontSize multiplicado por el tamaño máximo de datos de entrada
                        }
                    }
                    if (opcionActPtr->status == 0 && !opcionesAct.empty())                              // Si en realidad ninguna opción fue presionada, llegará a este if...
                        while (!opcionesAct.empty()) opcionesAct.pop_back();                            // Lo cual solo vaciará a opcionesAct

                    if (opcSelectedPtr->name != opcionActPtr->name)                               // Si la opción seleccionada NO es la opción vacía
                    {
                        actBarPtr->status = isPressed(actBarPtr);                                 // Verifica el estado de la barra de entrada de la opción seleccionada
                        if (opcSelectedPtr->type == "tinyint") {                                  // Si el tipo de la columna es booleano/tinyint...
                          modeInput = "boolean";                                                  // modeInput será "boolean"
                          actBarPtr->ysize = littleFontSize * 2;                                  // ysize será un tamaño más pequeño, y
                          actBarPtr->xsize = littleFontSize * 2;                                  // xsize también, para que den la ilusión de que es una casila
                        }
                        else if (opcSelectedPtr->type == "int")     modeInput = "regexponly";     // Si el tipo de dato de la columna es "int", modeInput será "regexponly"
                        else if (opcSelectedPtr->type == "varchar") modeInput = "allchars";       // Si el tipo de dato es varchar, modeInput será "allchars"
                        if (actBarPtr->status > 1)                                                // Si el estado de la barra de datos de entrada NO es 0 (SÍ fue presionada)
                            actBarPtr->input = inputfunc("backend", actBarPtr,                    // Se ejecutará la función inputfunc() para recibir datos de entrada, a la barra actBar
                                                         opcSelectedPtr->maxlen, modeInput,       // El tamaño máximo de los datos de entrada serán los especificados por el atributo maxlen, y el modo de entrada será el especificado antes
                                                         littleFontSize, BLANCO,                   // El tamaño del font será littleFontSize, y la letra será blanca, aunque no aplica por ser desde el backend
                                                         8*(opcSize-1), 120*(opcSize-1)*2, 8*(opcSize-1));    // Se usa opcSize para lidiar con bugs de muchas repeticiones por segundo de esta función al estar dentro del bucle for padre...
                                                         //littleFontSize, BLANCO);                // si no me explico bien, descomente esta línea y comente las dos anteriores, vaya a la pestaña "Actualizar" y escriba/borre/muevase en actBar
                            if (actBarPtr->status == 4 && modeInput == "boolean") {break;}        // Si el estado es 4 y es booleano debe de romper el bucle, ya que si no se rompe, inputfunc() recibirá esto muchas veces, y cambiará mucho su valor

                        if (execConfirmation == 100)                              // Llega a este if si aún se está confirmando realizar la query
                            alert(adminSelected, "backend");
                        else if (execConfirmation == 1)                           // Llega a este if si SÍ SE CONFIRMA la query
                        {
                            if (adminButtons[2]->outLog != "") adminButtons[2]->outLog += "\n";             // Si outLog no está vacío, agregará un newline por pura estética
                            adminButtons[2]->outLog += adminButtons[2]->selfquery + ";\n"s;                 // Se le agrega a outLog la query ya armada
                            sendquery(adminButtons[2]->selfquery.data(), 0, 0, 2);                          // Se envía la query
                            adminButtons[2]->outLog    += outQuery;                                         // El resultado de la query se introduce en outLog para mostrarlo en el frontend
                            adminButtons[2]->selfquery  = "UPDATE "s + tableSelected + " SET "s;            // Se reinicia la query de la pestaña "Actualizar" para prepararse en caso de una query nueva
                            actCnt = 0;                                                                     // Se reinicia el contador
                            updateData();                                                                   // Se actualiza la información, ya que fue modificada
                            objectCreation();                                                               // Se vuelven a crear los objetos, pero solo los relacionados a la base de datos
                            execConfirmation = 0;                                                           // Se reinicia el valor de execConfirmation para volver a ejecutar otra confirmación en caso de ser necesario
                        }
                        else if (execConfirmation == -1)
                        {
                            adminButtons[2]->selfquery  = "UPDATE "s + tableSelected + " SET "s;            // Se reinicia la query de la pestaña "Actualizar" para prepararse en caso de una query nueva
                            actCnt = 0;                                                                     // Se reinicia el contador
                            execConfirmation = 0;
                        }
                        if (IsKeyPressed(KEY_ENTER) && !actBarPtr->input.empty())                 // Si se presiona ENTER y actBar no se encuentra vacío...
                        {
                            actCnt++;               // actCnt empieza a sumarse
                            adminButtons[2]->selfquery += columnsVec[co]->name + " REGEXP '"s + columnsVec[co]->input + "'"s;     // Empieza a armar la query
                            if (actCnt < quancolumns)
                            {
                                adminButtons[2]->selfquery += " AND ";                    // Si la columna actual NO es la última en el bucle for padre, entonces agregará un AND
                            }
                            else                                                                                // Si SÍ es la última, no agregará un AND, y entonces...
                            {
                                alert(adminSelected, "backend");                  // Llamará a la función alert(), para verificar si es necesario una advertencia
                            }
                        }
                    }
                    else { actBarPtr->input = ""; }       // En caso de que NO se haya seleccionado ninguna opción, la barra de actualizar tendrá un valor vacío
                }
            }
            // ── Borrar ───────────────────────────────────────────────────────
            else if (adminSelected == butnames[3])                // Si la pestaña seleccionada es la pestaña "Borrar", entonces...
            {
                if (columnsVec[co]->fromTable == tableSelected)   // Verifica que la columna del bucle padre perteneza a la tabla seleccionada
                {
                    if (execConfirmation == 100)                              // Llega a este if si aún se está confirmando realizar la query
                        alert(adminSelected, "backend");
                    else if (execConfirmation == 1)                           // Llega a este if si SÍ SE CONFIRMA la query
                    {
                        if (adminButtons[3]->outLog != "") adminButtons[3]->outLog += "\n";               // Agrega un newline al outLog de "Borrar", por pura estética
                        adminButtons[3]->outLog    += adminButtons[3]->selfquery + ";\n"s;                // Agrega la query ya armada
                        sendquery(adminButtons[3]->selfquery.data(), 0, 0, 2);                            // Se envía la query
                        adminButtons[3]->outLog    += outQuery;                                           // Se almacena el resultado de la query dentro de outLog, para ver la respuesta en el frontend
                        adminButtons[3]->selfquery  = "DELETE FROM "s + tableSelected + " WHERE "s;       // Se reinicia la query de la pestaña "Borrar", esto para prepararse en caso de una futura query
                        borCnt = 0;                                                                       // Se reinicia el contador
                        updateData();                                                                     // Se actualiza la información, ya que fue modificada
                        objectCreation();                                                                 // Se vuelven a crear los objetos, pero solo los relacionados a la base de datos
                        execConfirmation = 0;                                                             // Se reinicia el valor de execConfirmation para volver a ejecutar otra confirmación en caso de ser necesario
                    }
                    else if (execConfirmation == -1)                          // Llega a este if si se desea cancelar la query
                    {
                        adminButtons[3]->selfquery  = "DELETE FROM "s + tableSelected + " WHERE "s;       // Se reinicia la query de la pestaña "Borrar", esto para prepararse en caso de una futura query
                        borCnt = 0;                                                                       // Se reinicia el contador
                        execConfirmation = 0;
                    }
                    if (IsKeyPressed(KEY_ENTER))                                                          // Si se presiona la tecla ENTER...
                    {
                        borCnt++;                                                                         // Aumenta un valor al contador
                        adminButtons[3]->selfquery += columnsVec[co]->name + " REGEXP '"s + columnsVec[co]->input + "'"s;       // Se empieza a armar la query
                        if (borCnt < quancolumns)
                        {
                            adminButtons[3]->selfquery += " AND ";                                        // Si la columna actual en el bucle for padre NO es la última, agregará un AND a la query
                        }
                        else                                                                              // En caso de que sí sea la última...
                        {
                            alert(adminSelected, "backend");                  // Llamará a la función alert(), para verificar si es necesario una advertencia
                        }
                    }
                }
            }
        }   // <---- FÍN DEL BUCLE FOR PADRE QUE RECORRE TODAS LAS COLUMNAS
    }   // FÍN DEL IF QUE VERIFICABA QUE LA PESTAÑA ACTUAL NO FUERA NI EXPLORAR, NI RESULTADOS, NI TERMINAL

    // ── Explorar ──────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[4])                              // Si la pestaña actual seleccionada es "Explorar"...
    {
        oldSelected = adminSelected;                                    // Se actualizará manualmente a oldSelected, ya que antes se actualizaba con la función drawcolumns, pero esa función solo se llama en las primeras 4 pestañas
        std::string line;                                               // Se declara line como un string
        std::vector<std::string> exploreList;                           // Se declara exploreList, vector cuyo propósito es almacenar los strings de outLog de la pestaña "Explorar", para luego traducirlos a explorarFinalOutput

        for (int t = 0; t < (int)tablesVec.size(); t++)                 // Se recorrerá cada tabla a continuación
        {
            tablesVec[t]->status = isPressed(tablesVec[t]);                       // Se verifica el estado de cada tabla
            if (IsKeyPressed(KEY_TAB))                                            // Si se presiona el tabulador debe de provocar que cambie entre las tablas, así que justo eso hará
            {
                if (tablesVec[t]->status > 1 && t + 1 < (int)tablesVec.size())    // Si el estado de la tabla actual es superior a 1 (se interactuó con ella) y NO es la última tabla (por eso t + 1), entonces...
                {
                    tablesVec[t]->status = 0;                                     // Su estado pasa a ser cero, se pasa a cero ya que la función del TAB es cambiar la tabla, entonces "apaga" esta y "activa" la otra por decir así
                    tablesVec[t + 1]->status = 4;                                 // La siguiente tabla pasa a simular que recibió un clic al modificar status
                    tableSelected = tablesVec[t + 1]->name;                       // La tabla seleccionada pasa a ser la tabla que se acaba de activar
                    rqst = true; break;                                           // Se activa rqst y se realiza un break, ya no se necesita recorrer más este bucle
                }
                else if (tablesVec[t]->status > 1 && t + 1 == (int)tablesVec.size())    // Si el estado de la tabla actual es superior a uno, pero es la última tabla de la lista, entonces...
                {
                    tablesVec[t]->status = 0;                                           // Se apaga la tabla actual
                    tablesVec[0]->status = 4;                                           // Se activa la PRIMERA tabla, ya que no existe una siguiente por que es la última tabla de la lista
                    tableSelected = tablesVec[0]->name;                                 // La primera tabla se nombra como la tabla seleccionada
                    rqst = true; break;                                                 // Se activa rqst y se realiza un break
                }
            }
            else if (IsGestureDetected(GESTURE_TAP))                                    // En caso de que se haya recibido un clic...
            {
                if (tablesVec[t]->status == 4)                                          // Si alguna la tabla actual en el bucle recibió ese clic...
                {
                  tableSelected = tablesVec[t]->name;                                   // Declara esa tabla como la tabla seleccionada
                  rqst = true;                                                          // Activa a rqst
                }
            }

        }
        if (restartExplorar || rqst)                                                    // Si restartExplorar o rqst se encuentra activo, se procederán a actualizar los datos o a reiniciar la pestaña
        {
            if (restartExplorar)                                                        // Si específicamente quien se encuentra activo es restartExplorar, entonces reiniciará la pestaña
            {
                for (auto& tab : tablesVec) tab->status = 0;                            // Todas las tablas tendrán su status a 0, es decir, estarán apagadas sin recibir alguna interacción
                tablesVec[0]->status = 4;                                               // La primera tabla será la única en tener un status igual a 4, simulando que recibió un clic, esto hace que sea automáticamente seleccionada
                tableSelected   = tablesVec[0]->name;                                   // Se actualizará la variable tableSelected para almacenar el nombre de la primera tabla como la tabla seleccionada (principalmente para la funcion drawcolumns)
                while (!exploreList.empty()) exploreList.pop_back();                        // Procederá a vaciar a exploreList para reiniciarlo
                restartExplorar = false;                                                // restartExplorar se reinicia a false, ya que sino pasará por este if eternamente
                rqst = true;                                                            // rqst se activa a true, ya que necesita actualizar los datos por la selección de la primera tabla como la activa
            }
            if (rqst)                                                                   // Si rqst se encuentra activa, entonces procederá a realizar una query a la base de datos
            {
                adminButtons[4]->selfquery = "SELECT * FROM "s + tableSelected;         // Arma la query con la tabla actual, la query busca todos los valores de la tabla
                sendquery(adminButtons[4]->selfquery.data(), 0, 0, 1, " | ");           // Envía la query a la base de datos y le dice que use al carácter pipe | como separador
                adminButtons[4]->outLog = outQuery;                                     // Almacena la respuesta en el outLog de la pestaña actual (adminButtons[4] es la pestaña "Explorar")
                rqst = false; add = 0;                                                  // Desactiva a rqst y reinicia a add, add es una variable que sirve para el scroll en la información que aparezca, este scroll debe reiniciarse así que pasa a 0
            }
        }

        if (adminButtons[4]->outLog != "")                                              // Si actualmente NO está vacía la respuesta de la base de datos con respecto a todos los valores de la tabla
        {
            while (!exploreList.empty()) exploreList.pop_back();                                                    // Procederá a vaciar a exploreList para volverlo a llenar eventualmente
            explorarFinalOutput = "";                                                                               // Vacía la salida de los datos que se pasa en el frontend
            if (GetTouchX() > explorarSquare[0] && GetTouchX() < explorarSquare[0] + explorarSquare[2] &&           // Verificará si el cursor se encuentra en el ancho del objeto explorarSquare, el cual es el cuadro de fondo de "Explorar"
                GetTouchY() > explorarSquare[1] && GetTouchY() < explorarSquare[1] + explorarSquare[3] &&           // Y también si se encuentra en el alto del objeto, es decir, con las dos veificaciones se fija si está adentro del cuadro
                GetMouseWheelMove() != 0)                                                                           // Y también verifica que el scroll del mouse NO sea igual a 0, si el scroll sube, procederá a tener un valor 1, y si baja -1
                add += GetMouseWheelMove();                                                                         // En caso de que todo el if anterior fuera verdadero, entonces se le sumará un valor a add, el cual sirve para el scroll

            double possibleNLexplore = (explorarSquare[3] - explorarSquare[1] * 0.1) / (littleFontSize + nlSpacing);              // Verifica cuantas newlines pueden caber en el cuadro explorarSquare
            if (possibleNLexplore - (int)possibleNLexplore > 0.5) possibleNLexplore = (int)possibleNLexplore + 1;                 // Se redondea el valor para evitar newlines partidos, se suma uno si el decimal es mayor a 0.5
            else                                                   possibleNLexplore = (int)possibleNLexplore;                    // Se resta el decimal si NO es mayor a 0.5

            for (int ch = 0; ch < (int)adminButtons[4]->outLog.size(); ch++)                          // Ahora procederá a recorrer cada carácter en el outLog de la pestaña "Explorar", que es donde se almacenó la respuesta de la tabla actual
            {
                if (adminButtons[4]->outLog[ch] != '\n') line += adminButtons[4]->outLog[ch];         // Si el carácter actual NO es un newline, se le sumará el caracter actual al string line
                else { line += "\n"; exploreList.push_back(line); line = ""; }                        // Si el carácter actual SÍ es un newline, se le sumará el newline al string line, line se introducirá al vector exploreList, y se vaciará line
            }
            while ((int)exploreList.size() - add < possibleNLexplore) add -= 1;                       // Si add es mayor al tamaño total de exploreList, se le restará uno hasta que lo iguale, para prevenir que el scroll se desborde
            if (add < 0) add = 0;                                                                     // En cambio, si add es negativo, procederá a ser igual a cero, esto para también evitar que el scroll se desborde

            int newlns = 0;                                                             // newlns almacenará la cantidad de newlines que se han escrito dentro de explorarFinalOutput, el cual será el string de la respuesta para el frontend
            for (int l = 0 + add; l < (int)exploreList.size(); l++)                     // Se recorrerán todas las líneas del vector exploreList comenzando desde la variable add
            {
                if (add == 0) { if (newlns > possibleNLexplore - 1) break; }            // Si add es igual a 0 y la cantidad de newlines es mayor a la cantidad de newlines posibles menos 1, romperá el bucle
                else          { if (newlns >= possibleNLexplore) break; }               // En caso de que add NO sea igual a cero, entonces si la cantidad de newlines es mayor o igual a la de la cantidad de newlines posibles, romperá el bucle
                explorarFinalOutput += exploreList[l];                                  // La línea actual se suma al string explorarFinalOutput
                newlns++;                                                               // La cantidad de newlines que se agregan al string se suman uno por uno dentro de newlns
            }
        }
        else { explorarFinalOutput = "NO HAY DATOS\n"; }                                // Si actualmente el outLog de la pestaña "Explorar" se encuentra vacío, solo mostrará el string explorarFinalOutput diciendo "NO HAY DATOS"
    }

    // ── Resultados ────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[5])        // En caso de que la pestaña seleccionada sea "Resultados"
    {
        oldSelected = adminSelected;              // Actualiza a oldSelected por la misma razón por la que se actualiza en "Explorar"
        if (restartResultados)                    // restartResultados indica que debería de actualizar los resultados que muestra en pantalla, restartResultados se activa cuando se entra a esta pestaña
        {
            if (nullOption)                       // Si nullOption se encuentra activo, es decir, sí encontró un partido que sea la opción NULO, entonces procederá a hacer una consulta
            {
                std::string q1 = "SELECT "s + *nameColumnVotosNombre + " FROM "s + *nameTablePartidos +                         // q1 consulta acerca de la cantidad de votos de los partidos que NO sean el partido nulo
                                 " WHERE "s + *nameColumnPartidosNombre + " != '"s + *nameNuloOpcion + "';"s;
                sendquery(q1.data(), 0, 0);
                strresultados = outQuery;                                                                                       // La respuesta se almacena en strresultados
                std::string q2 = "SELECT "s + *nameColumnVotosNombre + " FROM "s + *nameTablePartidos +                         // q2 consulta acerca de la catidad de votos que tiene el partido nulo
                                 " WHERE "s + *nameColumnPartidosNombre + " = '"s + *nameNuloOpcion + "';"s;
                sendquery(q2.data(), 0, 0);
                strresultados += outQuery;                                                                                      // La respuesta se almacena en strresultados
            }
            else                                // Si no se encontró el partido nulo, entonces ejecuta la query normalmente
            {
                std::string q = "SELECT "s + *nameColumnVotosNombre + " FROM "s + *nameTablePartidos;
                sendquery(q.data(), 0, 0);
                strresultados = outQuery;     // Y almacena el resultado de todos los partidos en strresultados
            }
            std::string qAbs = "SELECT COUNT(*) FROM "s + *nameTableEstudiantes +           // Procede a realizar la query para obtener la cantidad de estudiantes cuyo valor de la columna Voto es igual a 0, es decir, no votaron
                               " WHERE "s + *nameColumnVotoNombre + " = '0';";
            sendquery(qAbs.data(), 0, 0);
            strresultados += outQuery;                                  // Y la cantidad de estudiantes que no votaron se almacena en strresultados, estos estudiantes serían el valor de la cantidad de abstensionismo
            restartResultados = false;            // Y como ya actualizó los datos, procede a desactivar a restartResultados
        }

        for (int chr = 0; chr < (int)strresultados.length(); chr++)           // Ahora, por cada carácter de strresultados...
        {
            if (strresultados[chr] != '\n') votes += strresultados[chr];      // Si el carácter NO es un newline, acumulará los carácteres recorridos en votes
            else if (!votes.empty()) {                                        // Ahora, si ocurre este if, significa que el carácter sí es un newline, y de paso verificará si votes NO está vacío, así que...
              percentages.push_back(std::stoi(votes));                        // Se introducirá el valor de votes como un valor entero con stoi, y se introducirá en el vector percentages
              votes = "";                                                     // Luego, vacía a votes, para almacenar el siguiente valor en el bucle
            }
        }

        resTogglePtr->status = isPressed(resTogglePtr);                       // Se verifica el estado del botón resToggle, el cual es el que se encuentra encima de la gráfica con un símbolo de # o de %, cambiará la gráfica
        if (resTogglePtr->status == 4)                                        // Si el botón fue presionado...
        {                                                                                                                         // outResultsMode es la variable que modifica el modo de mostrar la gráfica, si en porcentajes o en cantidades
            if ((std::string)outResultsMode == "quantity") { outResultsMode = "percentages"; resTogglePtr->name = "#"; }          // Si el valor de outResultsMode era "quantity", lo pasa a percentages, y cambia el símbolo del botón
            else                                           { outResultsMode = "quantity";    resTogglePtr->name = "%"; }          // Si el valor más bien era "percentages", cambiará el valor a "quantity", y cambiará el símbolo del botón
        }

        informePtr->status = isPressed(informePtr);                           // Se verifica el estado del botón inforne, el cual es el que se encarga de enviar la señal de generar un PDF
        if (informePtr->status == 4)                                          // Si el estado de informe es igual a 4, es decir, si el botón informe fue presionado...
        {
            statistics("backend", "percentages", percentages, partidosVec);   // Llamará a la función statistics, que internamente se encarga de convertir los valores de las votaciones de cantidades a porcentajes por medio de "percentages"
            std::vector<int> quantities;                                      // Declarará un vector llamado quantities, el cual almacenará las cantidades de los votos, NO porcentajes, ya que están ya almacenados en el vector percentages
            for (int chr = 0; chr < (int)strresultados.length(); chr++)       // Recorrerá cada carácter de strresultados, el cual por defecto ya almacena las cantidades de los votos
            {
                if (strresultados[chr] != '\n') votes += strresultados[chr];                          // Este bucle es casi el mismo que el que se encuentra alrededor de la línea 440, su propósito es guardar los valores del string strresultados
                else if (!votes.empty()) { quantities.push_back(std::stoi(votes)); votes = ""; }      // como valores enteros dentro del nuevo vector quantities
            }
            successfulPdfCreation = htmlreport(percentages, quantities);      // Ahora con los vectores que almacenan las cantidades y porcentajes, se puede llamar a inform() para crear el pdf, y el código de estado lo almacenará la variale
        }

        statistics("backend", outResultsMode, percentages, partidosVec);      // Esta función se encarga de siempre llamar la gráfica, se llama siempre al final de la pestaña
    }

    // ── Terminal ──────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[6])          // En caso de que la pestaña seleccionada sea igual a "Terminal"...
    {
        oldSelected = adminSelected;                // Se actualiza oldSelected por la misma razón que se actualiza en "Explorar"
        if (restartTerminal)                        // Si se debe de reiniciar la terminal, entonces...
        {
            if (adminAuthenticated) intentosRestantes = 3;                                          // Los intentos de autenticación se reinician a 3
            adminAuthenticated = false;                                                             // La variable que verifica si se autenticó a la terminal se asigna a false, para reiniciar la autenticación
            adminButtons[6]->outLog = "";                                                           // El log de la terminal se vacía, para no mostrar comandos digitados en sesiones anteriores
            for (int b = 0; b < (int)termBars.size(); b++)                                          // A continuación, un bucle recorrerá todas las barras de entrada de la terminal, estas barras son las que sirven para autenticarse
            { termBars[b]->input32 = U""; termBars[b]->input = ""; termBars[b]->status = 0; }       // Y procederá a vaciar todas las barras, y las declarará como inactivas con status igual a cero
            termBars[0]->status = 4;                                                                // Luego, procederá a declarar automáticamente a la primera barra como activa
            restartTerminal = false;                                                                // Además, desactivará el reinicio de la terminal para que no se reinicie eternamente
            invalidIp = false; invalidCredentials = false; inputEmpty = false;                      // Y reiniciará las variables que avisan si se digitó una IP inválida, si las credenciales son inválidas, o si alguna credencial está vacía
            return;
        }
        else if (adminAuthenticated)              // Si el administrador SÍ se encuentra autenticado...
        {
            adminTerminalPtr->status   = isPressed(adminTerminalPtr);                                                     // El cuadro de la terminal estará constantemente revisando su estado
            barAdminTerminalPtr->status = isPressed(barAdminTerminalPtr);                                                 // Como también, la barra de comandos estará revisando su estadp
            if (adminTerminalPtr->status > 1 || barAdminTerminalPtr->status > 1)                                          // Si se está interactuando con el cuadro de la terminal o con la barra de comandos, entonces...
            {
                if (adminTerminalPtr->status > 1 && barAdminTerminalPtr->status == 0)                                     // En caso de que sea de que se interactúe en este instante con el cuadro y NO con la barra de comandos, entonces...
                { adminTerminalPtr->status = 0; barAdminTerminalPtr->status = 2; }                                        // Asignará al cuadro de la terminal como inactivo y a la barra como activa, ya que es la que debe de recibir la input
                inputfunc("backend", barAdminTerminalPtr, 1024, "allchars-admin", littleFontSize, BLANCO, 6);              // Ahora, a la barra de comandos se le estará recibiendo datos de entrada a través de inputfunc()
                if (IsKeyPressed(KEY_ENTER))                                                                              // Si se presiona la tecla ENTER...
                {
                    if (barAdminTerminalPtr->input != "clear") {                                          // Si el comando digitado NO es igual a "clear"
                        outputTerm = ptyfunc(barAdminTerminalPtr->input,                                                      // outputTerm almacenará los datos de salida de la función ptyfunc(), la cual se encarga de enviar el comando digitado
                                             termBars[3]->input, termBars[4]->input,                                          // A través de las credenciales previamente usadas para la autenticación
                                             termBars[0]->input, termBars[1]->input,
                                             termBars[2]->input);
                        adminButtons[6]->outLog += barAdminTerminalPtr->input + "\n" + outputTerm;                            // El log de los comandos se actualizará con el comando digitado, un newline, y la respuesta recibida del comando
                        logCommands.push_back(barAdminTerminalPtr->input);                                                    // Y el comando digitado se guardará en el historial deo comandos logCommands
                        barAdminTerminalPtr->input32 = U"";                                                                   // Se vaciará la barra de datos de entrada
                        barAdminTerminalPtr->input   = "";
                        logpos = 0;                                                                                           // Y la posición en el historial se reiniciará a cero
                    }
                    else                        //  En caso de que el comando SÍ era igual a clear, procederá a limpiar la terminal
                    {
                        barAdminTerminalPtr->input32 = U"";                                             // Se vaciará la barra de datos de entrada
                        barAdminTerminalPtr->input   = "";
                        adminButtons[6]->outLog = "";                                                   // Se vacía el log
                        logpos = 0;                                                                     // Reiniciará la posición en el historial
                    }
                }
            }
        }
        else                                // Si el administrador NO se encuentra autenticado...
        {
            for (int b = 0; b < (int)termBars.size(); b++)                                                        // Recorrerá cada barra de entrada encargada de recibir las credenciales de acceso
            {
                if (IsKeyPressed(KEY_TAB))                                                                        // Si se presiona la tecla TAB podrá cambiar de barra de entrada rápidamente a la siguiente o a la primera
                {
                    if (termBars[b]->status > 1 && b + 1 < (int)termBars.size())                                  // Si la barra actual en el bucle está recibiendo interacción alguna ya que su estado > 1 y NO es la última barra...
                    {
                        termBars[b]->status = 0;                                                                  // Su estado cambiará a cero
                        termBars[b + 1]->status = 2;                                                              // El estado de la siguiente barra cambiará a 2, suficiente para activarse y recibir datos de entrada
                        beam = 0; break;                                                                          // Reiniciará el parpadeo de la barra parpadeante y romperá el bucle
                    }
                    else if (termBars[b]->status > 1 && b + 1 == (int)termBars.size())                            // Si la barra actual está recibiendo interacción alguna ya que su estado es mayor a 1 y SÍ es la última barra...
                    {
                        termBars[b]->status = 0;                                                                  // Su estado cambiará a cero
                        termBars[0]->status = 2;                                                                  // El estado de la primera barra cambiará a 2
                        beam = 0; break;                                                                          // Reiniciará el parpadeo de la barra parpadeante y romperá el bucle
                    }
                }
                else termBars[b]->status = isPressed(termBars[b]);                                                // En caso de que NO se presione la tecla tab, solo estará vigilando el estado de cada barra

                if (termBars[b]->status > 1)                                                                      // Si el estado de la barra actual es superior a 1, es decir, se está interactuando con esa barra
                {
                    if (termBars[b]->name != termBars[1]->name)                                                   // Y si la barra actual no es la del índice 1 (osea la segunda, la cual recibe el puerto a conectar) entonces...
                        inputfunc("backend", termBars[b], 45, "allchars", mediumFontSize, BLANCO);                 // Procederá a recibir datos de cualquier tipo de carácter
                    else                                                                                          // En cambio de que sí sea la del índice 1 (la segunda, la del puerto a conectar) entonces...
                        inputfunc("backend", termBars[b], 10, "numberonly", mediumFontSize, BLANCO);               // Solo recibirá datos de entrada que sea únicamente números
                }

                if (IsKeyPressed(KEY_ENTER))                                                                      // Si se presiona la tecla ENTER, significa que se desea conectar a la base de datos con las credenciales proveídas
                {
                    if (!termBars[0]->input.empty() && !termBars[1]->input.empty() &&                             // Se comprueba si ninguna barra excepto la de password está vacía (ya que dependiendo la configuración, password puede ser vacía)
                        !termBars[2]->input.empty() && !termBars[3]->input.empty())
                    {
                        if (validIP(termBars[0]->input))                                                          // Se verificará si la IP es válda, si SÍ es válida, entrará al siguiente para la autenticación
                        {
                            auth = mysql_init(NULL);                                                              // Inicializa el puntero auth, el cual almacenará la dirección en memoria de la autenticación desde la terminal
                            if (!mysql_real_connect(auth,                                                         // Si mysql_real_connect() llega a tener un error significa que las credenciales NO son válidas, entonces...
                                                    termBars[0]->input.data(), termBars[3]->input.data(),
                                                    termBars[4]->input.data(), termBars[2]->input.data(),
                                                    std::stoi(termBars[1]->input), NULL, 0))
                            {
                                mysql_close(auth); auth = nullptr;                                                // Se cerrará la conexión a la que apuntaba el puntero auth, y declarará a auth como un puntero vacío, para limpiar la conexión
                                intentosRestantes--;                                                              // Resta los intentosRestantes
                                invalidCredentials = true;                                                        // Activa a invalidCredentials, para avisar en el frontend que las credenciales son inválidas
                                break;
                            }
                            else adminAuthenticated = true;                                                       // Si el if anterior no llega a tener un error, significa que las credenciales SÍ son válidas, entonces adminAuthenticated será true
                        }
                        else invalidIp = true;                                                                    // En caso de que la IP sea inválida, declarará a invalidIp como true para avisar de esto y enviar un mensaje en el frontend
                    }
                    else inputEmpty = true;                                                                       // En caso de que haya alguna barra vacía, declarará a inputEmpty como true, para avisar de esto y enviar un mensaje en el frontend
                }
            }
        }
    }
}

// ── Frontend ──────────────────────────────────────────────────────────────────
void screenAdminmenuDraw(Screen &currentScreen,
                         bool &invalidCredentials,                                              // Llama a variables que
                         bool &inputEmpty,                                                      // sirven para verificar
                         bool &invalidIp,                                                       // principalmente errores
                         bool &adminAuthenticated_local,                                        // en la autenticación en "Terminal"
                         bool &successfulPdfCreation,                                           // y la creación del pdf de informe
                         const std::string &explorarFinalOutput,                                // como también la output de "Explorar"
                         std::string& modeInput,                                                // el modo de entrada de la columna actual
                         std::string &outResultsMode)                                           // y el tipo de modo para la gráfica en "Resultados"
{
    if (currentScreen == ADMINMENU) transition("show");
    exitAdminPtr->status        = isPressed(exitAdminPtr);           // Se empieza a verificar el estado
    enterConfigPtr->status      = isPressed(enterConfigPtr);         // de los botones de salir, actualizar datos
    refreshPtr->status          = isPressed(refreshPtr);             // o ingresar a la pantalla CONFIGURATION
    cambiarFrontendPtr->status  = isPressed(cambiarFrontendPtr);     // o cambiar el modo del frontend
    backupPtr->status           = isPressed(backupPtr);              // o ejecutar un backup de la base de datos
    resetDataPtr->status        = isPressed(resetDataPtr);           // o ejecutar un backup de la base de datos
    // Se actualiza el estado de cada botón no seleccionado cada frame para que PrettyDrawRectangle
    // detecte el hover y cambie el grosor del borde dorado correctamente
    for (int i = 0; i < (int)adminButtons.size(); i++)
        adminButtons[i]->status = isPressed(adminButtons[i]);
    DrawRectangle(adminPanel[0], adminPanel[1],                     // Dibuja el cuadro del fondo
              adminPanel[2], adminPanel[3], VOCADORADOSUAVE);
    DrawLineEx((Vector2){configPanel[0], adminPanel[1]},
           (Vector2){(configPanel[0]+configPanel[2]), adminPanel[1]},
            4.0f, DORADO_BORDE);
    drawSelected(adminButtons, littleFontSize, adminSelected);                                  // Empieza a dibujar los botones de las pestañas del panel de administración
    DrawTextEx(fontTtf, "PANEL DE ADMINISTRACIÓN"s.data(),                                      // Y escribe el título "PANEL DE ADMINISTRACIÓN"
               (Vector2){(float)centertext("PANEL DE ADMINISTRACIÓN"s, screenWidth, fontSize),
                          (float)(screenHeight * 0.03)},
               fontSize, 2, COLORTEXTO);

    // ── Consultar ──────────────────────────────────────────────────────────────────────────
    if (adminSelected == butnames[0])                                                                         // Si la pestaña actual es "Consultar"
        oldSelected = drawcolumns(tablesVec, columnsVec, tableSelected, littleFontSize, adminSelected);       // Dibuja las columnas y tablas normalmente
    // ── Agregar ────────────────────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[1])                                                                    // Si la pestaña actual es "Agregar"
        oldSelected = drawcolumns(tablesVec, columnsVec, tableSelected, littleFontSize, adminSelected);       // Dibuja las columnas y tablas normalmente
    // ── Actualizar ─────────────────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[2])                                                                    // Si la pestaña actual es "Actualizar"
    {
        opcionActPtr->status = isPressed(opcionActPtr);
        oldSelected = drawcolumns(tablesVec, columnsVec, tableSelected, littleFontSize, adminSelected);       // Dibuja las columnas y tablas normalmente y...
        PrettyDrawRectangle(opcionActPtr);                                                                    // Dibuja la opción de la lista desplegable
        if (opcionActPtr->status > 1)                                                                         // Si el estado del botón es mayor a uno, es decir, está recibiendo interacción alguna, entonces...
        {
            for (int counter = 0; counter < (int)opcionesAct.size(); counter++)                               // Recorrerá todo el vector de opciones y...
            {
                opcionesAct[counter]->status = isPressed(opcionesAct[counter]);                               // Se actualiza el estado de cada opción cada frame para detectar hover correctamente
                PrettyDrawRectangle(opcionesAct[counter]);                                                    // Dibujará el cuadro de cada opción
                DrawTextEx(fontTtf, opcionesAct[counter]->name.data(),                                        // Además del nombre de cada opción
                           (Vector2){(float)(opcionActPtr->xloc + littleFontSize),
                                      (float)(opcionesAct[counter]->yloc + (opcionesAct[counter]->ysize * 0.5) - (littleFontSize * 0.5))},
                           littleFontSize, 2, NEGRO);
            }
        }
        else                                                                                                  // En caso de que el estado del botón NO es mayor a uno, es decir, no se está interactuando con él
        {
            if (opcSelectedPtr->name == opcionActPtr->name)                                                   // Verifica si opcSelected tiene el nombre por defecto de opcionAct
            {
                DrawTextEx(fontTtf, opcionActPtr->name.data(),                                                // Y dibuja el nombre por defecto de opcionAct
                           (Vector2){(float)(opcionActPtr->xloc + littleFontSize),
                                      (float)(opcionActPtr->yloc + (opcionActPtr->ysize * 0.5) - (littleFontSize * 0.5))},
                           littleFontSize, 2, NEGRO);
            }
            else                                                                                              // Si la opción opcSelected NO es el nombre por defecto de opcionAct, es decir, SÍ se seleccionó una opción de la lista desplegable
            {
                DrawTextEx(fontTtf, opcSelectedPtr->name.data(),                                              // Escribirá el nombre de la opción seleccionada
                           (Vector2){(float)(opcionActPtr->xloc + littleFontSize),
                                      (float)(opcionActPtr->yloc + (opcionActPtr->ysize * 0.5) - (littleFontSize * 0.5))},
                           littleFontSize, 2, NEGRO);
                PrettyDrawRectangle(actBarPtr);                                                               // Procederá a dibujar la barra de datos de entrada
                if      (opcSelectedPtr->type == "tinyint") modeInput = "boolean";                            // Y declarará el modo de datos de entrada
                else if (opcSelectedPtr->type == "int")     modeInput = "regexponly";
                else if (opcSelectedPtr->type == "varchar") modeInput = "allchars";
                inputfunc("frontend", actBarPtr, opcSelectedPtr->maxlen, modeInput, littleFontSize);          // Para luego mostrar los datos introducidos en la barra de datos de entrada
            }
        }
    }
    // ── Borrar ─────────────────────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[3])                                                                    // Si la pestaña actual es "Borrar"
        oldSelected = drawcolumns(tablesVec, columnsVec, tableSelected, littleFontSize, adminSelected);       // Dibuja las columnas y tablas normalmente
    // ── Explorar ───────────────────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[4])                                                                    // Si la pestaña actual es "Explorar"
    {
        DrawRectangle(explorarSquare[0], explorarSquare[1], explorarSquare[2], explorarSquare[3],             // Dibuja el cuadro de fondo de los datos de "Explorar"
                      darkMode ? (Color){45, 45, 48, 255} : (Color){220, 200, 160, 255});
        drawSelected(tablesVec, littleFontSize, tableSelected);                                               // Dibuja las tablas
        DrawTextEx(monoTtf, explorarFinalOutput.data(),                                                       // Y dibuja la salida de la información de la tabla actual
                   (Vector2){(float)(screenWidth * 0.13), (float)(explorarSquare[1] * 1.095)},
                   littleFontSize, 2, COLORTEXTO);
    }
    // ── Resultados ─────────────────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[5])                                                                    // Si la pestaña actual es resultados
    {
        statistics("frontend", outResultsMode, percentages, partidosVec, screenWidth * 0.15, screenHeight * 0.5);               // Dibujará el gráfico
        resTogglePtr->status = isPressed(resTogglePtr);
        PrettyDrawRectangle(resTogglePtr);                                                                                      // Dibujará el botón encargado de cambiar de porcentajes a cantidades
        DrawTextEx(fontTtf, resTogglePtr->name.data(),                                                                          // Dibujará el nombre del botón centrado dentro de él
                   (Vector2){resTogglePtr->xloc + (float)centertext(resTogglePtr->name, resTogglePtr->xsize, littleFontSize),
                              (float)(resTogglePtr->yloc + (resTogglePtr->ysize - littleFontSize) / 2)},
                   littleFontSize, 2, NEGRO);                                                                                   // NEGRO porque el botón es beige
        informePtr->status = isPressed(informePtr);                                           // Se actualiza el estado del botón cada frame para detectar hover correctamente
        PrettyDrawRectangle(informePtr);                                                      // Dibujará el botón para hacer el informe en PDF
        DrawTextEx(fontTtf, informePtr->name.data(),                                          // Dibujará el nombre del botón del PDF centrado dentro de él
                   (Vector2){informePtr->xloc + (float)centertext(informePtr->name, informePtr->xsize, littleFontSize),
                              (float)(informePtr->yloc + (informePtr->ysize - littleFontSize) / 2)},
                   littleFontSize, 2, NEGRO);                                                 // NEGRO porque el botón es beige
        // Verificación de errores de la función informe() relacionados a la creación del PDF
        if (pdfError)            shortmessage("ERROR: Ocurrio un error al crear el PDF", fontSize, pdfError);                     // Si se activa pdfError desde la función informe(), mostrará ese mensaje
        else if (pdfFontError)   shortmessage("ERROR: Ocurrio un error al cargar la fuente de texto", fontSize, pdfFontError);    // Si hubo un error con la fuente de texto, se activa pdfFontError desde la función informe(), y muestra ese mensaje
        else if (pdfRandomError) shortmessage(pdfErrorString, fontSize, pdfRandomError);                                          // Si hubo un error muy específico desde la función informe(), entonces muestra el código de error
        else if (successfulPdfCreation) shortmessage("PDF creado con exito!", fontSize, successfulPdfCreation);                   // Si el PDF se creó con éxito, muestra ese mensaje
    }
    // ── Terminal ───────────────────────────────────────────────────────────────────────────
    else if (adminSelected == butnames[6])                              // Si la pestaña actual es "Terminal"
    {
        if (adminAuthenticated)                                         // Si el administrador se encuentra autenticado...
        {
            DrawRectangle(                                              // Dibujará el cuadro de la terminal
                adminTerminalPtr->xloc,
                adminTerminalPtr->yloc,
                adminTerminalPtr->xsize,
                adminTerminalPtr->ysize,
                NEGRO
                );
            DrawRectangle(                                              // Y también la barra de comandos de la terminal
                barAdminTerminalPtr->xloc,
                barAdminTerminalPtr->yloc,
                barAdminTerminalPtr->xsize,
                barAdminTerminalPtr->ysize,
                NEGRO
                );
            inputfunc("frontend", barAdminTerminalPtr, 1024, "allchars-admin", littleFontSize, BLANCO);        // Además de que dibujará los datos de entrada que se están escribiendo en la terminal
            logfunction(adminSelected);                                                                       // Luego, muestra los datos con logfunction()
        }
        else if (!adminAuthenticated && intentosRestantes > 0)                            // Verifica si no se han agotado los intentos de autenticación, en caso de que no se hayan agotado, entonces...
        {
            for (int b = 0; b < (int)termBars.size(); b++)                                // Recorrerá todas las barras de entrada para las credenciales de autenticación
            {
                DrawTextEx(fontTtf, termBars[b]->name.data(),                             // Dibujará el nombre de cada barra
                           (Vector2){(float)(screenWidth * 0.12),
                                      (float)(termBars[b]->yloc + (termBars[b]->ysize * 0.5) - (littleFontSize * 0.5))},
                           littleFontSize, 0, NEGRO);
                PrettyDrawRectangle(termBars[b]);                                         // Y también dibujará la barra misma
                inputfunc("frontend", termBars[b], 0, "allchars", littleFontSize);        // Además de dibujar los dtos que se hayan introducido en la barra actual del bucle
            }
        }
        else                          // En caso de que se hayan agotado los intentos de autenticación...
        {
            DrawTextEx(fontTtf, "Ha agotado sus intentos, retirese"s.data(),              // Mostrará un mensaje diciendo que se agotaron los intentos
                       (Vector2){(float)centertext("Ha agotado sus intentos, retirese"s, screenWidth, fontSize),
                                  (float)(screenHeight * 0.5)},
                       fontSize, 2, NEGRO);
        }
        // Verificación de errores relacionados a la autenticación
        if (invalidCredentials)                 // Si las credenciales son inválidas...
            shortmessage("Credenciales invalidas, tiene "s + std::to_string(intentosRestantes) + " intentos restantes"s,        // Mostrará el siguiente mensaje con la cantidad de intentos restantes
                         mediumFontSize, invalidCredentials);
        else if (inputEmpty) shortmessage("Los datos se encuentran vacios", fontSize, inputEmpty);          // Si hay credenciales vacías, mostrará el siguiente mensaje
        else if (invalidIp)  shortmessage("La IP digitada es invalida",     fontSize, invalidIp);           // Si la IP es inválida, mostrará el siguiente mensaje
    }

    // Dibuja al cuadro de advertencia en caso de que haya alguna

    if (execConfirmation == 100)
    {
        alert(adminSelected, "frontend");
    }

    // Botones en las esquinas

    if (backupReleased) shortmessage(backupRetString, mediumFontSize, backupReleased);      // Si se realizó un backup, muestra el mensaje de respuesta del backup
    if (successReset) shortmessage("El reinicio de los datos fue realizado con éxito", mediumFontSize, successReset);      // Si se realizó el reinicio exitosamente, muestra este mensaje
    if (errorReset) shortmessage("El reinicio de los datos tuvo errores", mediumFontSize, errorReset);      // Si se realizó el reinicio con fallos, muestra este mensaje


    PrettyDrawRectangle(enterConfigPtr);      // Dibuja el botón para entrar a la configuración del programa
    PrettyDrawRectangle(exitAdminPtr);        // Dibuja el botón para salir del panel de administración
    PrettyDrawRectangle(refreshPtr);          // Dibuja el botón para refrescar los datos cargados desde la base de datos
    PrettyDrawRectangle(cambiarFrontendPtr);  // Dibuja el botón para cambiar el modo del frontend de claro a oscuro
    PrettyDrawRectangle(backupPtr);           // Dibuja el botón para refrescar los datos cargados desde la base de datos
    PrettyDrawRectangle(resetDataPtr);        // Dibuja el botón para resetear los datos de la base de datos
    if (currentScreen != ADMINMENU && alphaIsZero == false)
    {
        transition("hide");
        screenAdminmenuDraw(currentScreen,
                            invalidCredentials, inputEmpty, invalidIp,
                            adminAuthenticated, successfulPdfCreation,
                            explorarFinalOutput, modeInput, outResultsMode);
    }
}
