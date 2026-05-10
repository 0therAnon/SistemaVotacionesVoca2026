#include "globals.hpp"

// ── Configuration ─────────────────────────────────────────────────────────────
/*

!!! Leer la función configureData() y loadConfig() en el archivo ./config/config.cpp para mayor entendimiento

El vector configurations almacenará los valores de cada argumento del archivo de configuración, así que por ejemplo, si una línea del archivo de configuración comienza por "server="
el cual en el vector configurations es índice cero, el programa empezará a leer todo lo que viene después del símbolo de igual "=" y terminará cuando detecte un newline "\n"
y eso que haya leído, lo reemplazará en el vector en el índice cero, que es donde se encuentra la palabra "server=", por ejemplo:

en el archivo de configuración existe una línea igual a: server=127.0.0.1

lo que ocurre sería:

buscar configurations[0] en el archivo de configuración...
configurations[0] es igual a "server="
si se encuentra una línea que inicie con "server=" y lo que siga después sea una IP válida como 127.0.0.1 o cualquiera:
  configurations[0] = 127.0.0.1
en caso contrario
  configurations[0] no fue encontrado, lanzar error
fin
continuar con configurations[1]... y así hasta completar a todos

*/

std::vector<char*> configurations = {
    strdup("server="), strdup("user="), strdup("password="), strdup("database="),
    strdup("port="), strdup("admPassword="), strdup("labName="), strdup("nameTableEstudiantes="),
    strdup("nameTablePartidos="), strdup("nameColumnVotoNombre="),
    strdup("nameColumnVotosNombre="), strdup("nameColumnPartidosNombre="),
    strdup("nameColumnNuloPartido="), strdup("pathProgramFont="),
    strdup("pathLogsFont="), strdup("pathPdfFont="),
    strdup("informeName=")
};

// Todos estos son punteros que apuntan a cierto índice del vector configurations
char** admPassword             = &configurations[5];    // admPassword apunta al índice 5, el cual es el de la contraseña del panel de admistración
char** password                = &configurations[2];    // password apunta al índice 2, el cual es igual a la contraseña que se utiliza en la conexión del programa a la la base de datos
char** database                = &configurations[3];    // database apunta al índice 3, el cual es el nombre de la base de datos que se utiliza en la conexión del programa a la base de datos
char** labName                 = &configurations[6];    // labName apunta al índice 6, el cual es el nombre del laboratorio en el que se encuentra la computadora actual
char** server                  = &configurations[0];    // server apunta al índice 0, el cual es la IP del servidor MySQL al cual se realiza la conexión del programa a la base de datos
char** user                    = &configurations[1];    // user apunta al índice 1, el cual es el nombre de usuario que se utiliza en la conexión del programa a la base de datos
char** port                    = &configurations[4];    // port apunta al índice 4, el cual es el número de puerto que se utiliza en la conexión del programa a la base de datos

char** nameColumnPartidosNombre = &configurations[11];  // Estos valores son valores extra que
char** nameColumnNuloPartido    = &configurations[12];  // decidí agregar, ya que no sé si
char** nameColumnVotosNombre    = &configurations[10];   // en algún momento algun valor de la
char** nameColumnVotoNombre     = &configurations[9];   // base de datos puede llegar a cambiar
char** nameTableEstudiantes     = &configurations[7];   // como nombres de columnas o tablas de
char** nameTablePartidos        = &configurations[8];   // la base de datos en caso de usar una distinta

char** pathProgramFont          = &configurations[13];  // Estos valores son rutas a los fonts
char** pathLogsFont             = &configurations[14];  // que el sistema necesita que son
char** pathPdfFont              = &configurations[15];  // necesarios para el programa y el PDF de informe
char** informeName              = &configurations[16];  // como también la definición del nombre de salida del PDF

// ── MySQL ─────────────────────────────────────────────────────────────────────
MYSQL_RES* res  = nullptr;    // Variable que almacena un puntero a la respuesta de las queries a la base de datos MySQL
MYSQL_ROW  row;               // Variable que almacena las filas en la respuesta de las queries a la base de datos MySQL
MYSQL*     auth = nullptr;    // Variable que almacenará un puntero a la conexión a la base de datos MySQL que se usará en la pestaña "Terminal" del panel de administración
MYSQL*     conn = nullptr;    // Variable que almacenará un puntero a la conexión a la base de datos MySQL primaria del programa

