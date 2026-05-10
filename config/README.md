# Acerca del archivo de configuración

El nombre del archivo es .config, se encuentra en la carpeta del programa, si no se ve en su explorador de archivos, es por que es lo ideal, al comenzar con un punto el archivo se "esconde" entre los archivos.
Si está en Linux, puede ejecutar el comando ls -la en la carpeta del programa para buscar entre los archivos "escondidos".
Lo ideal sería que el archivo tenga algún tipo de encriptación, pero aún no he decidido incorporarla hasta que ya dominen el código en su mayor totalidad.

```
[Credentials]           │ [Credentials] Es la sección del archivo de configuración en la que se configuran los valores para la autenticación a la base de datos, como la contraseña del panel de administración
server=127.0.0.0        │ server: es la dirección IP del servidor de la base de datos, es 127.0.0.0 de manera predeterminada, ya que 127.0.0.0 apunta a la propia computadora en la que se esté ejecutando
user=root               │ user: es el usuario a autenticar al servidor de la base de datos
password=0th3r@N0n      │ password: es la contraseña a usar para la autenticación de la base de datos
database=Votaciones     │ database: es el nombre de la base de datos
port=3306               │ port: es el puerto de la base de datos
admPassword=1234        │ admPassword: es la contraseña del panel de administración del programa

[Extra]                                 │ [Extra] Es la sección en la que se encuentran configuraciones poco probables que se modifiquen, es principalmente si ocurren cambios en los nombres de los recursos que se necesitan de la base de datos
nameColumnPartidosNombre=Nombre         │ nameColumnPartidosNombre: es el nombre de la columna en la tabla de los partidos que contenga el nombre de los partidos, ejemplo: BLAZE se encuentra en la columna Nombre, de la tabla Partidos
nameNuloOpcion=NULO                     │ nameNuloOpcion: es el nombre del partido que se usará para votar nulo, ejemplo: NULO va a ser el nombre del partido que servirá para votar nulo
nameColumnVotosNombre=Votos             │ nameColumnVotosNombre: es el nombre de la columna que deba de encontrarse en la tabla de los partidos, que especifique los votos de los partidos, ejemplo: En la columna Votos de los partidos, veo que BLAZE tiene más votos
nameColumnVotoNombre=Voto               │ nameColumnVotoNombre: es el nombre de la columna que deba de encontrarse en la tabla de los estudiantes, que especifique si un estudiante votó o no, ejemplo: En la columna Voto, veo que Ronald sí votó
nameTableEstudiantes=Estudiantes        │ nameTableEstudiantes: es el nombre de la tabla que contenga la información de los estudiantes
nameTablePartidos=Partidos              │ nameTablePartidos: es el nombre de la tabla que contenga información de los partidos

[Paths]                                                 │ [Paths] Es la sección que tiene las rutas de los fonts, además del nombre del archivo de informe sobre la cantidad de votos de cada partido
pathProgramFont=./fonts/GoMonoNerdFont-Regular.ttf      │ pathProgramFont: es la ruta que contiene el font del programa
pathPdfFont=./fonts/LibertinusMono-Regular.ttf          │ pathPdfFont: es la ruta que contiene el font a usar en el pdf de informe
informeName=Informe VOCA Votaciones 2026.pdf            │ informeName: es el nombre del archivo PDF de informe
```
