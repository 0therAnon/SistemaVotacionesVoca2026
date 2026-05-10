// PrettyDrawRectangle and drawSelected bodies live in drawing.hpp (template functions).
// This file contains only the non-template drawing utilities.
#include "drawing.hpp"
#include <string>
#include <vector>

int centertext(std::string message, double width, double fontsize)              // Esta función se encarga de centrar texto, de argumentos tiene al mensaje a centrar, el tamaño en pixeles para centrar el texto, y el tamaño del font de la letra
{
    Vector2 textWidth = MeasureTextEx(fontTtf, message.data(), fontsize, 2);    // MeasureTextEx() sirve para centrar el texto a partir del font, el mensaje, el tamaño del font y el espaciado entre las letras, y esto lo guarda en textWidth
    return (int)((width - textWidth.x) / 2);
}

std::string adjustLimit(const std::string &text, int n = 1000)                  // Sirve para ajustar el máximo de las propiedades outLog de los botones del panel de administración, cortando todo lo anterior a las 1000 líneas más recientes
{                                                                               // Sin esta función, en pestañas como "Consultar" que pueden llegar a cargar una gran catidad de datos, podría provocar un poco de lag en el programa
    int count = 0;                                                              // Se declara count en cero, esta variable sirve para contar la cantidad de newlines que contó desde el final
    size_t pos = text.size();                                                   // pos sirve para empezar desde el final del string

    while (pos > 0) {                                                           // En caso de que pos siga siendo mayor a 0, osea, mientras no se haya terminado de recorrer
        pos = text.rfind('\n', pos - 1);                                        // Busca el newline más cercano hacia la izquiera desde pos (es decir, busca desde el final del string)
        if (pos == std::string::npos) {break;}                                  // Si rfind() no encontró más newlines entonces llegó al final del texto, procede a salir del bucle
        count++;                                                                // Va sumando cuántos newlines encuentra
        if (count == n) {                                                       // Cuando ya contró n cantidad de líneas, procederá a...
            return text.substr(pos + 1);                                        // retornar el texto, ya cortado
        }
    }

    return text;                                                                // Si el texto llega a tener una cantidad de líneas menor al límite, entonces lo devuelve normal
}

// Esta función a continuación se encarga de dibujar el cuadro de resultados en el panel de administración, si por ejemplo, se hace una consulta en el panel "Consultar", muestra el resultado

