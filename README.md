# Compiladores Automáticos
Primero que nada, para poder ejecutar el programa, actualmente hay 2 compiladores automáticos:

* ./compiler.sh: Se encarga de específicamente compilar el programa en el entorno de Linux.
* ./exe_compiler.sh: Se encarga de compilar específicamente un ejecutable para windows, osea un archivo .exe, pero desde linux, además de cargar sus dependencias y archivos importantes, se comprime todo en un rar para que solo deba descomprimirse y ejecutarse.
* Dependencias: exe_compiler tiene 2 dependencias muy importantes para poder funcionar, tener mingw para c++ y rar instalado en linux.

Lo ideal sería compilar el código ejecutando compiler.sh para debuggear en el entorno de Linux, y para ya usarlo el día de las votaciones ejecutar exe_compiler.sh

# Organización del Proyecto
A continuación, se explicarán las carpetas y archivos más importantes.

## main.cpp y globals.cpp
Son los archivos principales del programa:

* main.cpp: es el archivo con el flujo principal del programa, este es el que se encarga de llamar a las funciones y subfunciones de los demás archivos.
* globals.cpp: este archivo posee las variables globales del programa, son las variables más importantes que se llaman entre las funciones y subfunciones.

## build/
Posee los archivos de tipo objeto (.o) y archivos binarios como comprimidos, aquí se guardan los compilados de resultado de compiler.sh y el .rar resultante de exe_compiler.sh

* linux: esta carpeta contiene archivos necesarios para la ejecución en linux.
* deps: es la carpeta con las dependencias principales del programa, como las librerías mysql, libHaru, Raylib, etc, dependiendo del sistema operativo.
* windows: posee los dlls y archivos necesarios para la ejecución en windows.

## config/
Esta carpeta lo que contiene son las funciones relacionadas a la configuración del programa, como la carga de los valores del archivo de configuración, el cual es un archivo oculto en la carpeta, llamado ".config", el cual tiene este formato:
```
+-------------------------------------------------------+
|  [Credentials]                                        >-+
|  server=127.0.0.0                                       |
|  user=root                                              |
|  password=0th3r@N0n                                     +---- Credenciales para la conexión de la base de datos
|  database=Votaciones                                    |
|  port=3306                                              |
|  admPassword=1234                                     >-+
|
|  [Extra]
|  nameColumnPartidosNombre=Nombre                      >-+
|  nameNuloOpcion=NULO                             |
|  nameColumnVotosNombre=Votos                            +---- Valores de la base de datos como columnas o tablas
|  nameColumnVotoNombre=Voto                              |
|  nameTableEstudiantes=Estudiantes                       |
|  nameTablePartidos=Partidos                           >-+
|
|  [Paths]
|  pathProgramFont=./fonts/GoMonoNerdFont-Regular.ttf   >-+
|  pathPdfFont=./fonts/LibertinusMono-Regular.ttf         +--- Rutas del sistema para fuentes o nombre del PDF
|  informeName=Informe VOCA Votaciones 2026.pdf         >-+
+-------------------------------------------------------+
```
Funciones en ./config/config.cpp:

* configureData(): Se encarga de verificar si existe el archivo de configuración y cargar los valores al programa.
* loadConfig(): En caso de que la función anterior falle, el programa enviará al usuario a la pantalla de configuración para no introducir toda la configuración variable por variable.

## db/
Esta carpeta contiene las dos funciones relacionadas a la base de datos en ./db/database.cpp:

* sendquery(): Se encarga de enviar las queries a la base de datos, utiliza la conexión primaria del programa ("conn") y almacena el resultado en una variable global llamada "outQuery".
* updateData(): Su propósito es actualizar información de la base de datos y cargarla al programa; sirve para verificar si el archivo de configuración está correctamente configurado.

## fonts/
Esta carpeta almacena las fuentes de las letras, sin ellas, las letras del programa y del PDF de informe se verían feas y poco adecuadas, solo posee dos archivos .tff

## platform/
Se encarga de la funcionalidad multiplataforma. Existe mucho código que necesita verificar si se compilará para windows o para linux, además de ser compatibles con UTF-8, UTF-16 y UTF-32. (Explicación detallada en ./platform/README.txt).

* ./platform/clipboard.cpp: clipboard() - Carga el contenido del portapapeles desde Linux o Windows.
* ./platform/encoding.cpp: UTF8ToUTF32() / UTF32ToUTF8() / UTF16ToUTF32() - Se encargan de traducir las codificaciones de texto.
* ./platform/validIP: validIP() - Verifica si una IP es válida para chequeos de configuración y conexiones.
* ./platform/pty.cpp: to_wstring() / ptyfunc() - Traducción a wstring para la API de windows y creación de una pseudoterminal (pty) para la pestaña "Terminal".
* ./platform/compat.hpp: Permite que los conflictos de funciones como DrawText o CloseWindow de Windows no choquen con raylib.