// ── Colors ────────────────────────────────────────────────────────────────────
// Los valores de los colores se definen en RGB y Alpha (RGBA) | Alpha es la transparencia del color, si Alpha es 0, es transparente, si es 255, es totalmente sólido
// Por ejemplo, el VOCAVERDE tiene un valor de ROJO = 0
Color VOCAVERDE         = {245, 235, 210, 0};  // Beige pastel (botón normal)
Color VOCAVERDESUAVE    = {250, 244, 228, 0};  // Beige muy suave (tablas normal)
Color VOCAAMARILLO      = {220, 200, 160, 0};  // Beige más cálido (hover)
Color VOCAAMARILLOSUAVE = {250, 244, 228, 0};  // Beige suave
Color VOCADORADO        = {220, 200, 160, 0};  // Beige cálido (seleccionado)
Color VOCADORADOSUAVE   = {245, 235, 210, 0};  // Beige pastel
Color DORADO_BORDE      = {212, 175, 55, 0};   // Dorado para los bordes de todos los botones
Color NEGRO             = {0, 0, 0, 0};        // Color negro
Color GRIS              = {120, 120, 120, 0};     // Color gris
Color BLANCO            = {255, 255, 255, 0};  // Color blanco
Color COLORTEXTO        = NEGRO;               // Color negro para el texto

// ── File / query state ────────────────────────────────────────────────────────
std::ofstream configFile;   // Variable que servirá para almacenar el contenido del archivo de configuración
std::vector<std::vector<std::string>> vecRows;    // Vector bidimensional que almacenará las filas de las respuestas de las queries SQL, necesita un vector adentro el cual será el de las columnas
std::vector<std::string> logCommands;             // Vector que almacenará el historial de todos los comandos digitados en la pestaña "Terminal" del panel del administrador
std::vector<std::string> vecColumns;              // Vector que se encontrará dentro del vector bidimensional vecRows, este sirve para tener las columnas de la fila
std::vector<std::string> namepartidos;            // Vector que almacenará el nombre de los partidos cargados de la base de datos MySQL
std::vector<std::string> nametables;              // Vector que almacenará el nombre de las tablas cargadas de la base de datos MySQL
std::vector<std::string> lastLogs;                // Vector que almacenará los ultimos valores de cada log de adminButtons, se almacenan en este registro para que no se pierdan cuando se llama objectCreation()
std::vector<Color*>      colorsVec;               // Vector que almacenará todos los colores del programa
std::vector<double>      percentages;             // Vector que almacenará los porcentajes de los votos de cada partido, se usa principalmente en la función statistics() que se encuentra en ./ui/drawing.cpp
std::string oldTableSelected  = "";     // Variable que almacena la última tabla seleccionada, sirve principalmente para actualizar las columnas comparando la tabla actual con la vieja en la función drawcolumns() dentro de ./ui/drawing.cpp
std::string backupRetString   = "";     // Variable que almacena el string de respuesta del backup ejecutado
std::string partidoSelected   = "";     // Variable que almacena el partido seleccionado por el estudiante en la pantalla VOTATION
std::string configSelected    = "";     // Variable que almacena la pestaña actual de la pantalla CONFIGURATION
std::string pdfErrorString    = "";     // Variable que almacena el string de error en caso de un error con la función inform() que se encuentra en el archivo ./reports/inform.cpp
std::string tableSelected     = "";     // Variable que almacena la tabla seleccionada actualmente
std::string adminSelected     = "";     // Variable que almacena la pestaña actual del panel de administración
std::string butnames[7]       = {};     // Lista que almacena 7 valores, los cuales son los nombres de las pestañas de la ventana de administración ("Consultar", "Agregar"...)
std::string oldSelected       = "";     // Variable que almacena la última pestaña seleccionada en el panel de administración, se compara luego con el botón actual, en la función drawcolumns, sirve para detectar cambios de pestaña
std::string outQuery          = "";     // Variable que almacena el resultado de la última query ejecutada con la función sendquery(), esta función se encuentra en ./db/database.cpp
std::string oldbar            = "";     // Variable que almacena la última barra presionada, su uso se encuentra en la función ./ui/input.hpp