int logfunction(std::string selected,                                                               // Necesita averiguar cuál pestaña de administración se encuentra activa
                double lastColumnMeasures,                                                          // Busca dónde termina la última columna dibujada arriba del cuadro de resultados
                double fsize)                                                                       // Pide también como argumento, el tamaño del font para dibujar las letras de los resultados
{
    std::vector<std::string> logList;                                                               // Lista donde se guardarán todas las líneas del texto a mostrar, servirá mucho para el scroll
    std::string line = "";                                                                          // Acumulador temporal para construir cada línea
    std::string log  = "";                                                                          // El texto final que se dibujará en pantalla
    int modifiedFontSize = littleFontSize + (1 - littleFontSize) * 0.4;                             /* Calcula un tamaño de fuente especial usando interpolación lineal
                                                                                                       esto para conseguir el valor intermedio entre un font en una pantalla 1920*1080 y 1366*768
                                                                                                       para calcular dónde es el límite de un renglón en la pantalla y que no se desborden carácteres fuera del cuadro de resultados
                                                                                                       https://en.wikipedia.org/wiki/Linear_interpolation */

    if (selected == adminButtons[2]->name) lastColumnMeasures += actBarPtr->ysize + (fsize * 2);    /* La pestaña de actualizar (botones[2]) tiene una barra de entrada extra en pantalla, entonces lastColumnMeasures
                                                                                                      (que indica dónde empieza el recuadro verticalmente) se aumenta para no tapar esa barra */

    // outSquare es el cuadro en el que se mostrarán los datos, solo que hay que definir sus coordenadas, alto y ancho
    // Aquí hay dos casos, se encuentra en la pestaña "Terminal" o no:

    if (selected != adminButtons[6]->name || lastColumnMeasures != 0)                               // Si NO se encuentra en "Terminal"...
    {                                                                                               // Se calcula el recuadro centrado horizontalmente en la pantalla, el array outSquare[4] guarda las 4 coordenadas del rectángulo
        outSquare[0] = (screenWidth * 0.5) - ((screenWidth * 0.76) * 0.5);
        outSquare[1] = lastColumnMeasures;
        outSquare[2] = screenWidth * 0.76;
        outSquare[3] = screenHeight - lastColumnMeasures - screenHeight * 0.04;
        DrawRectangleRounded({outSquare[0], outSquare[1], outSquare[2], outSquare[3]},
                             0.02f, 0, NEGRO);                                              // Se dibuja el fondo negro del cuadro de resultados con esquinas ligeramente redondeadas
        DrawRectangleRoundedLinesEx({outSquare[0], outSquare[1], outSquare[2], outSquare[3]},
                                    0.02f, 0, 3.0f, DORADO_BORDE);                         // Se dibuja el borde dorado alrededor del cuadro de resultados, con el mismo color dorado que usan todos los botones del programa
    }
    else                                                                                            // Si SÍ se encuentra en "Terminal"...
    {                                                                                               // El recuadro usa las coordenadas del componente adminTerminal, que ya tiene su posición fija definida, sin recalcular nada
        outSquare[0] = adminTerminalPtr->xloc;
        outSquare[1] = adminTerminalPtr->yloc;
        outSquare[2] = adminTerminalPtr->xsize;
        outSquare[3] = adminTerminalPtr->ysize*0.98;                            // Se le resta un piquito, para que visualmente no choque con barAdminTerminalPtr, ya que ese objeto está justo deajo de este
    }

    int cnt           = 0;                                      // cnt actúa como contador para cuántos carácteres lleva la línea desde que inició, y si la línea sobrepasa el valor de carácteres límite, introducirá un newline
    int ch            = 0;                                      // ch actúa como acumulador de la cantidad de carácteres por pixeles que se puedan introducir dependiendo del ancho de outSquare
    int possibleChars = 0;                                      // possibleChars verifica la cantidad máxima de carácteres que se pueden introducir dependiendo del valor que dé como resultado el bucle siguiente

    while (ch < outSquare[2] - (outSquare[2] * 0.02))                       // Inicio del bucle, mientras la cantidad de pixeles que llevan las letras contadas sea menor al ancho de outSquare con un poquito menos de su tamaño real, no terminará
    {
      possibleChars++;                                                      // Se le suma un valor a possibleChars, va sumando mientras el bucle siga activo, es decir, ch no sobrepase el tamaño del ancho de outSquare
      ch = (int)(possibleChars * modifiedFontSize);                         // ch almacena la cantidad que possibleChars suministra y la multiplica por modifiedFontSize (el tamaño del font calculado con interpolación lineal)
    }                                                                       // El bucle terminará, cuando ch sobrepase el ancho de outSquare, y el que acumula la cantidad de carácteres posibles será entonces possibleChars

    double possibleNL = (outSquare[3] - (outSquare[3] * 0.05)) / (littleFontSize + nlSpacing);      // Se divide el alto de outSquare (outSquare[3] - outSquare[3] * 0.05) dejando un márgen de 5%, y lo divide por el alto de cada línea (nlSpacing)
    if (possibleNL - (int)possibleNL > 0.5) possibleNL = (int)possibleNL + 1;                       // Al haber calculado los newlines posibles en el cuadro de resultados, se redondea, para evitar carácteres con la mitad salíendose del cuadro
    else                                    possibleNL = (int)possibleNL;

    if (GetMouseWheelMove() != 0 &&                                                                 // Si se detecta que la rueda del mouse o el touchpad dan la orden de desplazar hacia arriba (estado 1)) o hacia abajo (estado -1)...
        GetTouchX() > outSquare[0] && GetTouchX() < outSquare[0] + outSquare[2] &&                  // Se verifica que el cursor se encuentre en el área X del cuadro y...
        GetTouchY() > outSquare[1] && GetTouchY() < outSquare[1] + outSquare[3])                    // también se verifica que se encuentre en el tamaño Y, si todo esto se cumple, se procederá a ejercer el movimiento de scroll
        add += GetMouseWheelMove();                                                                 // add almacenará el valor del scroll, GetMouseWheelMove() devuelve estado 1 si se mueve hacia arriba, y -1 si es hacia abajo

    logList.reserve(possibleNL);                                                                    

    for (int opt = 0; opt < (int)adminButtons.size(); opt++)                                        // Se recorre todo el vector de las pestañas del panel de administración...
    {
        if (selected == adminButtons[opt]->name && adminButtons[opt]->outLog != "")                 // Si la pestaña actual en el vector, coincide con la pestaña activa...
        {
            int cntNL = 0;                                                                          // cntNL actúa como contador, el cual se encargará de verifica que outLog no tenga más de 1000 líneas
            for (char c : adminButtons[opt]->outLog) {                                              // Recorre todo el outLog, el cual es la propiedad que almacena todo el registro del historial de respuestas de la pestaña actual
                if (c == '\n') {                                                                    // Si el carácter acual en el bucle SÍ es un newline, entonces...
                    cntNL++;                                                                        // Procederá a sumar un valor al contador
                }
            }
            if (cntNL > 1000) {adminButtons[opt]->outLog = adjustLimit(adminButtons[opt]->outLog, 1000);}     // Cuando el anterior bucle haya terminado, se verificará si outLog tiene más de 1000 líneas, si es así, lo regulará con adjustLimit()

            for (ch = 0; ch < (int)adminButtons[opt]->outLog.size(); ch++)                          // Este bucle lo que hará será recorrer todos los carácteres de outLog e insertar un newline si sobrepasa el límite impuesto por possibleChars
            {
                cnt++;                                                                              // Suma un carácter al contador
                if (cnt <= possibleChars && adminButtons[opt]->outLog[ch] == '\n')                  // Se verifica si el carácter actual es un newline, si es así, reinicia el contador
                {
                    cnt = 0;                                                                        // Reinicio del contador
                }
                else if (cnt >= possibleChars && adminButtons[opt]->outLog[ch] != '\n')             // Si el carácter NO es un newline y sobrepasa el límite de possibleChars, entonces...
                {
                    adminButtons[opt]->outLog.insert(ch, "\n");                                     // Inserta un newline
                    cnt = 0;                                                                        // Reinicia el contador
                    break;                                                                          // Rompe el bucle, esto hace que el bucle externo vuelva a empezar, hasta que todo texto esté bien dividido en newlines
                }
            }
            for (ch = 0; ch < (int)adminButtons[opt]->outLog.size(); ch++)                          // Ahora que el texto ya tiene los \n correctos, se vuelve a recorrer caracter por caracter
            {
                if (adminButtons[opt]->outLog[ch] != '\n')                                          // Si el carácter NO es un newline...
                {
                    line += adminButtons[opt]->outLog[ch];                                          // Se agrega el carácter al acumulador line
                }
                else                                                                                // Si el carácter SÍ es un newline...
                {
                    line += "\n";                                                                   // Se agrega el newline al acumulador line
                    logList.insert(logList.begin(), line);                                          // line se inserta AL INICIO de logList, esto facilida el scroll
                    line = "";                                                                      // Se procede a limpiar a line, para seguir con una línea después
                }
            }

            while ((int)logList.size() - add < possibleNL) {add -= 1;}                    // Si add es tan grande que quedarían menos líneas visibles que possibleNL, se reduce add hasta que haya suficiente contenido

            if (add < 0) add = 0;                                                         // Si add quedó negativo (se scrolleó demasiado hacia abajo), se resetea a 0

            /*  Este bucle a continuación recorre logList empezando desde la posición add (el offset del scroll),
                toma hasta possibleNL líneas y las inserta al inicio de log, como logList ya estaba invertido y ahora
                se inserta al inicio de nuevo, el resultado final en log tiene las líneas en el orden correcto para mostrar  */

            int countr = 0;                                                               // countr actúa también como contador
            for (int ln = 0 + add; ln < (int)logList.size(); ln++)                        // Se recorre logList desde la posición add
            {
                if (countr >= (int)possibleNL) break;                                     // Si la cantidad de líneas que recibe log es mayor a la cantidad posible, el bucle se rompe
                log.insert(0, logList[ln]);                                               // Si no ocurre el if, entonces se sigue agregando al inicio de log el primer string de logList (el cual sería el último recibido por que está invertido)
                countr++;                                                                 // countr va contando la cantidad de inserciones al string log
            }
            DrawTextEx(monoTtf, log.data(),                                               // Se procede a dibujar el texto encima de outSquare
                       (Vector2){(float)(outSquare[0] + screenWidth * 0.01),
                                  (float)(outSquare[1] + outSquare[3] * 0.05)},
                       littleFontSize, 2, BLANCO);
        }
    }
    return 0;                             // Retorna 0, código de estado exitoso
}

