# El compilador se ejecuta de la siguiente manera:
#
# ./compiler.sh
#
# En caso de que no pueda ejecutarlo, escriba:
#
# chmod +x ./compiler.sh
#
# Esto lo que hace es darle permisos de ejecución al compilador
#
# Lo que hace este compilador es ejecutar el comando make y ejecutar el programa, el comando make se encarga del compilado del programa, solo que make es más rapido debido a que solo recompila desde cero los archivos que hayan sido modificados
# El comando make para funcionar, lo que hace es leer un archivo llamado Makefile, el cual posee toda la configuración de compilado

ls ./build/bin.tar.xz 2>/dev/null

if [ $? -eq 0 ]; then
  tar -xvf ./assets.tar.xz
  tar -xvf ./build/bin.tar.xz
  tar -xvf ./build/deps/linux.tar.xz
  tar -xvf ./build/deps/windows.tar.xz
  mv bin ./build/bin
  mv linux ./build/deps
  mv windows ./build/deps
  rm ./assets.tar.xz
  rm ./build/bin.tar.xz
  rm ./build/deps/linux.tar.xz
  rm ./build/deps/windows.tar.xz
  sleep 1
fi

which make &>/dev/null

if [ $? -ne 0 ]; then
  echo "\nEl comando make no se encuentra en el sistema, por favor, instálelo"
  echo "Puede instalarlo usando:\n"
  echo "    sudo apt install make"
  echo "    sudo dnf install make"
  echo "    sudo pacman -S make"
  echo "\nDepende del instalador de su sistema"
  exit 1
fi

make --makefile=./build/Makefile                                          # Se procede a compilar

if [ $? -ne 0 ]; then                                                     # Si la compilación falla, envía un mensaje diciendo que falló, y sale del programa
  echo "\nEl programa tuvo errores en su compilación"
  echo "En caso de que alguna librería falte en su sistema, recuerde instalar las siguientes:"
  echo "    libpng-dev"
  echo "    libmysqlclient-dev"
  echo "    libssl-dev"
  echo "    zlib1g-dev"
  echo "    libgl-dev\n"
  exit 1
fi

export LD_LIBRARY_PATH=$PWD/build/bin/linux:$LD_LIBRARY_PATH              # En linux, el ejecutable necesita la librería dinámica que se encuentra en la carpeta bin/linux para poder ejecutarse, entonces la almacena en la variable LD_LIBRARY_PATH
mv ./build/bin/linux/main .                                               # Mueve el binario al directorio actual
./main                                                                    # Ejecuta el binario

if [ $? -eq 0 ]; then                                                     # Se comprueba el estado del programa
  echo -e "\nEl programa fue ejecutado de manera exitosa"
else
  echo -e "\nEl programa tuvo errores en su ejecución, sin embargo, si usted lee el error algo relacionado a memory leaks de libcrypto, no se preocupe, el error es de libcrypto, no nuestro"
fi