// ── Raylib / UI state ─────────────────────────────────────────────────────────
Font   fontTtf;             // Guardará el font del programa para funciones de la parte gráfica
Font   monoTtf;             // Guardará el font del programa para funciones de la parte gráfica
float explorarSquare[4];    // Lista que guardará ubicacion en el eje x, ubicacion en el eje y, tamaño de ancho y tamaño de alto, para un rectángulo que se usará de fondo en la pestaña "Explorar"
float mediumFontSize;       // Tamaño del font del programa mediano
float littleFontSize;       // Tamaño del font del programa pequeño
float configPanel[4];       // Lista separada para el panel de configuración, permite ajustar su posición independientemente del panel de administración
float adminPanel[4];        // Lista que guardará ubicacion en el eje x, ubicacion en el eje y, tamaño de ancho y tamaño de alto, para un rectángulo que se usará de fondo para todo el panel de administración
float outSquare[4];         // Lista que guardará ubicacion en el eje x, ubicacion en el eje y, tamaño de ancho y tamaño de alto, para un rectángulo que se usará de fondo para el resultado de las pestañas en el panel de administración
float screenHeight;         // Valor del tamaño de alto de la ventana
float screenWidth;          // Valor del tamaño de ancho de la ventana
float fontSize;             // Tamaño del font del programa

bool adminAuthenticated = false;  // Verifica si el administrador fue autenticado en la pestaña "Terminal" del panel de administración
bool backupReleased     = false;  // Verifica si se realizó un backup
bool pdfRandomError     = false;  // Verifica si hubo un error en la creación de los PDFs
bool pdfFontError       = false;  // Verifica si hubo un error con la carga del font al PDF
bool successReset       = false;  // Verifica si se reinició la base de datos correctamente
bool alphaIsZero        = false;  // Verifica si la transparencia de todos los colores es cero (para la función transition())
bool alphaIsFull        = false;  // Verifica si la transparencia de todos los colores es 255 (para la función transition())
bool errorReset         = false;  // Verifica si huno un error al reiniciar la base de datos
bool tabRestart         = false;  // Sirve como activador en las funciones de moverse entre barras de entrada con la tecla TAB en el panel de administración
bool nullOption         = false;  // Verifica si existe un partido NULO en las tablas existentes de la base de datos MySQL
bool darkMode           = false;  // Verifica si el modo de usuario del sistema es modo oscuro, en caso de que sea falso significa que es modo claro, el modo claro es el predeterminado
bool pdfError           = false;  // Verifica si hubo un error con la creación del PDF
bool showBeam           = true;   // Verifica si la línea de la barra de entrada tiene que mostrarse o no, para dar el efecto de que la línea está parpaedeando

int statusCodeUpdating  = 100;  // Es la variable que almacenará el número de retorno de la función updateData(), cada número especifica un error distinto en la función o si todo funcionó correctamente
int intentosRestantes   = 3;    // Verifica cuántas veces se intentó autenticar el administrador
int statusCodeConfig    = 0;    // Verifica el código de estado de la configuración
int columnSelected      = 0;    // Almacena el id de la columna seleccionada
int framesCounter       = 0;    // Almacena la cantidad de tiempo en FPS, aumenta por medio de un bucle, mientras más aumente más cantidad de tiempo ha pasado
int stopBackspace       = 0;    // Es parecido a la variable beam, pero este pone un límite al botón de borrar y suprimir, para que no borre demasiadas veces
int maxBarLenght        = 0;    // Largo máximo de texto soportado para ser digitado en una barra de entrada
int quanstudents        = 0;    // Cantidad de estudiantes
int quanpartidos        = 0;    // Cantidad de partidos
int quancolumns         = 0;    // Cantidad de columnas
int quantables          = 0;    // Almacena la cantidad de tablas de la base de datos MySQL
int stopArrows          = 0;    // Lo mismo que la variable beam, y stopBackspace, pero hace de que no vaya a la izq. o der. demasiadas veces
int maxLenName          = 0;    // Largo máximo de un nombre o texto
int nlSpacing           = 16;   // Espaciado entre newlines
int stopCtrls           = 0;    // Igual a la variable stopArrows, pero ahora con las opciones de pegado
int inputpos            = 0;    // Posición actual con el cursor en una barra de entrada
int opcSize             = 0;    // Almacena la cantidad de las opciones de la lista desplegable en la pestaña "Actualizar", recomiendo probar lo que ocurre cuando no se usa para que se entienda mejor su función
int logpos              = 0;    // Posición en el registro de logCommands{}
int tabCnt              = 0;    // Variable usada en la función de TAB para contar cuantos TAB lleva la función
int value               = 0;    // Variable de valores de la función sendquery()
int beam                = 0;    // Pone un límite a las repeticiones por FPS de la barra de entrada de inputfunc(), esto es como un contador
int add                 = 0;    // Variable usada en logfunction() para el scroll