/* Esta función a continuación se encarga de dibujar las barras de entrada de datos del panel de administración, se le llama drawcolumns por que las barras de entrada de datos corresponde a las columnas cargadas de cada tabla
   ya que por ejemplo, si el administrador selecciona la tabla "Estudiantes", drawcolumns() lo que hace es dibujar la barra de datos de cédula, nombre, los apellidos, y el estado del votante.
   Esta función también detecta click y maneja los datos de entrada, así que es como una combinación entre backend y frontend, pero siempre debe de invocarse en las secciones de frontend. */

std::string drawcolumns(std::vector<sqlobject*> &cTables,                   // Vector de las tablas cargadas desde la base de datos
                        std::vector<column*> &cVector,                      // Vector de todas las columnas cargadas desde la base de datos
                        std::string &tSelected,                             // Tabla actual seleccionada
                        double fsize,                                       // Tamaño del font
                        std::string &selected,                              // Pestaña activa del panel de administración
                        char* mode)                                         // Modo de operación
{
    quancolumns = 0;                                                        // Contador global que lleva cuántas columnas se han dibujado, para posicionarlas verticalmente una debajo de la otra
    int lastColumnMeasures = 0;                                             // Guardará la coordenada Y donde terminó la última columna dibujada, para que logfunction sepa dónde empezar el recuadro de resultados
    std::string nameModified;                                               // Le agrega un : a los nombres de cada columna, para mejor vista en el frontend
    bool changed  = false;                                                  // Bandera que indica si se cambió de pestaña o de tabla
    std::string modeInput = "regexponly";                                   // Modo de entrada para las barras: "regexponly", "allchars", o "boolean" | Los modos provienen de inputfunc()
    int oldMax = 0, newMax = 0;                                             // Rastrean el nombre de columna más largo para calcular el ancho máximo del texto
    int oldMaxBar = 0, newMaxBar = 0;                                       // Rastrean la barra de entrada más ancha

    if (oldSelected != selected || oldTableSelected != tSelected)                 // La función principal de este if es detectar si se llega a cambiar de pestaña en el panel de administración, o si se llega a cambiar de tabla, si eso ocurre...
    {
        for (int number = 0; number < (int)cVector.size(); number++)              // Se procederá a recorrer cada columna y vaciar su contenido, además de resetear su estado a 0
        {
            if (cVector[number]->type == "tinyint")                               // En el caso especial de que la columna actual solo maneje valores booleanos (1 y 0)...
            {
                cVector[number]->input    = "0";                                  // Se indicará de que de manera predeterminada en input, se introduzca un cero
                cVector[number]->input32  = U"0";                                 // Y lo mismo en input32
            }
            else
            {
                cVector[number]->input   = "";                                    // input tiene que estar vacío ya que no puede almacenar ningun dato de entrada, por que no se ha digitado nada en la columna
                cVector[number]->input32 = U"";                                   // Lo mismo que con input
            }
            cVector[number]->status  = 0;                                         // Y el estado de cada una pasará a 0 (inactiva, no recibe datos de entrada)
            changed = true;
        }
        tabCnt = 0;                                                               // Reinicia a tabCnt, el contador de la función de tabulación para navegar entre las columnas
        opcSelectedPtr->name  = opcionActPtr->name;                               // Resetea a la lista desplegable de la pestaña "Actualizar", esto en caso de que se acceda a esa pestaña
        actBarPtr->input = ""; actBarPtr->input32 = U"";                          // Resetea los valores de la barra de entrada de la pestaña "Actualizar"
        oldSelected      = selected;                                              // La variable que almacenaba el viejo valor de la pestaña actual, se actualiza
        oldTableSelected = tSelected;                                             // La variable que almacenaba el viejo valor de la tabla actual, también se actualiza
        adminButtons[0]->selfquery = "SELECT * FROM "s + tSelected + " WHERE ";   // Se resetea la query de la pestaña "Consultar"
        adminButtons[1]->selfquery = "INSERT INTO "s  + tSelected + " (";         // Se resetea la query de la pestaña "Agregar"
        adminButtons[2]->selfquery = "UPDATE "s       + tSelected + " SET ";      // Se resetea la query de la pestaña "Actualizar"
        adminButtons[3]->selfquery = "DELETE FROM "s  + tSelected + " WHERE ";    // Se resetea la query de la pestaña "Borrar"
    }

    if ((std::string)mode == "default")                                                         // Si el modo es default...
    {
        for (int numberTable = 0; numberTable < (int)cTables.size(); numberTable++)             // Procederá a recorrer todos los botones de las tablas disponibles
        {
            cTables[numberTable]->status = isPressed(cTables[numberTable]);                     // Esto para detectar si el mouse hace clic en alguna tabla
            if (cTables[numberTable]->status == 4) {tSelected = cTables[numberTable]->name;}    // Y si se hace clic sobre alguna, la tabla seleccionada ahora será la que se hizo clic
        }
        drawSelected(cTables, fsize, tSelected);                                                // Después, se procederá a dibujar cada tabla
    }

    for (int number = 0; number < (int)cVector.size(); number++)                                // Este bucle se encarga de dibujar cada columna, verificar sus estados y recibir los datos de entrada la columna seleccionada
    {
        if (cVector[number]->fromTable == tSelected)                                            // Si la columna que está recorriendo el bucle pertenece a la tabla seleccionada...
        {
            cVector[number]->xloc = (screenWidth * 0.12) +                                      // Procederá a declarar los valores de ubicación de cada columna, en esta línea empieza a ubicar sus coordenadas en el eje x
                                    ((int)cVector[number]->name.size() * fsize) -
                                    ((int)cVector[number]->name.size() * (fsize * 0.41)) +
                                    (screenWidth * 0.018);
            newMax = (int)cVector[number]->name.size() * fsize;                                 // Calcula el tamaño del nombre de la columna actual

            cVector[number]->yloc = screenHeight * 0.23 +                                       // Aquí calcula sus coordenadas en el eje y
                                    ((fsize * 2) * quancolumns) +
                                    (screenHeight * 0.02 * quancolumns);

            // Aquí verifica el tipo de dato de entrada y el ancho de la barra

            if      (cVector[number]->type == "varchar") { modeInput = "allchars";   cVector[number]->xsize = fsize * std::stoi(cVector[number]->maxlen); }
            else if (cVector[number]->type == "int")     { modeInput = "regexponly"; cVector[number]->xsize = fsize * std::stoi(cVector[number]->maxlen); }
            else if (cVector[number]->type == "tinyint") { modeInput = "boolean";    cVector[number]->xsize = fsize * 2; }

            newMaxBar = cVector[number]->xsize;                                                 // newMaxBar almacena el tamaño en pixeles de la barra de entrada de la columna actual
            cVector[number]->ysize = fsize * 2;                                                 // Calcula el tamaño de altura de la columna actual

            if ((std::string)mode == "default")                                                 // Si el modo es default...
            {
                nameModified = cVector[number]->name + ":";                                     // Ahora se guarda el nombre de la columna en una variable llamada nameModified junto con unos :, esto para que la columna se vea con : en el frontend
                DrawTextEx(fontTtf, nameModified.data(),                                        // Dibuja ahora el nombre de la columna
                           (Vector2){(float)(screenWidth * 0.12),
                                      (float)(cVector[number]->yloc + (cVector[number]->ysize * 0.5) - (fsize * 0.5))},
                           fsize, 2, NEGRO);
                // Se calcula el hover por separado sin afectar el estado real de escritura
                int tempStatus = isPressed(cVector[number]);                                    // Se obtiene el estado temporal solo para el efecto visual del hover
                if (tempStatus == 1 && cVector[number]->status <= 1)                           // Si hay hover y la columna no estaba activa, se aplica el hover solo visualmente
                    cVector[number]->status = 1;                                               // Se aplica el estado hover sin interrumpir la escritura
                PrettyDrawRectangle(cVector[number]);                                           // Dibuja la barra con el hover ya reflejado
                cVector[number]->status = isPressed(cVector[number]);                           // Ahora sí se actualiza el estado real para la lógica de escritura
                if (cVector[number]->status > 1)                                                // Si se interactúa con la columna, entonces recibirá datos de entrada
                {
                    if (cVector[number]->status == 4) {tabRestart = true;}                      // Si la columna recibe un clic, entonces tabRestart se activará, esto para reiniciar la función de tabulación y avisar que la columna actual cambió
                    cVector[number]->input = inputfunc("backend", cVector[number],              // Se llama a la función inputfunc()) como modo backend para que reciba datos de entrada de la barra de la columna actual
                                                       std::stoi(cVector[number]->maxlen),
                                                       modeInput, fsize, NEGRO);
                    columnSelected = cVector[number]->id;                                       // Nombra como la columna seleccionada a la columna actual
                }
                inputfunc("frontend", cVector[number], 0, modeInput, fsize);                    // Ahora, vuelve a llamar a inputfunc() pero como modo frontend, para que muestre los datos de entrada de la columna actual
            }

            quancolumns++;                                                                      // Hay que recordar que desde la línea 43 estamos en un bucle que recorre todas las columnas, quancolumns almacena la cantidad total de columnas
            if (newMax    > oldMax)    {oldMax    = newMax;}                                    // Si el tamaño del nombre de la columna actual es mayor que al último valor que se obtuvo, oldMax ahora tendrá ese valor, esto sirve para opcionAct
            if (newMaxBar > oldMaxBar) {oldMaxBar = newMaxBar;}                                 // Lo mismo de la línea anterior, ahora con el tamaño de la barra de entrada, esto sirve para ayudar el tamaño de actBar en la pestaña "Actualizar"
            lastColumnMeasures = (int)(cVector[number]->yloc + cVector[number]->ysize + (fsize * 1.2));  // También ayuda en la pestaña "Actualizar", esto sirve para indicar la posición de la última columna dibujada
        }
    }

    maxLenName   = oldMax;                              // Se guardan las medidas máximas globales, acá se guarda el tamaño máximo del nombre
    maxBarLenght = oldMaxBar;                           // Y acá se guarda el tamaño máximo de la barra de entrada

    if (changed)                                                                                    // Si se detectó un cambio de pestaña o de tabla...
    {
        for (int number = 0; number < (int)cVector.size(); number++)                                // Recorrerá todas las columnas
        {
            if (cVector[number]->fromTable == tSelected) { cVector[number]->status = 4; break; }    // Y cuando llegue a encontrar la primera que coincida con la tabla actual, la declarará con estado 3 (activa) y romperá el bucle
        }                                                                                           // Esto sirve para que el administrador ya de una vez tenga activa la primera columna de la tabla actual sin tener que hacer clic
    }
    if (lastColumnMeasures != 0) logfunction(selected, lastColumnMeasures, fsize);                  // Si se dibujó al menos a una columna, se llama a logfunction() enviándole la ubicación sobre donde tiene que iniciar el cuadro de resultados
    return oldSelected;                                                                             // Retorna la vieja pestaña seleccionada, esto sirve para la siguiente vez en la que se tendrá que ejecutar esta función (es 60 veces x segundo)
}

