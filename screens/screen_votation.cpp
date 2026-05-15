#include "../globals.hpp"
#include "screen_votation.hpp"
#include "../ui/input.hpp"
#include "../ui/drawing.hpp"

// Función del backend de la pantalla VOTATION

void screenVotationUpdate(Screen& currentScreen,        // Necesita la variable currentScreen para cambiar la pantalla del programa
                          bool& votoBlanco)             // Necesita la variable votoBlanco para modificar y leer su valor, el cual indica si se intentó votar en blanco, es decir, ninguna opción incluyendo nulo, lo cual NO es válido
{
    // Se actualiza el estado de todos los botones cada frame para detectar hover correctamente
    votarPtr->status = isPressed(votarPtr);             // Actualiza el estado del botón votar cada frame para detectar hover

    for (int i = 0; i < (int)partidosVec.size(); i++)   // Recorrerá todos los partidos
    {
        partidosVec[i]->status = isPressed(partidosVec[i]);                 // Verifica el estado de cada partido
        if (partidosVec[i]->status == 4)                                    // Si un partido recibe un clic
            partidoSelected = partidosVec[i]->name;                         // El partido que recibió el clic se declara como partido seleccionado
    }

    if (votarPtr->status == 4)                                              // Si se presiona el botón votar...
    {
        if (!partidoSelected.empty())                   // Si hay un partido seleccionado...
        {
            continuarPtr->status = 0;                   // Se resetea el estado de continuar para que no aparezca resaltado al llegar a CONFIRMATION
            regresarPtr->status  = 0;                   // Se resetea el estado de regresar también por la misma razón
            votarPtr->status     = 0;                   // Se resetea el estado de votar
            for (int i = 0; i < (int)partidosVec.size(); i++)
                partidosVec[i]->status = 0;             // Se resetea el estado de todos los partidos
            currentScreen = CONFIRMATION;               // Si partidoSelected NO se encuentra vacío, enviará al usuario a CONFIRMATION para confirmar su voto
        }
        else votoBlanco = true;                         // Si partidoSelected SÍ se encuentra vacío, le mostrará al usuario en el frontend un mensaje diciendole de que NO se puede votar en blanco
    }
}

// Frontend de VOTATION

void screenVotationDraw(Screen &currentScreen, bool& votoBlanco)           // Necesita a la variable votoBlanco para verificar si debe mostrar el mensaje comunicando de que debe votar por una opción
{
    if (currentScreen == VOTATION) transition("show");
    DrawTextEx(fontTtf, "Por favor, escoja un partido por el que desea votar"s.data(),                                            // En el frontend mostrará un mensaje diciendo de que vote por algún partido
               (Vector2){(float)centertext("Por favor, escoja un partido por el que desea votar"s, screenWidth, fontSize),
                          (float)(screenHeight * 0.1)},
               fontSize, 2, COLORTEXTO);
    DrawTextEx(fontTtf, "O si lo desea, puede votar nulo"s.data(),                                                                // Además de que comunicará que puede votar nulo
               (Vector2){(float)centertext("O si lo desea, puede votar nulo"s, screenWidth, fontSize),
                          (float)(screenHeight * 0.2)},
               fontSize, 2, COLORTEXTO);
    drawParties(partidosVec, littleFontSize * 1.5, partidoSelected);                                                              // Dibujará a los partidos disponibles, y el string "custom" significa que con bordes personalizados
    PrettyDrawRectangle(votarPtr);                                                                                                // Dibujará el botón para votar
    DrawTextEx(fontTtf, votarPtr->name.data(),                                                                                    // Y dibujará el nombre de ese botón
               (Vector2){votarPtr->xloc + (float)centertext(votarPtr->name, continuarPtr->xsize, fontSize),
                          votarPtr->yloc + (float)((votarPtr->ysize - fontSize) / 2)},
               fontSize, 2, NEGRO);
    if (votoBlanco) shortmessage("Tiene que seleccionar una opción", mediumFontSize, votoBlanco);                                 // Si votoBlanco es verdadero, es decir, se intentó votar por ninguna opción, aparecerá un mensaje
    if (currentScreen != VOTATION && alphaIsZero == false)
    {
        transition("hide");
        screenVotationDraw(currentScreen, votoBlanco);
    }
}
