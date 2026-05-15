#pragma once
#include "../globals.hpp"
#include "input.hpp"
#include <string>
#include <vector>

// ── Forward declarations ──────────────────────────────────────────────────────
int centertext(std::string message, double width, double fontsize);
std::string keepLastNLines(const std::string& text, int n = 1000);
// ── PrettyDrawRectangle ───────────────────────────────────────────────────────
template<typename T>
inline void PrettyDrawRectangle(T obj)              // Esta función se encarga de recibir un objeto y dibujarlo con bordes redondos, además de líneas redondas
{
    // Efecto de presión: cuando status es 3 (presionado con cursor encima) o 4 (clic en este instante),
    // el botón se desplaza 3 pixeles hacia abajo y se encoge, simulando que fue físicamente hundido
    float pressOffset = (obj->status == 3 || obj->status == 4) ? 3.0f : 0.0f;
    float sizeReduce  = (obj->status == 3 || obj->status == 4) ? 3.0f : 0.0f;

    // Efecto hover: cuando status es 1 (cursor encima sin clic), el botón crece 2 pixeles
    // en todas las direcciones para avisarle al usuario que puede presionarlo
    float hoverExpand = (obj->status == 1) ? 2.0f : 0.0f;

    // Se recalculan las coordenadas aplicando los efectos anteriores
    float rx = obj->xloc  - hoverExpand;
    float ry = obj->yloc  - hoverExpand + pressOffset;
    float rw = obj->xsize + hoverExpand * 2 - sizeReduce;
    float rh = obj->ysize + hoverExpand * 2 - sizeReduce;

    // Sombra suave debajo del botón, desaparece al presionar para reforzar el efecto de hundimiento
    if (obj->status != 3 && obj->status != 4)
        DrawRectangleRounded({rx + 4, ry + 6, rw, rh}, 0.4f, 0, GRIS);

    // Cuerpo del botón con bordes en forma de píldora (0.5f)
    // Si status > 1 usa highColor (beige cálido), sino usa normalColor (beige pastel)
    DrawRectangleRounded({rx, ry, rw, rh},
                         0.4f, 0, obj->status > 1 ? *(obj->highColor) : *(obj->normalColor));

    // Borde dorado: más grueso en hover, más fino al presionar, grosor estándar en reposo
    float borderThick = (obj->status == 1)                     ? 5.0f :   // Hover: 6px para feedback visual claro
                        (obj->status == 3 || obj->status == 4) ? 6.5f :   // Presionado: 2px, el botón se hundió
                                                                  3.5f;   // Reposo: 5.5px, borde dorado bien visible
    DrawRectangleRoundedLinesEx({rx, ry, rw, rh}, 0.4f, 0, borderThick, DORADO_BORDE);
}

// ── drawSelected ──────────────────────────────────────────────────────────────
template<typename V>
inline void drawSelected(V butVector,               // Esta función se encarga de dibujar los objetos que se encuentran en un vector
                        double fontsize,            // También pide un tamaño del font para los nombres
                        std::string selected)       // String que se compara para verificar si por ejemplo, el botón en el vector coincide con la pestaña en uso en el panel de administración, entonces aunque su status cambie, se rellena resaltado
{
    for (int i = 0; i < (int)butVector.size(); i++)       // Recorre todos los botones del vector
    {
        if (butVector[i]->name != selected)               // Si el botón actual en el vector NO coincide con el string del nombre del botón resaltado, entonces...
        {
            PrettyDrawRectangle(butVector[i]);            // Simplemente lo dibuja con PrettyDrawRectangle, el cual por sí mismo se encarga de verificar su estado
        }
        else                                              // En caso de que el nombre del botón SÍ coincida con el string del nombre del botón resaltado, entonces se procederá a dibujar el botón de forma manual resaltado
        {
            DrawRectangleRounded({(float)butVector[i]->xloc, (float)butVector[i]->yloc,                             // Dibuja el botón con bordes redondos
                                  (float)butVector[i]->xsize, (float)butVector[i]->ysize},
                                 0.5f, 0, *(butVector[i]->highColor));
            DrawRectangleRoundedLinesEx({(float)butVector[i]->xloc, (float)butVector[i]->yloc,                      // Dibuja las líneas del borde del botón con bordes redondos
                                         (float)butVector[i]->xsize, (float)butVector[i]->ysize},
                                        0.5f, 0, 5.5f, DORADO_BORDE);
        }
        float width = (butVector[i]->xloc + butVector[i]->xsize * 0.5f) * 2.0f;                 // Se calcula el ancho del botón, esto se usará para luego llamar a centertext() para centrar el nombre en el botón
        DrawTextEx(fontTtf, butVector[i]->name.data(),                                          // Se dibuja el nombre del botón
                   (Vector2){(float)centertext(butVector[i]->name, width, fontsize),
                              (float)((butVector[i]->yloc + butVector[i]->ysize + butVector[i]->yloc) / 2.0 - fontsize / 2.0)},
                   fontsize, 2, NEGRO);
    }
}

// ── drawImages ────────────────────────────────────────────────────────────────
template<typename X>
inline void drawImages(X textureVector)                 // Esta función se encarga de dibujar las imágenes que se encuentran en un vector
{
    for (auto textr : textureVector)
        DrawTexture(textr->texture, textr->xloc, textr->yloc, WHITE);
}

