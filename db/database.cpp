#include "../globals.hpp"
#include "../platform/ipvalid.hpp"
#include "../ui/objects.hpp"
#include "../config/config.hpp"
#include "database.hpp"
#include <filesystem>
#include <string>

// Envia una query MySQL y envía la salida en la variable global "outQuery".
// query    : variable que contiene la query a enviar a la base de datos
// min      : columna minima a incluir en la salida
// sub      : columnas a restar desde el final
// response : 0 = ignorar resultado, 1 = texto con separador, 2 = "rows affected"
// divisor  : es el separador que se usará en la salida, se usa en la pestaña "Explorar" del panel de administración

int sendquery(const char* query, int min, int sub, int response, std::string divisor)
{
    std::cout<<query<<"\n";
    int columns = 0;                        // Variable que almacenará la cantidad total del columnas resultantes en la query, se declara con 0, luego se modifica
    outQuery = "";                          // outQuery es variable global que tendrá almacenado siempre las respuestas/outputs de las queries, si no se declara vacía, se combinará con un resultado anterior, y provocar errores
    if (mysql_query(conn, query))           // Se realiza la query que se pasó como argumento, y se usa "conn" como la conexión que realice la query, si todo sale bien la función mysql_query() devolverá 0 por lo que este if no debería ejecutarse
    {                                       // En caso de que este if se ejecute, significaría que la función anterior mysql_query() tuvo un error, por lo que respondió con un estado NO igual a 0, entrando en este if
        outQuery = mysql_error(conn);       // A outQuery se le pasa como contenido el error que haya ocurrido en la query a la base de datos por medio de la función mysql_error()
        outQuery += "\n";                   // Se le agrega un newline
        std::cout<<outQuery<<"\n\n\n\n";
        return 1;                           // Retorna 1, lo cual significa que hubo un error, si esta línea ocurre, el resto del código lo ignora
    }
    res = mysql_use_result(conn);           // Almacena un puntero a la respuesta de la conexión que se usó para realizar la query anterior, es decir, res almacena la respuesta
    if (response == 0) {                    // Si el argumento de response es igual a 0, significa que no se desea que se le asigne algún valor a outQuery mostrando la respuesta, sino ignorandola pero haberla ejecutado
        mysql_free_result(res);             // Se libera la respuesta para que no hayan conflictos en la memoria
        return 0;                           // Retorna 0, código de estado exitoso
    }
    if (res == nullptr)                     // Si el puntero no está apuntando a ninguna parte válida en la memoria, significaría que es muy probable de que haya ocurrido un error, entonces...
    {
        if (mysql_field_count(conn) != 0)   // Se intenta contar las filas de la respuesta, si al contar las filas de la respuesta ocurre un error, mysql_field_count() devolvería un código de estado distinto a 0, lo cual no sería normal
        {
            outQuery += "ERROR ";           // A outQuery se le asigna como contenido "ERROR"
            outQuery += mysql_error(conn);  // Luego se le asigna el error
            outQuery += "\n";               // Y por último un newline
            mysql_free_result(res);         // Se libera la respuesta para evitar conflictos en la memoría
            return 1;                       // Retorna 1, código de estado no exitoso
        }
    }
    if (response == 2)                                  // Si response es igual a 2, significa que se quiere obtener únicamente la cantidad de filas/registros que hayan sido afectados
    {
        int affected = mysql_affected_rows(conn);       // affected almacena la cantidad de registros affectados
        outQuery += "Query OK, ";                       // Se almacena "Query OK, ", lo cual en una base de datos MySQL se vería típicamente cuando la base de datos responde con las filas afectadas
        outQuery += std::to_string(affected);           // Se le almacenan también las filas afectadas
        outQuery += " rows affected\n";                 // Y se le agrega " rows affected\n" lo cuál también se vería en una base de datos MySQL como respuesta al modificar o actualizar registros
        mysql_free_result(res);                         // Libera la respuesta para evitar conflictos en la memoria
        return 0;                                       // Retorna 0, código de estado exitoso
    }

    /* En caso de que response no sea igual a 0 o 2 y no haya ocurrido error alguno, se procederá con una respuesta normal, la cual es guardar la respuesta en outQuery

    Aquí se construirá una "tabla en memoria", supongamos que la base de datos responde la query así:

    +---------+--------+-------+
    | nombre  | cedula | voto  |
    |---------|--------|-------|
    | Ana     | 12345  | NULL  |
    | Carlos  | 67890  | 1     |
    +---------+--------+-------+

    El código lo convertirá en un vector bidimensional, así:

    vecRows = [
        ["Ana",    "12345", "NULL"],  <- vecColumns de la fila 1
        ["Carlos", "67890", "1"   ]   <- vecColumns de la fila 2
    ]                                                                   */

    columns = mysql_num_fields(res);                                          // columns almacena la cantidad de columnas de la respuesta
    while ((row = mysql_fetch_row(res)) != NULL)                              // row almacenará todas las filas/registros de la respuesta mientras row no sea igual a NULL, ya que solo será NULL cuando termine de leer todos los registros
    {
        for (int i = 0; i < columns; i++)                                     // i recorrerá cada columna de row, es decir, este bucle está hecho para recorrer cada columna de cada registro, y la columna la almacena el vecColumns
        {
            if (row[i] != NULL) {vecColumns.push_back(row[i]);}               // Si el valor del campo de la columna actual NO es igual a NULL, lo guarda normal
            else                {vecColumns.push_back("NULL");}               // Si el valor del campo de la columna actual SÍ es igual a NULL, guarda el valor como un string diciendo "NULL"
        }
        vecRows.push_back(vecColumns);                                        // Cuando finalice el bucle, vecColumns tendrá todas las columnas del registro actual, lo único que se deberá de hacer será guardar el registro en veRows
        while (!vecColumns.empty()) vecColumns.pop_back();                    // Se vacía vecColumns, para su uso en el siguiente registro
    }
    mysql_free_result(res);                                                   // Libera la respuesta, cuando llega a esta línea ya las columnas y registros se guardaron en vectores, por lo que ya no necesita la respuesta original

    // Este bloque convierte vecRows en el string outQuery que se mostrará en pantalla. Los parámetros controlan qué columnas se muestran:

    // min — columna desde donde empezar. min=1 saltaría la primera columna.
    // max — columna hasta donde llegar. Es columns - sub, así que sub=1 ignoraría la última columna.
    // divisor — separador entre columnas. Por defecto es un espacio, pero puede ser " | " para hacerlo más legible.

    int max = columns - sub;                                                  // Es la columna máxima a llegar a mostrar en outQuery
    for (int fila = 0; fila < (int)vecRows.size(); fila++)                    // Recorre todas las filas/registros de vecRows
    {
        for (int columna = min; columna < max; columna++)                     // Recorre las columnas correspondientes a los argumentos de min y sub, siendo max la resta entre columns y sub
        {
            if (columna + 1 < max) outQuery += vecRows[fila][columna] + divisor;    // vecRows[fila][columna] se puede asociar a un campo específico en una fila y columna, esto se agrega a outQuery junto con un separador, como un espacio o "|"
            else              outQuery += vecRows[fila][columna];             // Si el campo actual es el último, no agregará un separador a outQuery
        }
        outQuery += "\n";                                                     // Se le agrega un newline para seguir con el siguiente registro/fila
    }
    while (!vecColumns.empty()) {vecColumns.pop_back();}  // Se limpian los vectores
    while (!vecRows.empty())    {vecRows.pop_back();}     // Se deben de limpiar para que funcionen correctamente en caso de que se invoque de nuevo la función sendquery() y funcione correctamente
    return 0;                                             // Retorna 0, código de estado exitoso
}

