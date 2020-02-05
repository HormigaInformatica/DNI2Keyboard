# DNI2Keyboard
Pequeña aplicación que se queda en segundo plano, al conectar un dni en el lector, escribe el numero de DNI como si fuera un teclado

Pendiente de documentar el código, puede servir como ejemplo también para una aplicación en segundo plano, con icono en la bandeja del sistema, con menú contextual.

También tiene código de comunicacion con librería PKCS11, en concreto con la del DNI Electrónico Español, pues utilizando su DLL (c:\windows\system32\DNIe_P11_x64.dll) 

Básicamente, al iniciar comprueba que esté la librería dll del DNI Electronico (descargable desde https://dnielectronico.es ) y haya un lector conectado y accesible. 
Se queda en segundo plano, consultando cada segundo si hubiera un dni dentro del lector. 
Una vez encontrado el DNI, busca el certificado público de autenticación y extrae el numero del dni del subject del certificado, dentro del tag SERIALNUMBER. 
Una vez obtenido el número, se envía como si fueran pulsaciones de teclado, finalizando con la tecla ENTER (se podría modificar con una entrada en el registro, o un archivo de configuracion, una entrada en el menu contextual o recibiendo argumentos de entrada) pero como es una aplicación básica, recién hecha, sin propósitos comerciales y ademas incluye codigo fuente, quien quiera colaborar, bienvenido sea ;)

Desarrollada con Visual Studio 2017.