// Esta función a continuación sirve para mostrar un mensaje en la pantalla por determinado periodo de tiempo

int shortmessage(std::string msg, double fs, bool &activator, int timeFps)      // Sus parámetros son el mensaje a mostrar en pantalla, el tamaño del font, la variable que activa el mensaje, y el tiempo en FPS que durará el mensaje
{
    if (framesCounter < timeFps)                                                // Si el tiempo que lleva mostrandose el texto en pantalla es menor al tiempo que se solicita en la llamada de la función, osea, que aún debe mostrar el texto...
    {
        framesCounter++;                                                        // El contador del tiempo sumará un valor
        double w    = msg.length() * fs;                                        // w almacenará el tamaño del mensaje
        double r[4] = {(screenWidth * 0.5) - (w / 2),                           // r almacenará los valores del rectangulo donde se mostrará el mensaje
                       (screenHeight * 0.5) - ((screenHeight * 0.3) / 2),
                       w, screenHeight * 0.3};
        DrawRectangle(r[0], r[1], r[2], r[3], BLANCO);                           // Se procede a dibujar el rectángulo
        DrawRectangleLines(r[0], r[1], r[2], r[3], NEGRO);                      // Se dibujan los bordes del rectángulo
        DrawTextEx(fontTtf, msg.data(),                                         // Y se dibuja el texto del mensaje
                   (Vector2){(float)(r[0] + centertext(msg, r[2], fontSize)),
                              (float)(r[1] + (r[3] - fontSize) / 2)},
                   fontSize, 2, NEGRO);
    }
    else                                                                        // Si el tiempo que lleva mostrandose el texto en pantalla ya fue superado al tiempo solicitado, entonces...
    {
        activator = false;                                                      // El activador del mensaje se desactiva
        framesCounter = 0;                                                      // El contador vuelve a empezar a 0, esto para el caso de que se necesite este contador en otra función o llamarse de nuevo para dar otro mensaje con esta función
    }
    return 0;                                                                   // Retorna 0, código de estado exitoso
}