template<typename P>
inline void drawParties(P pVec,
                        double fontsize,
                        std::string selected)
{
    for (int i = 0; i < (int)pVec.size(); i++)       // Recorre todos los botones del vector
    {
        float width = (pVec[i]->xloc + pVec[i]->xsize * 0.5f) * 2.0f;                      // Se calcula el ancho del botón, esto se usará para luego llamar a centertext() para centrar el nombre en el botón
        if (pVec[i]->name != selected)               // Si el botón actual en el vector NO coincide con el string del nombre del botón resaltado, entonces...
        {
            float pressOffset = (pVec[i]->status == 3 || pVec[i]->status == 4) ? 3.0f : 0.0f;
            float sizeReduce  = (pVec[i]->status == 3 || pVec[i]->status == 4) ? 3.0f : 0.0f;
            float hoverExpand = (pVec[i]->status == 1) ? 2.0f : 0.0f;
            float rx = pVec[i]->xloc  - hoverExpand;
            float ry = pVec[i]->yloc  - hoverExpand + pressOffset;
            float rw = pVec[i]->xsize + hoverExpand * 2 - sizeReduce;
            float rh = pVec[i]->ysize + hoverExpand * 2 - sizeReduce;
            if (pVec[i]->status != 3 && pVec[i]->status != 4)
                DrawRectangleRoundedLines({rx + 4, ry + 6, rw, rh}, 0.5f, 0, pVec[i]->fadeColor);
            DrawTexture(pVec[i]->flag, rx, ry, WHITE);
            float borderThick = (pVec[i]->status == 1)                         ? 5.0f :   // Hover: 6px para feedback visual claro
                                (pVec[i]->status == 3 || pVec[i]->status == 4) ? 6.5f :   // Presionado: 2px, el botón se hundió
                                                                                 3.5f;    // Reposo: 5.5px, borde dorado bien visible
            DrawRectangleRoundedLinesEx({rx, ry, rw, rh}, 0.5f, 0, borderThick, pVec[i]->fadeColor);
            DrawTextEx(fontTtf, pVec[i]->name.data(),                                          // Se dibuja el nombre del botón
                       (Vector2){(float)centertext(pVec[i]->name, width, fontsize),
                                  (float)((pVec[i]->yloc + pVec[i]->ysize + pVec[i]->yloc) / 2.0 - fontsize / 2.0)},
                       fontsize, 2, NEGRO);
        }
        else                                              // En caso de que el nombre del botón SÍ coincida con el string del nombre del botón resaltado, entonces se procederá a dibujar el botón de forma manual resaltado
        {
            DrawTexture(pVec[i]->flag, pVec[i]->flagxloc, pVec[i]->flagyloc, WHITE);
            DrawRectangleRoundedLinesEx({(float)pVec[i]->xloc, (float)pVec[i]->yloc,                      // Dibuja las líneas del borde del botón con bordes redondos
                                         (float)pVec[i]->xsize, (float)pVec[i]->ysize},
                                        0.5f, 0, 5.5f, pVec[i]->subColor);
            DrawTextEx(fontTtf, pVec[i]->name.data(),                                          // Se dibuja el nombre del botón
                       (Vector2){(float)centertext(pVec[i]->name, width, fontsize),
                                  (float)((pVec[i]->yloc + pVec[i]->ysize + pVec[i]->yloc) / 2.0 - fontsize / 2.0)},
                       fontsize, 2, NEGRO);
        }
    }
}

template<typename I>
inline Color getMainColor(I image)                       // Esta función tiene como propósito encontrar el color principal en una imagen
{
    Color *pixels = LoadImageColors(image);             // Se cargan los pixeles de las imágenes para ser recorridas
    long r = 0, g = 0, b = 0, a = 0;                    // Se asignan valores long a r, g, b y alpha, ya que pueden haber muchos valores de cada uno dependiendo de la imagen
    int pixelCount = image.width * image.height;        // Se declara a pixelCount como la variable que almacenará la cantidad total de pixeles

    for (int px = 0; px < pixelCount; px++) {           // Se declara px como cero, mientras px sea menor a pixelCount, px aumentará
        r += pixels[px].r;                              // Se le suma un valor a r dependiendo del valor que tenga en el pixel actual
        g += pixels[px].g;                              // Se le suma un valor a g dependiendo del valor que tenga en el pixel actual
        b += pixels[px].b;                              // Se le suma un valor a b dependiendo del valor que tenga en el pixel actual
        a += pixels[px].a;                              // Se le suma un valor a alpha dependiendo del valor que tenga en el pixel actual
    }

    UnloadImageColors(pixels);                          // Se descargan los pixeles de la imagen una vez ya recorrido el bucle

    return (Color){ (unsigned char)(r / pixelCount),    // Se retorna el valor del color que más se encontró en la imagen
                    (unsigned char)(g / pixelCount),
                    (unsigned char)(b / pixelCount),
                    255 };
}

// ── Non-template declarations (bodies in drawing.cpp) ────────────────────────
int         logfunction(std::string selected, double lastColumnMeasures = 0, double fsize = 0);
std::string drawcolumns(std::vector<sqlobject*>& cTables, std::vector<column*>& cVector,
                        std::string& tSelected, double fsize,
                        std::string& selected, char* mode = (char*)"default");
int         shortmessage(std::string msg, double fs, bool &activator, int timeFps = 150);
std::vector<double> statistics(std::string mode, std::string outputMode,
                               std::vector<double>& dataVec,
                               std::vector<parties*> partVec,
                               int posx = 0, int posy = 0);
int         alert(std::string botonActual, std::string mode);
int         transition(std::string mode);
