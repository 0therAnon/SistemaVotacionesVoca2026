# Este compilador lo que hace es específicamente compilar para dar como resultado un archivo .exe ejecutable de windows

echo -e "\n\
Instalar por favor los siguientes paquetes para realizar correctamente la compilación multiplataforma:\n\
  libpng-dev
  libmysqlclient-dev
  libssl-dev
  zlib1g-dev
  libgl-dev
  mingw-w64-gcc
  mingw-w64-crt
  mingw-w64-binutils
  mingw-w64-winpthreads"


which x86_64-w64-mingw32-g++ &>/dev/null

if [ $? -ne 0 ]; then
  echo "\nNo se encontró MinGW para realizar el compilado multiplataforma, por favor instálelo"
  exit 1
fi

# Compilado en mínima optimización

# echo -e "\nCompilando en modo debug...\n" && cmd="x86_64-w64-mingw32-g++ -Wall -g -O0 -std=c++20"

# echo -e "Compilado en modo release (máxima optimización) (el modo release es el modo en el que el ejecutable se compilará para su uso real, es más rápido y sirve para probar la velocidad real del ejecutable):"

echo -e "\nCompilando en modo release...\n\n" && cmd="/bin/x86_64-w64-mingw32-g++ -s -DNDEBUG -O3 -std=c++20"

# ── Icono del ejecutable (.exe) ──────────────────────────────────────────────
# Compila el archivo de recursos icon.rc (que apunta a assets/vocaLogo.ico) a un
# objeto .o usando windres de MinGW. Ese objeto se enlaza junto al programa para
# que el .exe muestre el icono en el explorador de archivos de Windows.
# Requiere: assets/vocaLogo.ico (convertir vocaLogo.png a .ico con ImageMagick) y
# el archivo icon.rc en la raíz del proyecto.
RES_OBJ=""
if [ -f "./icon.rc" ] && [ -f "./assets/vocaLogo.ico" ]; then
  echo -e "\nCompilando el icono del ejecutable con windres..."
  x86_64-w64-mingw32-windres ./icon.rc -O coff -o ./icon.res.o
  if [ $? -eq 0 ]; then
    RES_OBJ="./icon.res.o"
    echo "Icono del ejecutable compilado correctamente"
  else
    echo "Advertencia: no se pudo compilar el icono, se compilará el .exe sin icono incrustado"
  fi
else
  echo -e "\nAviso: no se encontró ./icon.rc o ./assets/vocaLogo.ico; el .exe se compilará sin icono incrustado"
fi

# Añade el resto de argumentos para el compilador:

cmd=$cmd" ./*/*.cpp ./*.cpp "
cmd=$cmd" $RES_OBJ "
cmd=$cmd" -I ./build/deps/windows -I ./build/deps/windows/lib/ -I ./build/deps/windows/mariadb/ -I ./build/deps/windows/mysql/ -I ./build/deps/windows/src/ -I ./config/ -I ./db/ -I ./platform/ -I ./reports/ -I ./screens/ -I ./ui/"
cmd=$cmd" -L ./build/deps/windows/lib/ -L ./build/deps/windows -L ./build/deps/windows/mariadb/ -L ./build/deps/windows/mysql/ -L ./build/deps/windows/src/ -L ./config/ -L ./db/ -L ./platform/ -L ./reports/ -L ./screens/ -L ./ui/ "
cmd=$cmd" -static"
cmd=$cmd" -llibmariadb -lmysqlclient -lhpdf -llibpng16 -lzlib -llibcrypto_static -llibssl_static -lraylib"
cmd=$cmd" -lGlU32 -lOpenGL32 -lws2_32 -lgdi32 -lwinmm -lkernel32 -lm"
cmd=$cmd" -o ./build/bin/windows/main.exe"

echo -e "\nComando ejecutado:\n\n$cmd\n"                                                                                                     # Muestra el comando para compilar en la pantalla
$cmd                                                # Ejecuta el comando para compilar | Es normal si aparecen advertencias en morado, pero si hay un error, aparecerá en rojo, y no logrará concluir la compilación

rm -f ./icon.res.o 2>/dev/null    # Limpia el objeto del icono tras compilar

# Darle permisos de ejecución al archivo del código compilado para poder ejecutarse | chmod es un comando que permite otorgar permisos de lectura, escritura y ejecución, con esto, le doy permisos de ejecución

if  [ $? -eq 0 ]; then                                                  # Si el compilado fue exitoso (código de estado 0) procede a eliminar el .rar viejo y crear uno nuevo
  echo -e "\nSe pudo crear el archivo compilado con éxito"
fi

if [ -f "./build/windows/SistemaVotaciones.rar" ]; then
  echo -e "\nEliminando .rar anterior para crear el actualizado..."
  rm -f ./build/bin/windows/SistemaVotaciones.rar
fi

chmod +x ./build/bin/windows/main.exe
mv ./build/bin/windows/main.exe "./build/bin/windows/Sistema de Votaciones.exe"   # Renombro el ejecutable de "main.exe" a "Sistema de Votaciones.exe"

which rar &>/dev/null    # Verifica si existe rar en el sistema

if [ $? -eq 0 ]; then
  rar a -ep ./build/bin/windows/SistemaVotaciones.rar ./build/bin/windows/dlls/*
  rar a -ep1 ./build/bin/windows/SistemaVotaciones.rar ./build/bin/windows/mariadb-11.4.10-winx64/*
  rar a ./build/bin/windows/SistemaVotaciones.rar ./build/bin/windows/VC_redist.x64.exe
  rar a ./build/bin/windows/SistemaVotaciones.rar ./assets/*
  rar a ./build/bin/windows/SistemaVotaciones.rar ./fonts/*
  rar a ./build/bin/windows/SistemaVotaciones.rar .config
  rar a -ep ./build/bin/windows/SistemaVotaciones.rar "./build/bin/windows/Sistema de Votaciones.exe"
  rm "./build/bin/windows/Sistema de Votaciones.exe"
else
  echo -e "\nEl comando rar no se encontró en el sistema, por favor instálelo"
  exit 1
fi

if  [ $? -eq 0 ]; then      # Si el comprimido con sus dependencias se compiló correctamente (código de estado 0) mostrará un mensaje de que todo ocurrió perfectamente
  echo -e "\n\nArchivo ./build/bin/windows/SistemaVotaciones.rar creado con éxito listo para su uso"
else                        # Si el comprimido tuvo errores, mostrará un mensaje diciendo de que falló
  echo -e "\n\nNo se pudo crear el comprimido con éxito"
  exit
fi
