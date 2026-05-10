#pragma once
#include "../globals.hpp"
#include "../platform/encoding.hpp"
#include "../platform/clipboard.hpp"

// Forward declaration — body is in drawing.hpp
template<typename T> void PrettyDrawRectangle(T obj);

// ── isPressed ─────────────────────────────────────────────────────────────────
/* isPressed() verifica el estado del objeto que se le pasa como argumento, actualmente hay 4 estados, pero me gustaría posteriormente agregar un estado más:

    0: No presionado, y tampoco hay interacción alguna
    4: Presionado en este instante
    3: Fue presionado anteriormente, y el cursor está encima del objeto
    2: Fue presionado anteriormente, y el cursor NO está encima del objeto
    1: No fue presionado anteriormente, pero el cursor está encima del objeto

*/

inline int isPressed(auto& obj)
{
    float xloc = obj->xloc, yloc = obj->yloc, xsize = obj->xsize, ysize = obj->ysize;
    int laststate = obj->status;

    // Se usa GetMouseX/Y para PC y GetTouchX/Y para pantallas táctiles
    // En PC GetTouchX/Y siempre devuelve 0 cuando no hay toque, por eso se necesitan ambos
    float cursorX = (GetTouchX() > 0 || GetTouchY() > 0) ? GetTouchX() : GetMouseX();   // Si hay toque táctil usa Touch, sino usa Mouse
    float cursorY = (GetTouchX() > 0 || GetTouchY() > 0) ? GetTouchY() : GetMouseY();   // Lo mismo para el eje Y

    bool inside = (cursorX >= xloc && cursorX <= xloc + xsize &&                         // Verifica si el cursor está dentro del ancho del objeto
                   cursorY >= yloc && cursorY <= yloc + ysize);                          // Y dentro del alto del objeto

    if ((IsGestureDetected(GESTURE_TAP) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && inside)   // Si hubo clic (táctil o mouse) dentro del objeto
        return 4;                                                                                  // Retorna 4: clic en este instante

    if ((IsGestureDetected(GESTURE_TAP) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && !inside)  // Si hubo clic fuera del objeto
        return 0;                                                                                  // Retorna 0: sin interacción

    if (inside)                                                                           // Sin clic pero cursor encima
        return (laststate == 0 || laststate == 1) ? 1 : 3;                               // 1 si nunca fue presionado, 3 si ya fue presionado antes

    return (laststate == 0 || laststate == 1) ? 0 : 2;                                   // Cursor fuera: 0 si nunca presionado, 2 si ya fue presionado

    return 0;
}

// ── GetCursorFromMouseClick ───────────────────────────────────────────────────
// Desplaza al cursor de las barras de entrada de datos inputBar, hacia donde apunte el cursor del mouse
inline int GetCursorFromMouseClick(auto* bar, int fsize)              // Necesita como argumentos la barra que se detecta el clic, y el tamaño del font de la barra actual
{
    float mouseX     = GetMouseX();                                   // Busca la posición del mouse en el eje X (ancho de la pantalla)
    float textStartX = bar->xloc * 1.04;                              // Busca la posición donde inicia a mostrarse el texto en el frontend, que es usualmente al 4% del largo de la barra

    if (mouseX <= textStartX) return 0;                               // Si la posición del mouse es menor o igual a la posición mínima en la que se encuentra la barra, entonces solo retorna 0, indicando que el cursor NO está en la barra

    for (int i = 0; i <= (int)bar->input32.size(); i++)               // Ahora, recorrerá cada carácter en la  barra de entrada
    {
        std::u32string sub(bar->input32.begin(), bar->input32.begin() + i);     // sub ahora almacenará todos los carácteres desde el inicio del string hasta el carácter que tenga el índice del contador, esto para ir sumandolos para después
        std::string sub8 = UTF32ToUTF8(sub);                                    // Se procede a traducir sub a UTF-8 y se almacena dentro de sub8
        Vector2 width = MeasureTextEx(fontTtf, sub8.c_str(), fsize, 0);         // Se calcula width, el tamaño acerca de la posición que toma el cursor de la barra de datos 
        if (mouseX < textStartX + width.x) return i;                            // Se compara acerca de que si la posición del mouse, sobre la barra de datos de entrada, coincide en su mayoría con la posición calculada, de ser así, se retorna
    }
    return bar->input32.size();               // En caso de que esto no ocurra, el cursor del mouse se encuentra en la posición máxima de la barra, entonces devuelve la posición máxima
}

// ── inputfunc ─────────────────────────────────────────────────────────────────
/* Esta función se encarga de recibir los datos de entrada de los objetos inputBar, además del movimiento en la barra y el pegado (copiado no se ha implementado, y no pienso hacerlo la verdad), además del uso de las codificaciones UTF-32 y UTF-8
   Se encarga de traducir los datos de entrada a UTF-32 a UTF-8 por que lamentablemente Raylib, la librería gráfica, es un dolor de huevos para mostrar carácteres en UTF-32, lo cual habría facilitado todo, además de que funciones como
   desplazarse entre los carácteres por medio de las flechas del teclado, usar el botón DELETE y el botón BACKSPACE y diversas funciones más, se dificultarían más con el uso de UTF-8 internamente, ya que carácteres como la Ñ usan 2 bits,
   no como los carácteres del abecedario ingles como la A, que ocupa solo un bit, entonces tendría que identificar cuándo debe el cursor desplazarse 2 bits en el string, y cuando borrar dos bits, etc, así que decidí usar UTF-32, en el que
   todos los carácteres usan 4 bits, y es mucho mejor de manejar, así que a esto se debe la traducción entre UTF-8 a UTF-32 y viceversa, y también a esto se debe el uso de input32 e input como propiedades de las inputBar  */

inline std::string inputfunc(std::string mode,                            // Verifica cual es el modo, si "backend" o "frontend"
                             auto& bar,                                   // Es el objeto de la barra la cual recibirá datos de entrada o solo mostrará los datos actuales con "frontend"
                             int maxlen = 21,                             // Es el tamaño máximo aceptado de los datos de entrada, por defecto suele admitir únicamente 21 carácteres
                             std::string inputmode = "allchars",          // Verifica el modo de datos de entrada: 
                                                                          /* boolean:         Solo admite unos y ceros
                                                                             allchars:        Admite letras y números, excepto comillas simples
                                                                             numberonly:      Admite solo números
                                                                             regexponly:      Admite números y símbolos
                                                                             allchars-admin:  Admite absolutamente todo tipo de carácteres en el abecedario latín */
                             double fsize = 0,                            // Es el tamaño del font de la letra a mostrar en el frontend
                             Color col = NEGRO,                           // Es el color de la letra a mostrar en el frontend
                             int maxBackspace = 7,                        // Es un limitador para que la tecla BACKSPACE y DELETE no borren todo el contenido inmediatamente cuando se presionen
                             int maxBeam = 120,                           // Es un limitador para verificar cuanto tiempo se debe mostrar la barra del cursor y cuanto tiempo debe ocultarse, para dar el efecto de parpadeo
                             int maxStopArrows = 8)                       // Es parecido al limitador de la tecla BACKSPACE y DELETE, solo que con las flechas de derecha e izquierda, sino se desplazaría inmediatamente hasta el otro extremo del texto
{
    if (mode == "backend")            // Si el modo es "backend"
    {
        if (inputmode != "boolean")                     // Y el modo NO es boolean
        {
            if ((inputpos > (int)bar->input32.size()) || inputpos < 0)          // inputpos es la posición de la barra parpadeante en la que se encuentra el usuario | En caso de que su posición en el texto
                inputpos = bar->input32.size();                                 // Sea mayor al largo del texto o tiene un valor negativo, se declarará al final del texto

            bool pasted = false;                            // pasted verifica si se pegó texto con CTRL+V, se declara false al inicio de la función
            std::u32string clipdata = U"";                  // Almacenará el texto que se reciba del portapapeles en caso de que se pegue algo

            if (oldbar != bar->name)                        // oldbar almacena el nombre de la barra de datos de entrada en la que se ejecutó inputfunc() la última vez, si llega a cambiar significa que se cambió de barra, entonces...
            {
                oldbar = bar->name;                         // Se actualiza el nombre de la nueva barra actual
                inputpos = bar->input32.size();             // La posición de la barra de escritura se reinicia al tamaño del string que almacene la barra actual
                beam = 0;                                   // El contador del parpadeo se reinicia a cero
            }
            else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_V) && // En caso de que se presione la tecla CTRL y la tecla V (significa que se desea pegar algo) y...
                     !stopCtrls && (int)bar->input.length() < maxlen)   // stopCtrls es falso (stopCtrls sirve para que haya un límite de veces de pegado por segundo, si se encuentra falso, admite pegado) y el tamaño del input no supera el máximo
            {
                clipdata  = clipboard();          // clipdata almacenará lo que devuelva la función clipboard() la cual se encarga de recibir los datos del portapapeles
                pasted    = true;                 // pasted se declarará como verdadero, esto verifica de que se acaba de pegar un texto
                stopCtrls = 1;                    // Declaro a stopCtrls 1 para que no admita pegados de portapapeles hasta que su contador lo reinicie
            }

            int codepoint = GetCharPressed();                     // GetCharPressed() es una función que recibe los carácteres del teclado, codepoint almacena el valor que reciba GetCharPressed()
            while (codepoint > 0 || pasted)                       // Mientras codepoint sea mayor a cero (es decir, que se llegó a digitar algo) o se llegó a pegar algo, entonces...
            {
                if (pasted)                                       // Si se pegó algo, se procederá a verificar que lo que se haya pegado esté entre los carácteres admitidos en la entraad
                {
                    codepoint = (int)clipdata[0];                 // codepoint almacenará el primer carácter del texto almacenado en clipdata, que es el texto proveniente del portapapeles
                    clipdata.erase(0, 1);                         // Como codepoint ya guardó el primer carácter, entonces procede a eliminarlo
                    if (clipdata.empty()) pasted = false;         // En caso de que clipdata se encuentre vacío, declarará a pasted falso para que ya finalice el bucle después de este procedimiento
                }
                if ((int)bar->input32.size() < maxlen)            // Si el tamaño máximo de carácteres posibles en la barra actual NO ha sido superado, entonces procederá a verificar si el carácter de codepoint es válido
                {

                    // Para esos if, recomiendo leer ASCII.txt

                    if ((inputmode == "numberonly" && (codepoint > 47 && codepoint < 58)) ||  // Si el modo de datos de entrada es "numberonly", verificará que el valor del codepoint se encuentre mayor a 47 y menor al 58 (solo números), o...
                        (inputmode == "regexponly" && codepoint != 39 &&                      // si el modo de datos de entrada es "regexponly" verificará que el valor del codepoint se encuentre entre los valores de números y símbolos, o...
                         ((codepoint > 32 && codepoint < 65) ||
                          (codepoint > 90 && codepoint < 96) ||
                          (codepoint > 122 && codepoint < 127))) ||
                        (((inputmode == "allchars" && codepoint != 39) ||       // si el modo de datos de entrada es "allchars", verificará que el valor del codepoint se encuentre entre los carácteres admitidos (todos excepto '), o...
                          inputmode == "allchars-admin") &&                     // si el modo de datos de entrada es "allchars-admin" admitirá que el valor de codepoint sea cualquiera de todos los carácteres válidos del latín
                         ((codepoint > 160 && codepoint < 257) ||
                          (codepoint > 31  && codepoint < 128))))
                    {
                        bar->input32.insert(bar->input32.begin() + inputpos, (char32_t)codepoint);  // Si el if anterior admite a codepoint dependiendo de su modo, entonces lo introducirá en el string según la posición de inputpos
                        inputpos++;                                                                 // Y la posición de la barra del cursor, aumentará un espacio, ya que se acaba de sumar un nuevo carácter al string
                    }
                }
                codepoint = GetCharPressed();
            }

            // Utilidades del teclado, tecla flecha  izq. y der., backspace y delete

            if ((!IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_CONTROL)) &&        // Verifica si se ha soltado alguno de los botones para realizar el CTRL+V, en caso de que sea así, stopCtrls se declara a 0
                !IsKeyDown(KEY_V) && stopCtrls)
                stopCtrls = 0;

            if (IsKeyDown(KEY_UP) && stopArrows > maxStopArrows &&                      // Verifica si se ha presionado la flecha arriba, y si se encuentra en la pestaña "Terminal" del panel de administración
                logpos < (int)logCommands.size() && inputmode == "allchars-admin")
            {
                stopArrows = 0; beam = 0; logpos++;                                     // Reinicia los valores de stopArrows, para que no suba demasiadas veces hacia arriba en un segundo, también el parpadeo de la barra lo reinicia y logpos aumenta
                bar->input32 = UTF8ToUTF32(logCommands[logCommands.size() - logpos]);   // logpos es la variable que permite acceder a un índice de logCommands, logCommands es el historial de los comandos ingresados en la pestaña "Terminal"
                bar->input   = UTF32ToUTF8(bar->input32);                               // Cuando se presiona la tecla arriba, se logra acceder al comando más reciente del historial, y si se presiona más veces sube hasta el más viejo
                inputpos     = bar->input32.size();                                     // Se declara en input y en input32 el comando ingresado, y la posición de la barra parpadeante se actualiza al tamaño del comando
            }

            if (IsKeyDown(KEY_DOWN) && stopArrows > maxStopArrows &&                    // Verifica si se ha presionado la flecha abajo, es lo mismo del if anterior, solo que lo que hace es bajar al comando más nuevo
                logpos > 0 && inputmode == "allchars-admin")                            // en vez de subir en el historial de comandos al comando más viejo
            {
                stopArrows = 0; beam = 0; logpos--;
                if (logpos != 0)                                                        // Verifica que logpos no sea cero
                {
                    bar->input32 = UTF8ToUTF32(logCommands[logCommands.size() - logpos]);
                    bar->input   = UTF32ToUTF8(bar->input32);
                    inputpos     = bar->input32.size();
                }
                else { bar->input = ""; bar->input32 = U""; }                         // En caso de que logpos sea cero, solo declara un string vacío como el comando actual
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && bar->status == 4)          // Si se hace clic en alguna parte de adentro de la barra de datos de entrada, buscará donde posicionar la barra parpaeante
            { inputpos = GetCursorFromMouseClick(bar, fsize); beam = 0; }             // Se obtiene la posición por medio de GetCursorFromMouseClick() y el parpadeo de la barra se reinicia a 0

            if (IsKeyDown(269) || IsKeyDown(321)) { inputpos = (int)bar->input32.size(); beam = 0; }          // Tecla END, envía la barra parpaeante hasta el último carácter de la barra
            if (IsKeyDown(268) || IsKeyDown(327)) { inputpos = 0; beam = 0; }                                 // Tecla HOME, envía la barra parpadeante hasta el primer carácter de la barra

            if (IsKeyDown(KEY_LEFT)      && inputpos > 0                          && stopArrows    > maxStopArrows)  { inputpos--;                                                     stopArrows    = 0; beam = 0; }       // Tecla IZQ.
            if (IsKeyDown(KEY_RIGHT)     && inputpos < (int)bar->input32.size()   && stopArrows    > maxStopArrows)  { inputpos++;                                                     stopArrows    = 0; beam = 0; }       // Tecla DER.
            if (IsKeyDown(KEY_BACKSPACE) && inputpos > 0                          && stopBackspace > maxBackspace)   { bar->input32.erase(bar->input32.begin() + inputpos - 1); inputpos--; stopBackspace = 0; beam = 0; }  // Tecla BACKSPACE
            if (IsKeyDown(KEY_DELETE)    && inputpos < (int)bar->input32.size()   && stopBackspace > maxBackspace)   { bar->input32.erase(bar->input32.begin() + inputpos);             stopBackspace = 0; beam = 0; }      // Tecla DELETE

            bar->input = UTF32ToUTF8(bar->input32);                                 // Convierte los datos que se manejaron en input32 internamente como UTF-32 a UTF-8 a input para mostrarse en el frontend

            beam++;                                                 // Aumento del contador para el parpadeo de la barra parpadeante
            if (beam <= maxBeam / 2)  showBeam = true;              // Si el parpadeo es menor a la mitad del máximo del parpadeo, se mostrará la barra
            else if (beam > maxBeam)  beam = 0;                     // Si el contador del parpadeo es mayor al límite, el contador del parpadeo se reinicia a 0
            else                      showBeam = false;             // En caso de que el parpadeo sea mayor a la mitad del máximo del parpadeo pero no mayor al límite, solo no mostrará la barra parpadeante
            if (stopBackspace <= maxBackspace) stopBackspace++;     // Contador de los frames de stopBackspace para que no borre todo el texto inmediatamente al presionar la tecla BACKSPACE o DELETE
            if (stopArrows    <= maxStopArrows) stopArrows++;       // Contador de los frames de stopArrows, para que no se vaya a la izquierda o derecha del texto inmediatamente al presionar la flecha DER. p IZQ.
        }
        else                          // Si el modo es "boolean"
        {
            int codepoint = GetCharPressed();                                               // Declara a codepoint, que almacenará cualquier carácter presionado
            if (codepoint == 48 || codepoint == 49) {bar->input32 = (char32_t)codepoint; bar->input = (char)codepoint;}     // Si el carácter presionado es un uno o un cero, entonces el valor de la barra de entrada será el carácter digitado
            showBeam    = false;                                                            // No se necesita que haya una barra parpadeante, entonces showBeam se desactiva
            if (bar->status == 4)                                                           // Si se recibe un clic en la barra...
            {
                if (bar->input == "0") {bar->input = "1"; bar->input32 = U"1";}             // En caso de que el valor anterior de la barra haya sido 0, lo cambiará a 1 por el clic, tanto en las propiedad input como en input32
                else                   {bar->input = "0"; bar->input32 = U"0";}             // Ocurre lo mismo que en la línea anterior, solo que al revés (si más bien era uno, ahora será cero)
            }
        }
    }

    if (mode == "frontend")             // Si el modo es "frontend"...
    {
        Font fttf;
        if (bar->name != barAdminTerminalPtr->name) {PrettyDrawRectangle(bar); fttf = fontTtf;}         // Si el nombre de la barra NO es la de barAdminTerminalPtr, dibujará a la barra con la función PrettyDrawRectangle()
        else fttf = monoTtf;                                                                            // En caso de que sí lo sea, el font será igual a monoTtf, para usar el font con monoespaciado

        if (inputmode != "boolean") {                           // Si el modo NO es "boolean"...
            DrawTextEx(fttf, bar->input.c_str(),             // Dibujará el texto introducido en la barra
                       (Vector2){(float)(bar->xloc * 1.04),
                                  (float)(((bar->ysize - fsize) / 2 + bar->yloc) - (bar->ysize * 0.05))},
                       fsize, 0, col);

            if (bar->status > 1 && showBeam && inputpos <= (int)bar->input32.size())    // Y buscará donde poner la barra parpadeante, siempre en caso de que showBeam sea verdadero
            {
                // Se mide el ancho exacto del texto hasta la posición del cursor con MeasureTextEx
                // para que la barra parpadeante quede exactamente donde corresponde sin correrse
                std::string textUpToCursor = UTF32ToUTF8(bar->input32.substr(0, inputpos));
                Vector2 measured = MeasureTextEx(fttf, textUpToCursor.data(), fsize, 0);
                DrawLine(
                    (int)((bar->xloc * 1.04) + measured.x),
                    (int)(bar->yloc + ((bar->ysize - fsize) / 4)),
                    (int)((bar->xloc * 1.04) + measured.x),
                    (int)(bar->yloc + fsize * 1.6),
                    bar->name != barAdminTerminalPtr->name ? NEGRO : BLANCO);            // En caso de que la barra de datos de entrada sea barAdminTerminalPtr, la barra parpadeante en vez de dibujarse negra, se dibujará blanca
            }
        }
        else          // Si el modo SÍ es "boolean"...
        {
            DrawTextEx(fontTtf, bar->input.c_str(),     // Dibujará el texto en el centro del cuadro, ya que es un cuadro en vez de una barra cuando el valor de datos de entrada es "boolean"
                       (Vector2){(float)(((bar->xsize - fsize) / 2 + bar->xloc) + (fsize * 0.3)),
                                  (float)(((bar->ysize - fsize) / 2 + bar->yloc) - (fsize * 0.05))},
                       fsize, 0, col);
        }
    }

    return bar->input;  // Al final de la función, retornará el texto digitado en la barra de datos de entrada
}
