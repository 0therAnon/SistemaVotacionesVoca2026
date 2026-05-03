#include "../globals.hpp"
#include "config.hpp"
#include "../platform/encoding.hpp"
#include <filesystem>               // Llamada a la librería filesystem, la cual se encarga de buscar el archivo de configuración en los archivos locales
#include <fstream>                  // Llamada a la librería fstream, la cual permite la lectura de archivos
#include <string.h>                 // Llamada a la librería string, la cual permite la creación de variables de tipo string y manipulación de estas

int configureData()                 // Nombro a la función configureData() sin ningún argumento ya que no necesita alguno, y la función es de tipo int por que quiero que retorne un valor entero como código de estado
{
    statusCodeConfig = 0;           // Variable que almacenará el código de estado de la función a nivel global, se define en 0 al inicio de la función, y su valor será cambiado en caso de algún error
    fs::path pathToConfig = fs::current_path() / ".config";       // fs::path es un tipo de variable que almacena una ruta específica, y la función fs::current_path() se encarga de buscar la ruta actual del programa, pathToConfig almacena la ruta del archivo de configuración

    if (!fs::exists(pathToConfig))  // Si la ruta del archivo de configuración NO (NO por que se usa símbolo de exclamación, lo cual niega el resultado) es una ruta válida (es decir, el archivo de configuración NO existe) procederá con este if
    {
        configFile.open(pathToConfig);                            // Abrirá el archivo de configuración (para crearlo) por medio de la ruta anteriormente guardada en pathToConfig
        configFile.close();                                       // Lo cierra, ya que solo se permitió el uso de este para crearlo, no para escribir en el archivo
        std::ofstream configFile(pathToConfig);                   // Abrirá el archivo de configuración y lo guardará en una variable tipo std::ofstream (permite la escritura de datos en el archivo) con el nombre configFile
        std::string defaultConfig;                                // Este string, almacenará los valores predeterminados de la configuración, los cuales los asignaré en la siguiente línea
        defaultConfig = "\
[Credentials]\n\
server=127.0.0.0\n\
user=root\n\
password=0th3r@N0n\n\
database=Votaciones\n\
port=3306\n\
admPassword=1234\n\
labName=\n\
\n\
[Extra]\n\
nameColumnPartidosNombre=Nombre\n\
nameColumnNuloPartido=NULO\n\
nameColumnVotosNombre=Votos\n\
nameColumnVotoNombre=Voto\n\
nameTableEstudiantes=Estudiantes\n\
nameTablePartidos=Partidos\n\
\n\
[Paths]\n\
pathProgramFont=./fonts/Inter_24pt-Regular.ttf\n\
pathLogsFont=./fonts/GoMonoNerdFont-Regular.ttf\n\
pathPdfFont=./fonts/LibertinusMono-Regular.ttf\n\
informeName=Informe VOCA Votaciones 2026.pdf\n\
";
        configFile << defaultConfig <<std::endl;                  // Se escribe el archivo de configuración nuevo con los valores predeterminados
        configFile.close();                                       // Cierra el archivo
        statusCodeConfig = 1;                                     // Devuelve estado de error en la configuración
    }
    std::ifstream configFile(pathToConfig);                       // Abrirá el archivo de configuración y lo guardará en una variable tipo std::ifstream (permite la lectura de datos del archivo) con el nombre configFile

    for (int parameter = 0; parameter < (int)configurations.size(); parameter++)  // Recorrerá cada parámetro del vector configurations, esto para ir asignando el valor de cada variable que necesita los valores del archivo de configuración
    {
        bool startAdding = false;                                                 // startAdding ayuda a saber cuando se encuentra un símbolo de "=" en la línea actual, y lo que vaya después de ese símbolo se guardará
        bool found = false;                                                       // Ayuda a verificar si el argumento que se estaba buscando se logró encontrar o no (esto para prevenir de que falte alguna línea en el archivo de configuración)
        std::string line = "";                                                    // Almacenará el string de la línea del archivo de configuración que se esté leyendo
        while (std::getline(configFile, line))                                    // Este while, recorrerá cada línea del archivo que se le haya asignado a la variable line
        {
            if (line.find(configurations[parameter]) != std::string::npos)        /* find() es una función para buscar en la línea algo, por ejemplo, si parameter es igual a 0, se buscará "server=" en la línea, por que el índice 0 de
                                                                                     configurations es "server=" y la función de std::npos, le dice al programa que no busque en ninguna línea del archivo que no sea la línea actual */
            {
                found = true;
                std::string paramStr = "";                                        // paramStr almacenará todo el argumento/parámetro después del símbolo "=" en la línea
                for (auto ch : line)                                              // Este bucle for, sirve para recorrer todos los carácteres de la línea actual
                {
                    if (startAdding) paramStr += ch;                              // Si el bool de startAdding es igual a true, paramStr empezará a acumular los carácteres
                    if (ch == '=') startAdding = true;                            // Para que startAdding sea igual a true, tiene que llegar al símbolo de igual, lo cual permitiría la acumulación del parámetro en paramStr
                }
                free(configurations[parameter]);                                  /* Si por ejemplo, se está recorriendo el índice 0 del vector configurations, el cual es igual a "server=", lo que hace free() es liberar el puntero que
                                                                                     apunta a ese string en la memoria, y se usará para apuntar a otro lado */
                configurations[parameter] = strdup(paramStr.data());              // Con esta línea, se hace la nueva asignación del puntero al argumento que se consiguió con el bucle for anterior
                configFile.clear();                                               // Se limpia todo lo acumulado en la memoria por el recorrido que se le hizo al archivo con la función find() y getline()
                configFile.seekg(0);                                              /* Le dice al programa que quiere apuntar a la línea 0 del archivo para comenzar de nuevo, ya que si no hago esto, en la siguiente busqueda
                                                                                     el programa buscará el próximo parámetro en la última línea que se quedó buscando anteriormente */
                break;                                                            // Rompe el bucle while, ya terminó lo que tenía que hacer con la línea encontrada por medio de find()
            }
        }
        if (!found)                                                               // Si la línea buscada no llega a ser encontrada...
        {
            configFile.clear();                                                   // Se limpiará lo acumulado en memoria por find() y getline()
            configFile.seekg(0);                                                  // La posición en la búsqueda para la siguiente línea se reinicia a 0
            statusCodeConfig = 2;                                                 // El estado de la función se almacenará con un estado 2 en la variable statusCodeConfig, esto para especificar que el error que faltó una o varias líneas
        }
    }
    configFile.close();                     // Cierra el archivo cuando ya termina todas sus operaciones
    return statusCodeConfig;                // Retorno del programa, devuelve el código de estado correspondiente
}

