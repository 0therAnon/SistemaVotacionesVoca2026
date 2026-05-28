#include <chrono>
#include <thread>
#include "../globals.hpp"
#include "screen_mainmenu.hpp"
#include "../db/database.hpp"
#include "../ui/input.hpp"
#include "../ui/drawing.hpp"

// Esta función se encarga del backend de la pantalla MAINMENU

void screenMainmenuUpdate(Screen& currentScreen,          // Necesita la variable currentScreen para modificar la pantalla actual
                          bool& existstudent,             // Neceista modificar la variable existstudent para indicar si el estudiante existe o no
                          std::string& studentName)       // Necesita a la variable studentName para ingresar el nombre del estudiante si se encontró en la base de datos
{
    cedulaBarPtr->status = isPressed(cedulaBarPtr);                       // Se verifica el estado de la barra de datos de entrada de la cedula
    if (cedulaBarPtr->status > 1)                                         // Si el estado de la barra es mayor a 1, significa que se quieren recibir datos de entrada, entonces...
        inputfunc("backend", cedulaBarPtr, 21, "allchars", fontSize);     // Llama a inputfunc() para recibir datos de entrada

    cedulaPtr->status = isPressed(cedulaPtr);                             // Se verifica el estado del botón cedula
    if ((IsKeyPressed(KEY_ENTER) || cedulaPtr->status == 4) &&            // Si se presiona ENTER o se presiona el botón cedula, el cual se encuentra debajo de la barra de datos de entrada cedulaBar, y...
        cedulaBarPtr->input == *admPassword)                              // el valor digitado en la barra de datos de entrada cedulaBar es igual a la contraseña del panel de ADMINMENU, entonces...
    {
        cedulaPtr->status = 0;                            // El estado del botón cedula ahora será igual a cero
        cedulaBarPtr->status = 0;                         // El estado de cedulaBar ahora será igual a cero para no recibir más datos de entrada
        cedulaBarPtr->input  = "";                        // Y se vaciará la input
        cedulaBarPtr->input32  = U"";                     // Y se vaciará la input
        currentScreen = ADMINMENU;                        // La pantalla actual ahora será la del panel de administración
        adminSelected = adminButtons[0]->name;            // La pestaña seleccionada será la primera en el vector adminButtons ("Consultar") para aparecer en esta pestaña automáticamente si se vuelve a acceder
        return;       // Retorno de la función, no se necesita hacer nada más
    }

    else if ((IsKeyPressed(KEY_ENTER) || cedulaPtr->status == 4) &&            // Si se presiona ENTER o se presiona el botón cedula, el cual se encuentra debajo de la barra de datos de entrada cedulaBar, y...
        !cedulaBarPtr->input.empty())                                          // el valor digitado NO se encuentra vacío
    {
        cedulaPtr->status = 0;                                    // El estado del botón cedula ahora será igual a cero
        cedulaBarPtr->status = 0;                                 // El estado de cedulaBar ahora será igual a cero para no recibir más datos de entrada
        cedulaPtr->selfquery += cedulaBarPtr->input;              // Se procede a armar la query con el contenido de la barra cedulaBar para verificar si el estudiante existe en la base de datos
        sendquery((cedulaPtr->selfquery + "'"s).data(), 1, 2);    // Se envía la query, recortando para que la output solo sea después de la primera columna, y antes de la última columna (por esos los "1,1" como argumentos en la función)

        if (!outQuery.empty())                            // Si outQuery (la respuesta de la query) NO se encuentra vacía, entonces...
        {
            existstudent = true;                          // El estudiante SÍ existe
            studentName  = outQuery;                      // Y el resultado de la query se guardará en studentName
        }
        else                                              // En caso de que la respuesta de la query SÍ se encuentre vacía, entonces...
        {
            existstudent = false;                         // Se declara que el estudiante NO existe
            regresarPtr->xloc = cedulaPtr->xloc;          // Y el botón regresar se pondrá en el centro de la pantalla
        }
        cedulaBarPtr->status = 0;                         // La barra cedulaBar no recibirá más datos de entrada y...
        currentScreen = CONFIRMATION;                     // Se cambiará a la pantalla CONFIRMATION
    }
}

// Frontend de MAINMENU

void screenMainmenuDraw(Screen& currentScreen)
{
    if (currentScreen == MAINMENU) transition("show");
    showTime(screenWidth*0.01, screenHeight*0.02);
    DrawTexture(vocaBannerTexture, (screenWidth*0.86f), (screenHeight*0.02f), BLANCO);
    DrawTexture(vocaBackgroundTexture, (screenWidth/2.0f) - (vocaBackgroundTexture.width / 0.75f),
                     (screenHeight/2.0f) - (vocaBackgroundTexture.height / 2.0f), BLANCO);
    DrawTextEx(fontTtf, "Digite su cédula en este campo"s.data(),                                                       // Se muestra un mensaje diciendo de que por favor digite la cédula en la barra
               (Vector2){(float)centertext("Digite su cedula en este campo"s, screenWidth, fontSize),
                          (float)(screenHeight * 0.23)},
               fontSize, 2, COLORTEXTO);
    DrawTextEx(fontTtf, "con uno de los siguientes formatos"s.data(),                                                   // Se agrega en la pantalla el mensaje "con los siguientes formatos"
               (Vector2){(float)centertext("con uno de los siguientes formatos"s, screenWidth, fontSize),
                          (float)(screenHeight * 0.3)},
               fontSize, 2, COLORTEXTO);
    DrawTextEx(fontTtf, "101110111 | 123456789012 | AZ0000-11111"s.data(),                                              // Y se agregan a la pantalla los tipos de formatos a ingresar
               (Vector2){(float)centertext("101110111 | 123456789012 | AZ0000-11111"s, screenWidth, fontSize),
                          (float)(screenHeight * 0.37)},
               fontSize, 2, COLORTEXTO);
    PrettyDrawRectangle(cedulaBarPtr);                                                                                  // Se dibuja el rectángulo de la barra de datos de entrada de la cédula
    PrettyDrawRectangle(cedulaPtr);                                                                                     // Y también se dibuja el botón de la cédula
    DrawTextEx(fontTtf, cedulaPtr->name.data(),
               (Vector2){cedulaPtr->xloc + (float)centertext(cedulaPtr->name, cedulaPtr->xsize, fontSize),
                          cedulaPtr->yloc + (float)((cedulaPtr->ysize - fontSize) / 2)},
               fontSize, 2, NEGRO);
    inputfunc("frontend", cedulaBarPtr, 0, "allchars", fontSize);                                                       // Se procede a dibujar el contenido de la barra de la cédula
    if (currentScreen != MAINMENU && alphaIsZero == false)
    {
        transition("hide");
        screenMainmenuDraw(currentScreen);
    }
}
