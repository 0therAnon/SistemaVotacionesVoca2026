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

ls ./build/deps/linux.tar.xz

if [ $? -eq 0 ]; then
  tar -xvf ./build/bin.tar.xz
  tar -xvf ./build/deps/linux.tar.xz
  tar -xvf ./build/deps/windows.tar.xz
  mv bin ./build/bin
  mv linux ./build/deps
  mv windows ./build/deps
  rm ./build/bin.tar.xz
  rm ./build/deps/linux.tar.xz
  rm ./build/deps/windows.tar.xz
  sleep 1
fi

which make

if [ $? -ne 0 ]; then
  echo ""
  echo "El comando make no se encuentra en el sistema, por favor, instale make"
  echo "Puede instalarlo usando:"
  echo "    "
  echo "    sudo apt install make"
  echo "    sudo dnf install make"
  echo "    "
  echo "Depende del instalador de su sistema"
  exit 1
fi

make --makefile=./build/Makefile                                          # Se procede a compilar

if [ $? -ne 0 ]; then                                                     # Si la compilación falla, envía un mensaje diciendo que falló, y sale del programa
  echo ""
  echo "El programa tuvo errores en su compilación"
  echo ""
  exit 1
fi

export LD_LIBRARY_PATH=$PWD/build/bin/linux:$LD_LIBRARY_PATH              # En linux, el ejecutable necesita la librería dinámica que se encuentra en la carpeta bin/linux para poder ejecutarse, entonces la almacena en la variable LD_LIBRARY_PATH
./build/bin/linux/main                                                    # Se ejecuta el binario

if [ $? -eq 0 ]; then                                                     # Se comprueba el estado del programa
  echo ""
  echo "El programa fue ejecutado de manera exitosa"
else
  echo ""
  echo "El programa tuvo errores en su ejecución, sin embargo, si usted lee el error algo relacionado a memory leaks de libcrypto, no se preocupe, el error es de libcrypto, no nuestro"
fi
