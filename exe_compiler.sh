# Este compilador lo que hace es específicamente compilar para dar como resultado un archivo .exe ejecutable de windows

# Compilado en mínima optimización

echo "\nCompilando en mínima optimización...\n\n" && cmd="x86_64-w64-mingw32-g++ -s -DNDEBUG -O0 -std=c++20"

# Compilado en modo release (máxima optimización) (el modo release es el modo en el que el ejecutable se compilará para su uso real, es más rápido y sirve para probar la velocidad real del ejecutable):

#echo "\nCompilando en modo release...\n\n" && cmd="x86_64-w64-mingw32-g++ -s -DNDEBUG -O3 -std=c++20"

# Añade el resto de argumentos para el compilador:

cmd=$cmd" ./*/*.cpp  ./*.cpp"                                                                                                                                    # Le dice al compilador dónde están los archivos para compilar
cmd=$cmd" -I ./build/deps/windows/lib/ -I ./build/deps/windows/mariadb/ -I ./build/deps/windows/mysql/ -I ./build/deps/windows/src/ -I ./config/ -I ./db/ -I ./platform/ -I ./reports/ -I ./screens/ -I ./ui/ -L ./build/deps/windows/lib/"  # Le dice al compilador dónde están las carpetas con las dependencias
cmd=$cmd" -L ./build/deps/windows/mariadb/ -L ./build/deps/windows/mysql/ -L ./build/deps/windows/src/ -L ./config/ -L ./db/ -L ./platform/ -L ./reports/ -L ./screens/ -L ./ui/"  # Le dice al compilador dónde están las librerías necesarias
cmd=$cmd" -lkernel32 -static -lws2_32 -lGlU32 -lOpenGL32 -lhpdf -lstdc++ -llibpng16 -llibmariadb -lzlib -llibcrypto_static -llibssl_static -lmysqlclient -lm -lraylib -lgdi32 -lwinmm"                  # Le dice al compilador cuáles librerías debe de llamar
cmd=$cmd" -o ./build/bin/windows/main.exe"                                                                                                                                  # Le dice al compilador el nombre del archivo de salida, el cual se llama main

echo "Comando ejecutado:\n\n$cmd\n\n"                                                                                                     # Muestra el comando para compilar en la pantalla
$cmd                                                                    # Ejecuta el comando para compilar | Es normal si aparecen advertencias en morado, pero si hay un error, aparecerá en rojo, y no logrará concluir la compilación

# Darle permisos de ejecución al archivo del código compilado para poder ejecutarse | chmod es un comando que permite otorgar permisos de lectura, escritura y ejecución, con esto, le doy permisos de ejecución

if  [ $? -eq 0 ]; then                                                  # Si el compilado fue exitoso (código de estado 0) procede a eliminar el .rar viejo y crear uno nuevo
  echo "\nSe pudo crear el archivo compilado con éxito"
  echo "Eliminando .rar anterior para crear el actualizado..."
  rm ./build/bin/windows/SistemaVotaciones.rar
  chmod +x ./build/bin/windows/main.exe
  mv ./build/bin/windows/main.exe "./build/bin/windows/Sistema de Votaciones.exe"   # Renombro el ejecutable de "main.exe" a "Sistema de Votaciones.exe"
  rar a -ep ./build/bin/windows/SistemaVotaciones.rar ./build/bin/windows/dlls/*
  rar a -ep1 ./build/bin/windows/SistemaVotaciones.rar ./build/bin/windows/mariadb-11.4.10-winx64/*
  rar a ./build/bin/windows/SistemaVotaciones.rar ./build/bin/windows/VC_redist.x64.exe
  rar a ./build/bin/windows/SistemaVotaciones.rar ./fonts/*
  rar a ./build/bin/windows/SistemaVotaciones.rar .config
  rar a -ep ./build/bin/windows/SistemaVotaciones.rar "./build/bin/windows/Sistema de Votaciones.exe"
  rm "./build/bin/windows/Sistema de Votaciones.exe"
else
  echo "\nLa creación del compilado tuvo errores, abortando..."
  exit
fi

if  [ $? -eq 0 ]; then      # Si el comprimido con sus dependencias se compiló correctamente (código de estado 0) mostrará un mensaje de que todo ocurrió perfectamente
  echo "\n\nArchivo ./build/bin/windows/SistemaVotaciones.rar creado con éxito listo para su uso"
else                        # Si el comprimido tuvo errores, mostrará un mensaje diciendo de que falló
  echo "\n\nNo se pudo crear el comprimido con éxito"
  exit
fi