// Esta función a continuación, es la responsable de mostrar la gráfica de los porcentajes y cantidades de votos por cada partido en la pestaña "Resultados"

std::vector<double> statistics(std::string mode,                          // El modo puede ser "backend" o "frontend"
                               std::string outputMode,              // "percentages" o "quantity"
                               std::vector<double>& dataVec,        // Vector con los datos (votos o porcentajes)
                               std::vector<sqlobject*> partVec,     // Vector con los partidos y sus nombres
                               int posx,                            // Posición X donde dibujar, esto es de solo frontend
                               int posy)                            // Posición Y donde dibujar, esto es de solo frontend
{
    if ((std::string)mode == "backend")                             // Si el modo es "backend"...
    {
        if ((std::string)outputMode == "percentages")               // Y si el modo de salida a mostrar es en porcentajes...
        {
            int total = 0;                                                    // Declara total, el cual es el que acumula la cantidad de votos totales para luego sacar sus porcentajes
            for (int item = 0; item < (int)dataVec.size(); item++)            // Recorre dataVec para ir sumando la cantidad de votos en total
            {
                total += dataVec[item];                                       // Se suman los votos a dataVec
                if (item + 1 == (int)dataVec.size())                          // Si el elemento actual es el último de dataVec, significa que ya tiene el valor total de todos los porcentajes, entonces...
                {
                    for (int n = 0; n < (int)dataVec.size(); n++)             // Realiza otro bucle en dataVec, este para reemplazar los valores de cantidades, a porcentajes, es decir, si el item 1 es igual a 20, lo reemplazo por su porcentaje
                    {
                        if (dataVec[dataVec.size() - 1] <= 0)                             // En caso de que no hayan votos...
                        { dataVec.insert(dataVec.begin(), 0); dataVec.pop_back(); }       // Inserto un cero, para evitar una división por cero, lo cual puede explotar el programa
                        else
                        { dataVec.insert(dataVec.begin(), dataVec[dataVec.size() - 1] * 100.0 / total); dataVec.pop_back(); }       // En caso de que no sea cero, hago la operación para sacar su porcentaje, y lo inserto al vector
                    }
                }
            }
        }
        return dataVec;     // Si el modo de salida NO era "percentages", devuelve el vector como tal sin modificación alguna, ya que el vector se recibe con cantidades, y el otro modo es "quantity", el cual quiere visualizar los datos en cantidades
    }
    else            // Si el modo NO es "backend"...
    {
        float maxPartName  = 0;                                                                 // Para que los partidos queden separados de manera igualitaria, necesita saber cual es el string más largo, esta variable almacena ese valor
        for (int item = 0; item < (int)dataVec.size(); item++)                                  // Este bucle se encarga de recorrer todos los nombres y porcentajes de todos los partidos
        {
            if ((int)partVec[item]->name.length() > maxPartName)                                // Si el nombre del partido en el bucle tiene un largo mayor al máximo definido anteriormente...
                maxPartName = partVec[item]->name.length();                                     // Se guardará el nuevo máximo
            else if ((int)std::to_string(dataVec[item]).length() - 4 > maxPartName)             // Si el largo del porcentaje es mayor que el definido anteriormente...
                maxPartName = std::to_string(dataVec[item]).length();                           // Se guardará el nuevo máximo
        }

        DrawRectangle(posx - screenWidth * 0.02,                                                        // Dibuja el fondo de la gráfica
                      posy - screenHeight * 0.003 * 100 - screenHeight * 0.02,
                      (screenWidth * 0.04 * dataVec.size()) +                                           // Su ancho depende de la cantidad de partidos
                      (maxPartName * ((littleFontSize / 2) * dataVec.size())) + screenWidth * 0.02,
                      screenHeight * 0.003 * 100 + (littleFontSize * 3) + screenHeight * 0.04,          // Su alto cubre como máximo el 30% de la pantalla, sumando el espacio de los nombres y porcentajes
                      BLANCO);                                                                           // Fondo blanco para que contraste con el fondo oscuro de la pantalla
        DrawRectangleLines(posx - screenWidth * 0.02,                                                   // Dibuja el borde dorado alrededor del cuadro de resultados
                           posy - screenHeight * 0.003 * 100 - screenHeight * 0.02,
                           (screenWidth * 0.04 * dataVec.size()) +
                           (maxPartName * ((littleFontSize / 2) * dataVec.size())) + screenWidth * 0.02,
                           screenHeight * 0.003 * 100 + (littleFontSize * 3) + screenHeight * 0.04,
                           DORADO_BORDE);                                                               // Usa el mismo color dorado que todos los botones del programa para ser consistente visualmente

        for (int item = 0; item < (int)dataVec.size(); item++)                                          // Recorre cada partido y dibuja su barra gráfca junto con su nombre y porcentaje
        {
            if ((std::string)outputMode == "percentages")                                               // Si el modo de salida es igual a porcentajes...
            {
                /* La altura de cada barra es screenHeight*0.003 * dataVec[item], como dataVec[item] es un porcentaje (0-100), la altura máxima posible es
                   screenHeight*0.003*100 = 30% de la pantalla, un partido con 50% tendría una barra de altura screenHeight*0.003*50  */

                DrawRectangle(posx + (screenWidth * 0.04 * item) + (maxPartName * (littleFontSize / 2) * item),
                              posy - screenHeight * 0.003 * dataVec[item],
                              screenWidth * 0.04,
                              screenHeight * 0.003 * dataVec[item], VOCADORADO);
            }
            else                                                                                        // Si el modo de salida NO es igual a porcentajes (es quantity) entonces...
            {
                if (dataVec[item] != 0)
                {

                /* Calcula studentsVoted = votos / totalEstudiantes, que da un valor entre 0 y 1 representando la fracción del total, luego la altura es
                   maximumSizeBar * studentsVoted, donde maximumSizeBar = screenHeight*0.3, así una barra nunca supera el 30% de la pantalla  */

                    double studentsVoted  = dataVec[item] / quanstudents;
                    double maximumSizeBar = screenHeight * 0.3;
                    DrawRectangle(posx + (screenWidth * 0.04 * item) + (maxPartName * (littleFontSize / 2) * item),
                                  posy - maximumSizeBar * studentsVoted,
                                  screenWidth * 0.04,
                                  maximumSizeBar * studentsVoted, VOCADORADO);
                }
            }

            std::string outputRounded = "";                                           // outputRounded almacenará el valor que se va a mostrar en la gráfica, almacenará un valor redondeado si el modo es "percentages"
            std::string floatString   = std::to_string(dataVec[item]);                // Almacena un string de float a string, pero to_string() incluye muchos decimales en su traducción, y para que se vea mejor en el frontend, se tiene que redondear
            int count = 10;                                                           // El contador inicia con un recorte para 10 digitos, esto no se usa si el número no tiene decimales
            for (int digit = 0; digit < (int)floatString.size(); digit++)             // Ahora, recorrerá todos los números en el string del float traducido
            {
                if (count == 0) break;                                                // Si el contador llega a cero, el bucle rompe, y lo que se haya guardado en outputRounded se mostrará en el frontend
                if (floatString[digit] == '.')                                        // Si en el string se encuentra un punto...
                {
                    if ((std::string)outputMode == "percentages") count = 3;          // El contador será igual a 3, es decir, outputRounded almancenará 3 valores más, los cuales serían el mismo punto que encontró actualmente, y 2 decimales
                    else break;                                                       // En caso de que el modo no sea igual a "percentages", solo romperá el bucle, ya que significa que solo se pide en la gráfica el valor entero, no decimal
                }
                outputRounded += floatString[digit];                                  // outputRounded va guardando los valres en el bucle
                count--;                                                              // count va restando, si count llega a 0, el bucle termina
            }

            DrawTextEx(fontTtf, outputRounded.data(),                                 // Se dibuja el porcentaje
                       (Vector2){(float)(posx + screenWidth * 0.04 * item + maxPartName * (littleFontSize / 2) * item),
                                  (float)(posy + littleFontSize)},
                       littleFontSize, 2, NEGRO);
            DrawTextEx(fontTtf, partVec[item]->name.data(),                           // Se dibuja el nombre del partido del bucle actual, ya que hay que recordar que estamos en un bucle desde la línea 48
                       (Vector2){(float)(posx + screenWidth * 0.04 * item + maxPartName * (littleFontSize / 2) * item),
                                  (float)(posy + littleFontSize * 2)},
                       littleFontSize, 2, NEGRO);
        }

        while (!dataVec.empty()) dataVec.pop_back();                                  // Cuando este bucle termine, se vaciará el vector de los datos de los votos por completo, ya que se tiene que volver a llenar cuando se invoque
        return dataVec;                                                               // Retorna el vector
    }
}

