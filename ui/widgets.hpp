#pragma once
#include <string>
#include "raylib.h"

// La función principal de este archivo es la declaración de las clases de los objetos en el programa

// Clase padre — nombre derivado de las iniciales de sus propiedades principales: name, x, y, xsize, ysize, status
class nxyxys {                                // Declaración de la clase padre
  public:                                     // Asignaré las siguientes variables como public, ya que pueden ser modificables en cualquier parte del programa
    std::string name;                         // Declaración de que "name" va a ser una variable de tipo std::string
    float xloc;                               // Declaración de que xloc va a ser una variable de tipo float (almacena decimales), esta variable se encarga de la ubicación en el eje X del objeto (eje X es el ancho de la pantalla)
    float yloc;                               // Declaración de que yloc va a ser una variable de tipo float, esta variable se encarga de la ubicación en el eje Y del objeto (eje Y es la altura de la pantalla)
    float xsize;                              // Declaración de que xsize va a ser una variable de tipo float, esta variable se encarga del tamaño en el eje X (ancho) del objeto
    float ysize;                              // Declaración de que ysize va a ser una variable de tipo float, esta variable se encarga del tamaño en el eje Y (alto) del objeto
    int status;                               // Estado 0 significa que no fue presionado estado 3 significa que ha sido presionado, estado 2 significa que fue presionado y tiene el cursor encima y estado 1 solo significa que fue presionado
    Color highColor;                          // Es el color que recibe el objeto cuando ha sido presionado o resaltado
    Color normalColor;                        // Es el color que recibe el objeto cuando no ha recibido ninguna interacción
    virtual void vincular() const = 0;
    virtual ~nxyxys() {}
};

class button : public nxyxys {                // Declaración de la clase button, es la que tiene las propiedades de los botones, "public nxyxys" significa que recibe las propiedades de la clase padre
  public:
    void vincular() const override {}
    std::string selfquery = "";               // Se declara que button tendrá una propiedad llamada selfquery, esta propiedad sirve para almacenar queries para enviar a la base de datos por sí mismos
    std::string outLog    = "";               // outLog es el registro de las respuestas que le haya enviado la base de datos, se usa en el panel de administracion, para más informacion leer logfunction en ui/drawing.cpp
    std::string oType     = "button";         // Se declara de que el tipo de objeto es "button", esto sirve para identificar el tipo de objeto actual
    Color highColor   = {220, 200, 160, 255};  // Beige cálido al presionar
    Color normalColor = {245, 235, 210, 255};  // Beige pastel en reposo
};

class inputBar : public nxyxys {              // Declaración de la clase inputBar, es la encargada de las propiedades de las barras de entrada que permiten la escritura de datos
  public:
    void vincular() const override {}
    std::string    input;                     // input es la variable que muestra en el frontend los datos de entrada en UTF-8, para mas informacion acerca de codificaciones, leer platform/README.txt
    std::u32string input32;                   // input32 es la variable que maneja internamente los datos de entrada en UTF-32, para mas informacion acerca de codificaciones, leer platform/README.txt
    std::string oType     = "inputBar";       // Se declara que el tipo de objeto es un inputBar
    Color highColor   = WHITE;                // Color predeterminado cuando se resalta     | VOCADORADO
    Color normalColor = WHITE;                // Color predeterminado en su estado normal   | VOCAVERDE
};

class sqlobject : public nxyxys {             // Declaración de la clase sqlobject, se usa principalmente en objetos que se hayan cargado de la base de datos SQL, como tablas, pero no columnas, las columnas tienen su propio objeto
  public:
    void vincular() const override {}
    int maxlen;                               // Declara el tamaño máximo de datos de entrada, esto puede variar
    std::string type;                         // Declara el tipo de objeto en caso de que tenga tipo
    int id;                                   // Declara el identificador del objeto en caso de que tenga que necesitar uno
    std::string oType     = "sqlobject";      // Se declara que el tipo de objeto es "sqlobject"
    Color highColor   = {220, 200, 160, 255};  // Beige cálido al presionar
    Color normalColor = {245, 235, 210, 255};  // Beige pastel en reposo
};

class column : public nxyxys {                // Declaración de la clase column, esta almacena columnas cargadas desde la base de datos, es la que más propiedades tiene
  public:
    void vincular() const override {}
    std::string  fromTable;                   // Especifica de cual tabla proviene la columna actual
    std::string   input;                      // input es la variable que muestra en el frontend los datos de entrada en UTF-8
    std::u32string input32;                   // input32 es la variable que maneja internamente los datos de entrada en UTF-32
    std::string  type;                        // Almacena el tipo de dato a introducir en la columna, es decir, si en la base de datos se carga de que la columna es de tipo int, la columna solo admitirá datos de tipo entero
    std::string  maxlen;                      // Almacena la cantidad máxima de datos a introducir, es decir, si en la base de datos se asigna un límite de 50 carácteres para almacenar, entonces solo se podrán escribir 50 carácteres
    Color highColor   = WHITE;                // Color predeterminado cuando se resalta     | VOCADORADO
    Color normalColor = WHITE;                // color predeterminado en su estado normal   | VOCAVERDE
    int id;
    std::string oType = "column";
};