int updateData()              /* Esta función se encarga de cargar información necesaria en el programa desde la base de datos, como también se encarga de cargar los archivos de los fonts y verificar que la configuración esté correcta
                                 Esta función retorna un entero, el cual es el código de estado de la función, tiene varios códigos de estado para identificar de la mejor manera el error que ocurra, y devuelve 0 si todo de ejecutó con exito */
{
    // Se evalúan las rutas de las fuentes de las letras

    quantables = 0; quanstudents = 0; quanpartidos = 0; quancolumns = 0;      // En caso de que estos valores contengan información, deben de reiniciarse poniendose en 0

    if (!fs::exists(*pathProgramFont)) return 13;                             // Si NO existe la ruta el font del programa, devuelve código de estado 13 -> "Error en la fuente de la letra del programa"
    else                                                                      // En caso de que sí exista...
    {
        if (&fontTtf != nullptr)                                              // Si la dirección en memoria a la que apunta fontTtf no es nula, osea de que sí es un font real el que tenía cargado...
        {
            UnloadFont(fontTtf);                                              // Procederá a deshacerse del font que tenía cargado, esto para posteriormente cargarse el nuevo font que se comprobó que existía
        }
        fontTtf = LoadFontEx(*pathProgramFont, fontSize, 0, 250);             // En esta línea, se procederá a cargar el nuevo font, con la ruta de pathProgramFont
    }
    if (!fs::exists(*pathLogsFont)) return 14;                                // Si NO existe la ruta el font monoespacaiado, devuelve código de estado 14 -> "Error en la fuente monoespaciada del programa"
    else                                                                      // En caso de que sí exista...
    {
        if (&fontTtf != nullptr)                                              // Si la dirección en memoria a la que apunta fontTtf no es nula, osea de que sí es un font real el que tenía cargado...
        {
            UnloadFont(monoTtf);                                              // Procederá a deshacerse del font que tenía cargado, esto para posteriormente cargarse el nuevo font que se comprobó que existía
        }
        monoTtf = LoadFontEx(*pathLogsFont, fontSize, 0, 250);                // En esta línea, se procederá a cargar el nuevo font, con la ruta de pathLogsFont
    }

    if (!fs::exists(*pathPdfFont)) return 15;                                 // Si NO existe la ruta del font del PDF, devuelve código de estado 15 -> "Error en la ruta de la fuente de la letra del PDF"

    // Se procederá a reiniciar la conexión MySQL y validar la autenticación al servidor

    if (conn != nullptr)                                                      // Si la dirección en memoria a la que apunta conn, NO es nula, es decir, sí es una conexión válida
    {
        mysql_close(conn);                                                    // Se cerrará la conexión MySQL
        conn = nullptr;                                                       // Y ahora conn apuntará a un puntero nulo, esto para reiniciar por completo su estado y dejarlo como nuevo para volver a usarlo
    }

    conn = mysql_init(NULL);                                                  // Ahora, conn almacenará una conexión MySQL inicializada, esto se hace por medio de mysql_init() para inicializar una conexión
    if (conn == nullptr) {return 1;}                                          // En caso de que mysql_init() falle, conn almacenará un puntero inválido, se retornará código de estado 1 -> "Error en la conexión a la base de datos"

    if (validIP(*server))                                                     // Ahora, se verifica que la IP sea válida, si la IP llega a ser válida...
    {
        bool reconnect = 1;                                                   // Sirve para pasarle el parámetro de MYSQL_OPT_RECONNECT a mysql_options() para habilitar el ajuste
        mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);                 // Se le pasa el argumento MYSQL_OPT_RECONNECT como parámetro a mysql_options() para indicar que se desea mantener una conexión que se reconecte en caso de pérdida
        if (!mysql_real_connect(conn, *server, *user, *password,              // Se realizará una conexión al servidor MySQL con las credenciales que se suministraron del archivo de configuración o de la pantalla CONFIGURATION
                                *database, std::atoi(*port), NULL, 0))        // Se debe convertir de valor ASCII a int la variable port, entonces se usa la función atoi(), que significa ascii to int
        {                                                                     // Si el if NO llega a ser exitoso (por eso el signo de exclamación ! antes de la función mysql_real_connect()) entonces ejecutará el siguiente bloque de código
            outQuery = mysql_error(conn) + "\n"s;                             // En outQuery, escribe el error que ocurrió en la conexión, junto con un newline
            mysql_close(conn);                                                // Llama a mysql_close() para cerrar la conexión
            conn = nullptr;                                                   // Declara a conn con un puntero nulo para limpiar su contenido
            return 1;                                                         // Y devuelve código de estado 1 -> "Error en la conexión a la base de datos"
        }
    }
    else {return 127;}                                                        // En caso de que el if de la validación de la IP falle, procederá a retornar código de estado 127 -> "Error | La IP del servidor no existe"

    // Carga la información de las tablas

    while (!nametables.empty()) {nametables.pop_back();}                      // Se procede a vaciar el vector nametables, en caso de que no se encuentre vacío, existe un bucle while que solo para hasta que el vector se encuentre vacío
    std::string nametable = "";                                               // Se vacía también a la variable nametable, ya que posteriormente va a servir como acumulador de los nombres de las tablas en un bucle

    if (sendquery(("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = '"s + (std::string)*database + "';"s).data(), 0, 0) != 0) {return 3;}  // Esta consulta busca la cantidad de tablas de la base de datos y verifica su nombre
    else if (outQuery.empty()) {return 3;}                                    // En caso de que la consulta anterior haya dado un fallo o la respuesta esté vacía, se retorna código de estado 3 -> "Error con el nombre de la base de datos"
    quantables = std::stoi(outQuery);                                         // Si todo salió exitosamente y ninguno de los if anteriores ocurren, la cantidad de tablas se guarda en quantables, stoi() significa string to int

    if (sendquery("SHOW TABLES;", 0, 0) != 0) {return 20;}                    // Se realiza una query para buscar el nombre de las tablas, si por alguna razón esta query falla, se retorna con 20 -> "Error desconocido"
    else if (outQuery.empty()) {return 20;}                                   // Si la respiesta de la query está vacía, también se retorna con 20 -> "Error desconocido"
    std::string strnametables = outQuery;                                     // En caso de que ninguno de los dos if anteriores hayan ocurrido, strnametables guardará todo el string que almacena las tablas

    for (int c = 0; c < (int)strnametables.length(); c++)                     // Este bucle sirve para guardar los nombres de las tablas dentro de un vector, llamado nametables, lo que hará será recorrer cada caracter del string strnametables
    {
        if (strnametables[c] != '\n'){                                        // Si el carácter que está recorriendo en el índice actual del bucle NO es un newline....
            nametable.push_back(strnametables[c]);                            // Guardará el carácter actual en nametable
        }
        else {                                                                // En caso de que el carácter actual SÍ sea un newline...
            nametables.push_back(nametable);                                  // Empujará al vector de los nombres de las tablas, el nombre de la tabla actual que se logró conseguir de todos los carácteres que se guardaron anteriormente
            nametable = "";                                                   // Y ahora, procederá a vaciar al string nametable, para acumular otro nombre de otra tabla
        }
    }

    // Carga cantidad de estudiantes

    if (sendquery(("SELECT COUNT(*) FROM "s + *nameTableEstudiantes).data(), 0, 0 != 0)) {return 6;}      // Se realiza una query para averiguar la cantidad de estudiantes en la base de datos
    else if (outQuery.empty()) {return 6;}                                                                // Si la consulta anterior devuelve un error, o la respuesta está vacía, retorna 6 -> "Error en el nombre de la tabla de los estudiantes"
    quanstudents = std::stoi(outQuery);                                                                   // En caso de que todo haya salido exitosamente, guarda la cantidad de estudiantes en quanstudents usando stoi(), que significa string to int

    // Carga información de las columnas de votos, y también información de los partidos

    if (sendquery(("SELECT * FROM "s + *nameTablePartidos).data(), 0, 0 != 0)) {return 7;}                                                  // Se consulta sobre todos los datos de la tabla de los partidos, para verificar su existencia
    else if (outQuery.empty()) {return 7;}                                                                                                  // Si ocurrió algún error, se retorna 7 -> "Error en el nombre de la tabla de los partidos"

    if (sendquery(("SELECT "s + *nameColumnVotoNombre + " FROM "s + *nameTableEstudiantes).data(), 0, 0) != 0) {return 8;}                  // Se verifica la existencia de la columna del estado del voto del estudiante...
    else if (outQuery.empty()) {return 8;}                                                                                                  // Si hay un error, se retorna 8 -> "Error en el nombre de la columna de los votos de los estudiantes"

    if (sendquery(("SELECT "s + *nameColumnVotosNombre + " FROM "s + *nameTablePartidos).data(), 0, 0) != 0) {return 9;}                    // Se verifica la existencia de la columna del total de los votos de los partidos
    else if (outQuery.empty()) {return 9;}                                                                                                  // Si hay un error, se retorna 9 -> "Error en el nombre de la columna de los votos de los partidos"

    if (sendquery(("SELECT COUNT("s + *nameColumnPartidosNombre + ") FROM "s + *nameTablePartidos).data(), 0,0) != 0) {return 10;}          // Se verifica la existencia de la columna del nombre de los partidos, y la cantidad de partidos que hay
    else if (outQuery.empty()) {return 10;}                                                                                                 // Si hay un error, se retorna 10 -> "Error en el nombre de la columna que almacena el nombre de los partidos"
    quanpartidos = std::stoi(outQuery);                                                                                                     // En caso de que los 2 if anteriores no ocurrieran, la cantidad de partidos se guarda en quanpartidos

    if (sendquery(("SELECT "s + *nameColumnPartidosNombre + " FROM "s + *nameTablePartidos +                                                // Verifico de que la opción a votar nulo se encuentre en la tabla partidos y sea la opción correcta
                   " WHERE "s + *nameColumnPartidosNombre + " = '"s + *nameNuloOpcion + "';").data(),
                  0, 0) != 0)
    {return 11;}                                                                                                                            // En caso de que la opción no sea correcta, retorna 11 -> "Error en el nombre de la opción a votar nulo"
    if (outQuery.empty()) return 11;

    if (sendquery(("SELECT "s + *nameColumnLabsNombre + " FROM "s + *nameTableEstudiantes).data(), 0, 0) != 0) {return 8;}                  // Se verifica la existencia de la columna del laboratorio del estudiante...
    else if (outQuery.empty()) {return 12;}                                                                                                 // Si hay un error, se retorna 12 -> "Error en el nombre de la columna de los laboratorios de los estudiantes"

    if (sendquery(("SELECT "s + *nameColumnPartidosNombre + " FROM "s + *nameTablePartidos).data(), 0, 0) != 0) {return 20;}                // Se consulta el nombre de los partidos
    else if (outQuery.empty()) {return 20;}                                                                                                 // Si hay un error, se retorna 20 -> "Error desconocido"
    std::string strnamepartidos = outQuery;                                                                                                 // En caso de que los 2 if anteriores no ocurrieran, el nombre de los partidos se guardan en strnamepartidos

    nullOption = false;                                                       // Verifica si se ha encontrado la opción nula entre los partidos

    while (!namepartidos.empty()) namepartidos.pop_back();                    // Si el vector que almacena a los partidos NO se encuentra vacío, lo vacía, ya que se volverá a llenar con el siguiente bucle for

    std::string namepartido     = "";                                         // Se nombra a namepartido como string vacío, ya que servirá para acumular los carácteres de los nombres de los partidos en el bucle
    for (int c = 0; c < (int)strnamepartidos.length(); c++)                   // Este bucle servirá para recorrer el string que contiene el nombre de todos los partidos carácter por carácter
    {
        if (strnamepartidos[c] != '\n')                                       // Si el carácter NO es un newline...
        {
            namepartido.push_back(strnamepartidos[c]);                          // Procederá a guardarlo en namepartido
        }
        else                                                                  // Si el carácter SÍ es un newline
        {
            if (namepartido == *nameNuloOpcion)                               // Verifica si el partido es la opción nula, en caso de que SÍ lo sea...
            {
                namepartido = "";                                             // Vacía a namepartido sin meterlo al vector namepartidos, ya que la opción nula no se debe meter en ese vector
                quanpartidos--;                                               // Se disminuye la cantidad de quanpartidos, ya que antes contó también al partido nulo
                nullOption = true;                                            // Se declara nullOption como true, verificando así el partido nulo
            }
            else                                                              // En caso de que el partido NO es la opción nula...
            {
                namepartidos.push_back(namepartido);                          // Mete ese partido al vector namepartidos
                namepartido = "";                                             // Vacía a namepartido para recibir los siguientes carácteres del siguiente partido
            }
        }
    }

    if (((std::string)*labName).empty()) {return -1;}

    return 0;               // Retorna 0, código de estado exitoso
}