## reports/
Carpeta para la creación de informes (PDF de salida con información de los partidos) en ./reports/inform.cpp:

* inform(): Función que crea el PDF acerca de la información de los partidos.
* error_handler(): En caso de error con la creación del PDF, evita que el programa explote.

## screens/
Posee todas las pantallas del programa (backend y frontend). Cada archivo tiene funciones que terminan en "Update" (backend) y "Draw" (frontend).
Existen 7 pantallas:

  - LOGO: Inicio del programa y llamada a funciones de configuración y carga de datos.
  - CONFIGURATION: Pantalla de error para arreglar inconvenientes con la conexión, tablas, columnas o rutas de fuentes.
  - MAINMENU: Menú principal tras el LOGO; permite digitar la cédula o acceder al panel de administración con contraseña.
  - ADMINMENU: Panel de administración con 7 pestañas:
    * Consultar: Realizar consultas.
    * Agregar: Agregar valores.
    * Actualizar: Cambiar valores.
    * Borrar: Borrar valores.
    * Explorar: Explorador de datos.
    * Resultados: Ver resultados.
    * Terminal: Uso de comandos.

      Este panel de administración, también posee 3 botones extra:

        - Configuración: Entra a la pantalla CONFIGURATION en caso de que se necesite configurar algo
        - Actualizar: En caso de que hayan datos que deban ser actualizados
        - Salir: Sale del panel de administración
  - VOTATION: Pantalla de votación.
  - CONFIRMATION: Pantalla para confirmar el estudiante, y también sirve para confirmar el voto.
  - ENDING: Pantalla de despedida y agradecimiento

## ui/
Su contenido son principalmente funciones relacionadas a agarrar datos del backend y mostrarlas en el frontend:

* ./ui/drawing.cpp
  * logfunction() - Es la función que se encarga de que en cada pestaña del panel de administración se pueda ver la respuesta, por ejemplo una consulta en la pestaña "Consular" aparece
                  la respuesta en un cuadro negro abajo, como también en "Agregar", "Actualizar" y "Borrar"
  * centertext() - Como dice su nombre, se encarga de centrar el texto en la pantalla o en una ubicación en el eje x, no tiene mucha ciencia
  * drawcolumns() - Es de las funciones más importantes, siendo sincero cometí una atrocidad, tuve que mezclar un poco del backend con el frontend con esta función para poder conseguir lo que hace, esta
                  función se encarga de recibir los datos de las columnas que se ingresen en sus respectivas barras, y también si se cambia la tabla en la que se encuentra en el panel de administración,
                  también cambiarán las columnas de la respectiva tabla, esta función se usa en el panel de administración para mostrar el nombre de las columnas y los datos que se han introducido
  * shortmessage() - Se encarga de mostrar un mensaje breve en el frontend del programa
  * statistics() - Muestra la gráfica en la pestaña Resultados del panel de administración
  * alert() - Función que mostrará una alerta antes de ejecutar una query de la pestaña Agregar, Actualizar o Borrar en el panel de administración

* ./ui/input.hpp:
  * isPressed() - Verifica si un objeto ha sido presionado (devuelve estado 3), tiene el cursor encima y fue presionado (devuelve estado 2), el cursor está en una posición fuera del objeto pero
                fue presionado (devuelve estado 1), o definitivamente el usuario no lo ha presionado pero puede que el cursor esté encima del objeto o no, solo que mientras no lo presione no cambiará
                de estado (estado 0)
  * GetCursorFromMouseClick() - Busca la posición en la que el cursor presionó la barra en su eje x, para posicionarlo en la barra detrás o delante de ciertas letras según la posición del cursor
  * inputfunc() - Es la función que recibe datos del teclado a partir de una barra de entrada, acepta Ctrl+V, HOME, DELETE, BACKSPACE (obviamente), END, y teclas de flechas, lo único que no pienso implementar: Ctrl+C

* ./ui/objects.cpp:
  * objectCreation() - Es la función que se encarga de la creación de todos los objetos en el programa, y si las funciones de configuración y conexión a la base de datos salen exitosas, también configura los objetos
                     relacionados a la base de datos, como columnas, tablas y los partidos

* widgets.hpp - No es una función, pero se encarga de la creación de las clases para los objetos, lo cual lo hace un archivo muy importante de mencionar

> [!IMPORTANT]
> A la hora de modificar el código, sugiero NO usar librerías extra o poco conocidas
