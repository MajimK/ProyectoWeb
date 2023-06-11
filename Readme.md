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

Instrucciones:

Abrir el cmd en la direccion donde se encuentra el webserver.c, y correr el comando:

gcc webserver.c -o <NOMBRE_DEL_ARCHIVO>

Para ejecutar el proyecto se le deben enviar dos parametros:

-el puerto por donde se va a escuchar (recomendamos usar cualquiera de estos tres: 1025,8080,8000)

-la direccion del ordenador que se va a mostrar

el comando a ejecutar seria:

./<NOMBRE DEL ARCHIVO> <PUERTO> <DIRECCION>


Ejemplo:

./ser 1025 /mnt/d/escuela

Una vez ejecutado el comando, abrir el navegador y escribir: 

localhost:<PUERTO>

Detalles de implementación:

Con esto se creará un socket que escuchara por el puerto enviado.
La respuesta que se le enviará al cliente en una página html, con los ficheros en el directorio enviado como parámetro
Para navegar por los directorios basta con dar click en la fila con el nombre.
Todos las carpetas poseen un tamaño por defecto(4096 bytes). De esta manera al ordenar por tamaño todas quedan agrupadas.
Para la ordenación basta con dar click encima de la cabecera de la columna (Name, Size o Date), esta ordenación se hara primero de forma ascendente, si volvemos a dar click ordenará de forma descendente. La fecha se encuentra sigue el formato Año-Mes-Dia.