bool alert(std::string bontonActual)
{
    return true;
}

int transition(std::string mode)
{
    alphaIsZero = true;
    alphaIsFull = true;
    if (mode == "hide")
    {
        alphaIsFull = false;
        if (GRIS.a > 0)
            GRIS.a = GRIS.a - 15;
        EndDrawing();         // Reiniciar el dibujado terminando
        BeginDrawing();       // y volviendolo a iniciar, para que el frontend se actualice a los cambios de los colores
        if (darkMode)                                         // Si el modo oscuro está activo...
            ClearBackground({45, 45, 48, 255});               // Limpia el background y lo "pinta" de un color oscuro
        else                                                  // Si el modo oscuro NO está activo
            ClearBackground({250, 244, 228, 255});            // Limpia el background y lo "pinta" de un color claro
    }
    for (int color = 0; color < (int)size(colorsVec); color++)
    {
        if (mode == "hide")
        {
            if ((int)colorsVec[color]->a != 0)
            {
                colorsVec[color]->a = (int)colorsVec[color]->a - 3;
                alphaIsZero = false;
            }
        }
        else if (mode == "show")
        {
            if ((int)colorsVec[color]->a != 255)
            {
                colorsVec[color]->a = (int)colorsVec[color]->a + 3;
                alphaIsFull = false;
            }
        }

    }
    if (mode == "show")
    {
        alphaIsZero = false;
        if (GRIS.a < 255 && alphaIsFull)
        {
            GRIS.a = GRIS.a + 15;
        }
    }
    if (alphaIsZero) return -1;
    else if (alphaIsFull) return 1;
    return 0;
}
