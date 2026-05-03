#include "../globals.hpp"
#include "screen_confirmation.hpp"
#include "../db/database.hpp"
#include "../platform/pty.hpp"
#include "../ui/input.hpp"
#include "../ui/drawing.hpp"

// Esta función se encarga del backend de la pantalla CONFIRMATION

void screenConfirmationUpdate(Screen& currentScreen,      // Necesita la variable de la pantalla actual
                              bool& existstudent,         // Necesita la variable existstudent para conocer su valor mostrar si el estudiante existe o no
                              bool& correctstudent,       // Necesita la variable correctstudent para modificar su valor y confirmar si el estudiante es el correcto o no
                              int& verifyvote)            // Necesita la variable verifyvote para modificar su valor y confirmar si el partido seleccionado es el correcto o no
{
    // Se actualiza el estado de los botones cada frame para detectar hover correctamente
    continuarPtr->status = isPressed(continuarPtr);    // Actualiza el estado de continuar en cada frame para detectar cuando el cursor pasa por encima
    regresarPtr->status  = isPressed(regresarPtr);     // Actualiza el estado de regresar en cada frame para detectar cuando el cursor pasa por encima
    if (!existstudent)        // Si el estudiante NO existe en la base de datos...
    {
        if (regresarPtr->status == 4)      // Verificará si se presiona el botón regresar, si el estado del botón llega a ser 4, es decir, llega a recibir un clic
        {
            cedulaBarPtr->status = 4;         // El estado de la barra de datos de entrada que recibe la cédula, se activa al nombrar su estado igual a 4
            cedulaPtr->selfquery  = "SELECT * FROM Estudiantes WHERE Cedula = '";       // Se reinicia la query del botón cedula
            regresarPtr->xloc     = cedulaPtr->xloc * 0.7;                              // El valor del botón regresar se reinicia a su valor original
            continuarPtr->status = 0;                                                   // Se resetea el estado de continuar para que no aparezca resaltado al volver a MAINMENU
            regresarPtr->status  = 0;                                                   // Se resetea el estado de regresar también por la misma razón
            currentScreen = MAINMENU;                                                   // Y ahora la pantalla actual se modifica a MAINMENU, para que el siguiente frame la pantalla sea la del menú principal
        }
        return;           // Retorno de la función, no se necesita hacer nada más
    }
    else                      // Si el estudiante SÍ existe en la base de datos...
    {
        if (regresarPtr->status == 4)          // Si se presiona el botón regresar...
        {
            continuarPtr->status = 0;                 // Se resetea el estado de continuar para que no aparezca resaltado al volver a MAINMENU
            regresarPtr->status  = 0;                 // Se resetea el estado de regresar también por la misma razón
            if (!correctstudent) {                    // Se verificará de que el estudiante NO se haya confirmado, si el estudiante NO se ha confirmado significará que anteriormente estaba en la pantalla MAINMENU, entonces...
                cedulaBarPtr->status = 4;             // La barra de la cedula empezará a recibir datos de entrada
                cedulaPtr->selfquery  = "SELECT * FROM Estudiantes WHERE Cedula = '";       // Se reinicia la query del botón cedula
                currentScreen = MAINMENU;             // La pantalla actual ahora se modifica a MAINMENU, para que en el siguiente frame la pantalla sea la del menú principal
            }
            else
            {
                votarPtr->status = 0;          // Se resetea el estado de votar para que no aparezca presionado al volver a VOTATION
                partidoSelected = "";
                currentScreen = VOTATION;      // En caso de que el estudiante SÍ se haya confirmado, significa que anteriormente estaba en la pantalla VOTATION, entonces únicamente se devolverá a esa pantalla
            }
            return;       // Retorno de la función, no se necesita hacer nada más
        }

        if (continuarPtr->status == 4)         // Si se presiona el botón continuar...
        {
            if (!correctstudent)                      // Se verificará de que el estudiante NO se haya confirmado, si el estudiante NO se ha confirmado significará que anteriormente estaba en la pantalla MAINMENU, entonces...
            {
                correctstudent = true;                // Declara a correctstudent como verdadero, es decir, confirma de que sí es el estudiante correcto
                continuarPtr->selfquery = cedulaPtr->selfquery + "' && Voto = '0'"s;    // Se arma la query para verificar si el estudiante ya ha votado
                sendquery(continuarPtr->selfquery.data(), 0, 0);                        // y se envía
                verifyvote = outQuery.length();                                         // Se almacena el tamaño de la respuesta de la base de datos respecto a la query enviada anteriormente
                continuarPtr->status = 0;                                               // Se resetea el estado de continuar para que no aparezca resaltado al cambiar de pantalla
                regresarPtr->status  = 0;                                               // Se resetea el estado de regresar también por la misma razón
                currentScreen = verifyvote > 1 ? VOTATION : ENDING;                     // Si el tamaño de la respuesta es mayor a 1, significa que NO ha votado, y envía al estudiante a la pantalla VOTATION, sino, a ENDING diciendole que ya votó
            }
            else           // En caso de que el estudiante SÍ se haya confirmado, significa que anteriormente estaba en la pantalla VOTATION, así que...
            {
                votarPtr->selfquery += "1' WHERE Cedula = '"s + cedulaBarPtr->input + "'";                // Arma la query para asignar que el estudiante ha votado
                sendquery(votarPtr->selfquery.data(), 0, 0, 0);                                           // Envía la query armada a la base de datos
                votarPtr->selfquery = "UPDATE Partidos SET Votos = Votos + 1 WHERE Nombre = '"s +         // Arma una query para sumarle 1 a la cantidad de votos del partido por el que el estudiante haya votado
                                                                         partidoSelected + "';"s;
                sendquery(votarPtr->selfquery.data(), 0, 0, 0);                                           // Envía la query a la base de datos
                votarPtr->selfquery = "UPDATE Estudiantes SET Laboratorio = '"s + *labName +              // Arma la query para identificar el laboratorio donde se ejerció el voto
                                                "' WHERE Cedula = '"s + cedulaBarPtr->input +"'"s;        // en el registro igual a la cédula del votante digitado en cedulaBar
                continuarPtr->status = 0;      // Se resetea el estado de continuar para que no aparezca resaltado al cambiar de pantalla
                regresarPtr->status  = 0;      // Se resetea el estado de regresar también por la misma razón
                sendquery(votarPtr->selfquery.data(), 0, 0, 0);                                           // Envía la query a la base de datos
                ptyfunc("backup", *user, *password, *server, *port, *database);                           // Ejecuta un backup de la base de datos de forma automática después del voto
                currentScreen = ENDING;                                                                   // Y procede a cambiar a la pantalla ENDING para despedirse del estudiante
            }
        }
    }
}