// ── jmp_buf ───────────────────────────────────────────────────────────────────
jmp_buf env;    // Variable usada para almacenar una dirección en la memoria a la que el flujo del programa regresará en caso de errores en la librería HPDF

// ── UI object collections ─────────────────────────────────────────────────────
std::vector<std::unique_ptr<nxyxys>> adminObj = {}; // Almacena punteros a todos los objetos cuya clase padre sea nxyxys, es decir, todos
std::vector<sqlobject*> partidosVec           = {}; // Almacena todos los punteros a objetos de tipo sqlobject que sean especificamente de los partidos cargados de la base de datos
std::vector<sqlobject*> opcionesAct           = {}; // Almacena todos los punteros a todas las opciones de la pestaña actualizar, osea, todas las columnas de la tabla en la que se encuentre el usuario
std::vector<button*> configbuttons            = {}; // Almacena todos los punteros a todos los botones que actúan como pestañas de la pantalla CONFIGURATION
std::vector<sqlobject*> tablesVec             = {}; // Almacena todos los punteros a todas las tablas cargadas desde MySQL
std::vector<inputBar*> termBars               = {}; // Almacena todos los punteros a todas las barras de la pestaña Terminal de la pantalla ADMINMENU
std::vector<inputBar*> extraBars              = {}; // Almacena todos los punteros a todas las barras de la pestaña "Extra" de la pantalla CONFIGURATION
std::vector<inputBar*> pathBars               = {}; // Almacena todos los punteros a todas las barras de la pestaña "Paths" de la pantalla CONFIGURATION
std::vector<column*> columnsVec               = {}; // Almacena todos los punteros de las columnas cargadas desde la base de datos
std::vector<button*> adminButtons             = {}; // Almacena todos punteros de los botones de la pantalla ADMINMENU

// ── Raw pointer aliases ───────────────────────────────────────────────────────
// Declaración de los punteros de los objetos, estos punteros necesito que sean globales ya que varios se usan externamente de la función main
sqlobject* opcSelectedPtr       = nullptr;
sqlobject* sinVotarPtr          = nullptr;
inputBar*  barAdminTerminalPtr  = nullptr;
inputBar*  admPasswordBarPtr    = nullptr;
inputBar*  adminTerminalPtr     = nullptr;
inputBar*  terminalBarPtr       = nullptr;
inputBar*  labNameBarPtr        = nullptr;
inputBar*  cedulaBarPtr         = nullptr;
inputBar*  actBarPtr            = nullptr;
button*    cambiarFrontendPtr   = nullptr;
button*    enterConfigPtr       = nullptr;
button*    saveConfigPtr        = nullptr;
button*    resetDataPtr         = nullptr;
button*    continuarPtr         = nullptr;
button*    opcionActPtr         = nullptr;
button*    resTogglePtr         = nullptr;
button*    exitAdminPtr         = nullptr;
button*    regresarPtr          = nullptr;
button*    informePtr           = nullptr;
button*    refreshPtr           = nullptr;
button*    backupPtr            = nullptr;
button*    cedulaPtr            = nullptr;
button*    votarPtr             = nullptr;