int verifyConnection(Screen &currentScreen)
{
    if (currentScreen == CONFIGURATION) return 0;                             // Si la pantalla actual es CONFIGURATION no se ejecutará el resto de la función, ya que es innecesario, por que significa que obviamente se está configurando algo

    timer = 0;                                                                // Reinicia el temporizador que llama a esta función cada 15 segundos
    if (conn != nullptr)                                                      // Si conn es una conexión válida...
    {
        if (mysql_ping(conn) != 0)                                            // Si hubo un error al hacer ping a la base de datos, entonces...
        {
            mysql_close(conn);                                                // Se cerrará la conexión MySQL
            conn = nullptr;                                                   // Y ahora conn apuntará a un puntero nulo, esto para reiniciar por completo su estado y dejarlo como nuevo para volver a usarlo
        }
        else return 0;                                                        // Si el ping sale exitoso, solo retorna la función, no hay nada más por hacer ya que el ping verifica la conexión a la base de datos
    }

    conn = mysql_init(NULL);                                                  // Ahora, conn almacenará una conexión MySQL inicializada, esto se hace por medio de mysql_init() para inicializar una conexión
    if (conn == nullptr) {return 1;}                                          // En caso de que mysql_init() falle, conn almacenará un puntero inválido, se retornará código de estado 1 -> "Error en la conexión a la base de datos"

    if (validIP(*server))                                                     // Ahora, se verifica que la IP sea válida, si la IP llega a ser válida...
    {
        bool reconnect = 1;                                                   // Sirve para pasarle el parámetro de MYSQL_OPT_RECONNECT a mysql_options() para habilitar el ajuste
        mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);                 // Se le pasa el argumento MYSQL_OPT_RECONNECT como parámetro a mysql_options() para indicar que se desea mantener una conexión que se reconecte en caso de pérdida
        if (!mysql_real_connect(conn, *server, *user, *password,              // Se realizará una conexión al servidor MySQL con las credenciales que se suministraron del archivo de configuración o de la pantalla CONFIGURATION
                                *database, std::atoi(*port), NULL, 0))        // Se debe convertir de valor ASCII a int la variable port, entonces se usa la función atoi(), que significa ascii to int
        {                                                                     // Si el if NO llega a ser exitoso (por eso el signo de exclamación ! antes de la función mysql_real_connect()) entonces ejecutará el siguiente bloque de código
            outQuery = mysql_error(conn) + "\n"s;                             // En outQuery, escribe el error que ocurrió en la conexión, junto con un newline
            mysql_close(conn);                                                // Llama a mysql_close() para cerrar la conexión
            conn = nullptr;                                                   // Declara a conn con un puntero nulo para limpiar su contenido
            statusCodeUpdating = updateData();                                // Actualiza los datos de la conexión con updateData(), principalmente para buscar el error específico que haya ocurrido en la conexión
            objectCreation();                                                 // Realiza la creación de los objetos del programa con objectCreation()
            currentScreen = CONFIGURATION;                                    // Cambia a la pantalla CONFIGURATION para verificar los ajustes del programa
            loadConfig();
            return 1;                                                         // Retorna 1, código de estado fallido
        }
    }
    else
    {
        statusCodeUpdating = updateData();                                    // Actualiza los datos de la conexión con updateData(), principalmente para buscar el error específico que haya ocurrido en la conexión
        objectCreation();                                                     // Realiza la creación de los objetos del programa con objectCreation()
        currentScreen = CONFIGURATION;                                        // Cambia a la pantalla CONFIGURATION para verificar los ajustes del programa
        return 1;                                                             // Retorna 1, código de estado fallido
    }
    return 0;                                                                 // Retorna 0 como código de estado exitoso
}