// Frontend de CONFIRMATION

void screenConfirmationDraw(bool existstudent,                  // Necesita saber si el estudiante existe en la base de datos
                            bool correctstudent,                // Necesita también saber si el estudiante se ha confirmado
                            const std::string& studentName)     // Y necesita el nombre del estudiante
{
    if (!correctstudent && existstudent)              // Si NO se ha confirmado de que el estudiante sea el correcto pero el estudiante SÍ existe...
    {
        DrawTextEx(fontTtf, "Verifique si su nombre es correcto:"s.data(),                                                      // Mostrará un mensaje para decirle al estudiante que confirme su nombre
                   (Vector2){(float)centertext("Verifique si su nombre es correcto:"s, screenWidth, fontSize),
                              (float)(screenHeight * 0.2)},
                   fontSize, 2, COLORTEXTO);
        DrawTextEx(fontTtf, studentName.data(),                                                                                 // Luego, mostrará el nombre del estudiante
                   (Vector2){(float)centertext(studentName, screenWidth, fontSize),
                              (float)(screenHeight * 0.3)},
                   fontSize, 2, COLORTEXTO);
        PrettyDrawRectangle(continuarPtr);                                                                                      // Dibujará el botón de continuar
        PrettyDrawRectangle(regresarPtr);                                                                                       // Como también dibujará el botón de regresar
        DrawTextEx(fontTtf, continuarPtr->name.data(),
                   (Vector2){continuarPtr->xloc + (float)centertext(continuarPtr->name, continuarPtr->xsize, fontSize),         // Luego dibujará el nombre del botón continuar
                              continuarPtr->yloc + (float)((continuarPtr->ysize - fontSize) / 2)},
                   fontSize, 2, BLACK);
        DrawTextEx(fontTtf, regresarPtr->name.data(),                                                                           // Y luego dibujará el nombre de regresar
                   (Vector2){regresarPtr->xloc + (float)centertext(regresarPtr->name, regresarPtr->xsize, fontSize),
                              regresarPtr->yloc + (float)((regresarPtr->ysize - fontSize) / 2)},
                   fontSize, 2, BLACK);
    }
    else if (!existstudent)                           // Si el estudiante NO existe en la base de datos...
    {
        DrawTextEx(fontTtf, "Usted no se encontro en la base de datos del VOCA"s.data(),                                        // Mostrará un mensaje diciendole al estudiante que no se encontró en la base de datos
                   (Vector2){(float)centertext("Usted no se encontro en la base de datos del VOCA"s, screenWidth, fontSize),
                              (float)(screenHeight * 0.2)},
                   fontSize, 2, COLORTEXTO);
        DrawTextEx(fontTtf, "Verifique que su cedula fue digitada correctamente:"s.data(),                                      // Y que verifique que digitó bien la cédula
                   (Vector2){(float)centertext("Verifique que su cedula fue digitada correctamente:"s, screenWidth, fontSize),
                              (float)(screenHeight * 0.3)},
                   fontSize, 2, COLORTEXTO);
        DrawTextEx(fontTtf, cedulaBarPtr->input.data(),                                                                         // Y mostrará la cédula digitada
                   (Vector2){(float)centertext(cedulaBarPtr->input, screenWidth, fontSize),
                              (float)(screenHeight * 0.4)},
                   fontSize, 2, COLORTEXTO);
        PrettyDrawRectangle(regresarPtr);                                                                                       // Esta vez solo dibujará el botón de regresar
        DrawTextEx(fontTtf, regresarPtr->name.data(),                                                                           // Y el nombre del botón regresar
                   (Vector2){regresarPtr->xloc + (float)centertext(regresarPtr->name, regresarPtr->xsize, fontSize),
                              regresarPtr->yloc + (float)((regresarPtr->ysize - fontSize) / 2)},
                   fontSize, 2, BLACK);
    }
    else                                              // Si ninguno de los dos if ocurrieron, significa que lo que corresponde dibujar sería el mensaje para confirmar el voto, entonces...
    {
        std::string mselected = "Seleccionó "s + partidoSelected + " es correcto?"s;                                            // Mostrará un mensaje diciendo que confirme el partido por el que votó, y el nombre del partido
        DrawTextEx(fontTtf, mselected.data(),
                   (Vector2){(float)centertext(mselected, screenWidth, fontSize),
                              (float)(screenHeight * 0.2)},
                   fontSize, 2, COLORTEXTO);
        PrettyDrawRectangle(continuarPtr);                                                                                      // Dibujará el botón continuar
        PrettyDrawRectangle(regresarPtr);                                                                                       // Como también dibujará el botón de regresar
        DrawTextEx(fontTtf, continuarPtr->name.data(),                                                                          // Procederá a dibujar el nombre del botón continuar
                   (Vector2){continuarPtr->xloc + (float)centertext(continuarPtr->name, continuarPtr->xsize, fontSize),
                              continuarPtr->yloc + (float)((continuarPtr->ysize - fontSize) / 2)},
                   fontSize, 2, BLACK);
        DrawTextEx(fontTtf, regresarPtr->name.data(),                                                                           // Como también dibujará el nombre del botón regresar
                   (Vector2){regresarPtr->xloc + (float)centertext(regresarPtr->name, regresarPtr->xsize, fontSize),
                              regresarPtr->yloc + (float)((regresarPtr->ysize - fontSize) / 2)},
                   fontSize, 2, BLACK);
    }
}
