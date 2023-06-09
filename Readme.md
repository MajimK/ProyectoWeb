Proyecto de Sistema Operativo: WebServer

*Integrantes:*
Kevin Majim Ortega Alvarez
Yoan Rene Ramos Corrales

*Funcionalidades:*
Funcionalidades Básicas (3 puntos)
Funcionalidades Adicionales:
El servidor muestra detalles de los archivos y directorios (Tamaño y fecha) y es capaz de ordenar por estos.
Permite peticiones de mutiples clientes.
Total de puntos: 5

Detalles de implementación:
Al ejecutar el programa se le deben enviar dos parámetros:
-el puerto por donde se va a escuchar 
-la dirección del ordenador que se desea abrir
Ejemplo:
./ser 1025 /mnt/d/escuela
donde ser es el nombre del archivo a correr

Con esto se creará un socket que escuchara por el puerto enviado.
La respuesta que se le enviará al cliente en una página html, con los ficheros en el directorio enviado como parámetro
Para navegar por los directorios basta con dar click en la fila con el nombre.
Todos las carpetas poseen un tamaño por defecto(4096 bytes). De esta manera al ordenar por tamaño todas quedan agrupadas.
Para la ordenación basta con dar click encima de la cabecera de la columna (Name, Size o Date), esta ordenación se hara primero de forma ascendente, si volvemos a dar click ordenará de forma descendente.
