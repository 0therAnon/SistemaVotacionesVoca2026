#include "../globals.hpp"
#include "screen_ending.hpp"
#include "../ui/drawing.hpp"

// Esta función se encarga del backend de la pantalla ENDING

void screenEndingUpdate(Screen& currentScreen,      // Necesita a currentScreen para modificar la pantalla actual
                        int verifyvote,             // Necesita a verifyvote para conocer su valor
                        bool& correctstudent)       // Necesita a correctstudent para modificar su valor
{
    framesCounter++;                                // framesCounter es un contador que cuenta los frames por segundo
    if (framesCounter > 600)                        // 60 fps * 10 seg = 600 frames | Aquí espero a que pasen 10 segundos para hacer lo siguiente...
    {
        cedulaBarPtr->status = 4;                                               // Cambio el valor de cedulaBar a 4, para que automáticamente cuando se envíe el usuario a MAINMENU, ya pueda escribir en la barra
        cedulaBarPtr->input  = "";                                              // Vacía el valor de la barra para que no se vea el valor de la cédula anterior
        cedulaBarPtr->input32 = U"";
        correctstudent = false;                                                 // correctstudent se cambia a false, para que solo sea true cuando se vuelva a verificar el estudiante
        cedulaPtr->selfquery = "SELECT * FROM Estudiantes WHERE Cedula = '";    // Se reinicia la query encargada de obtener el nombre del estudiante
        votarPtr->selfquery  = "UPDATE Estudiantes SET Voto = '";               // Se reinicia la query encargada de modificar el valor del voto del estudiante
        framesCounter  = 0;                                                     // Se reinicia el contador
        partidoSelected = "";                                                   // El partido actual se vacía
        currentScreen = MAINMENU;                                               // Cambia la pantalla actual a MAINMENU
    }
}

// Frontend de ENDING

void screenEndingDraw(Screen &currentScreen, int verifyvote)
{
    if (currentScreen == ENDING) transition("show");
    if (verifyvote == 0)                            // Si verifyvote, el cual almacena el largo de la respuesta que verifica si ya el estudiante ha votado o no, es igual a cero, significa que el estudiante ya votó entonces...
    {
        DrawTextEx(fontTtf, "Ya usted ha votado, no puede hacerlo de nuevo",                                                  // Dibuja el mensaje diciendole al estudiante de que ya ha votado
                   (Vector2){(float)centertext("Ya usted ha votado, no puede hacerlo de nuevo", screenWidth, fontSize),
                              (float)(screenHeight * 0.5)},
                   fontSize, 2, COLORTEXTO);
    }
    else                                            // En caso de que verifyvote sea un valor superior a cero, significa que se llegó a la pantalla ENDING después de ejercer el voto, entonces...
    {
        DrawTextEx(fontTtf, "Muchas gracias por su voto, puede retirarse",                                                    // Dibuja el mensaje diciendole al estudiante de que ya ha votado
                   (Vector2){(float)centertext("Muchas gracias por su voto, puede retirarse", screenWidth, fontSize),
                              (float)(screenHeight * 0.5)},
                   fontSize, 2, COLORTEXTO);
    }
    if (currentScreen != ENDING && alphaIsZero == false)
    {
        transition("hide");
        screenEndingDraw(currentScreen, verifyvote);
    }
}