int loadConfig()    // Nombro a la función loadConfig() sin argumentos ya que no son necesarios, y es de tipo int por plena costumbre
{
    /* El propósito principal de esta función es el hecho de cargar todos los parámetros posibles a las barras de entrada en la pantalla CONFIGURATION, ya que sino, al guardar los datos en CONFIGURATION habría que
       tener que escribir todo el contenido de cada parámetro a pesar de que esté especificado en el archivo de configuración, y sería sumamente tedioso volver a escribir todos los datos a pesar de que se encontrasen
       ya escritos en .config, en dado caso de que no me haya explicado bien, pueden probar lo que hace esta función alterando la contraseña de la base de datos o cualquier otro parámetro que los envíe a configurar el
       programa y descomentar "return 0;" antes de todos los renombramientos de input32 e input en cada barra de tareas, esto haría que no se ejecute nada de la función y se retorne inmediatamente sin poder ejecutar nada */

    // return 0;

    /* En cada línea, lo que ocurre es que cada barra de tareas que aparece en CONFIGURATION, su contenido se llena una por una con lo que se pudo obtener del archivo de configuración, hay que recordar que los objetos
       de clase inputBar tienen dos tipos de valores que se guardan al recibir datos de entrada, los cuales son input32 que sirven para manipular el texto internamente con la función inputfunc, e input, que sirve para
       mostrar el texto en el frontend en formato UTF-8, así que por eso se le cambia tanto el valor de input32 a cada barra de entrada tanto el de la variable input32 como input, solo que en input32 se deben traducir */

    termBars[0]->input32 = UTF8ToUTF32(*server);                      /**/              termBars[0]->input = *server;
    termBars[1]->input32 = UTF8ToUTF32(*port);                        /**/              termBars[1]->input = *port;
    termBars[2]->input32 = UTF8ToUTF32(*database);                    /**/              termBars[2]->input = *database;
    termBars[3]->input32 = UTF8ToUTF32(*user);                        /**/              termBars[3]->input = *user;
    termBars[4]->input32 = UTF8ToUTF32(*password);                    /**/              termBars[4]->input = *password;
    admPasswordBarPtr->input32 = UTF8ToUTF32(*admPassword);           /**/              admPasswordBarPtr->input = *admPassword;
    labNameBarPtr->input32 = UTF8ToUTF32(*labName);                   /**/              labNameBarPtr->input = *labName;
    extraBars[0]->input32 = UTF8ToUTF32(*nameTableEstudiantes);       /**/              extraBars[0]->input = *nameTableEstudiantes;
    extraBars[1]->input32 = UTF8ToUTF32(*nameTablePartidos);          /**/              extraBars[1]->input = *nameTablePartidos;
    extraBars[2]->input32 = UTF8ToUTF32(*nameColumnPartidosNombre);   /**/              extraBars[2]->input = *nameColumnPartidosNombre;
    extraBars[3]->input32 = UTF8ToUTF32(*nameColumnNuloPartido);      /**/              extraBars[3]->input = *nameColumnNuloPartido;
    extraBars[4]->input32 = UTF8ToUTF32(*nameColumnVotosNombre);      /**/              extraBars[4]->input = *nameColumnVotosNombre;
    extraBars[5]->input32 = UTF8ToUTF32(*nameColumnVotoNombre);       /**/              extraBars[5]->input = *nameColumnVotoNombre;
    pathBars[0]->input32 = UTF8ToUTF32(*pathProgramFont);             /**/              pathBars[0]->input = *pathProgramFont;
    pathBars[1]->input32 = UTF8ToUTF32(*pathLogsFont);                /**/              pathBars[1]->input = *pathLogsFont;
    pathBars[2]->input32 = UTF8ToUTF32(*pathPdfFont);                 /**/              pathBars[2]->input = *pathPdfFont;
    pathBars[3]->input32 = UTF8ToUTF32(*informeName);                 /**/              pathBars[3]->input = *informeName;
    return 0;
}
