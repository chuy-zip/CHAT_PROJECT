# CHAT
### Integrantes:
- Eunice Mata, 21231 (eunicean) 
- Ricardo Chuy, 222107 (chuy-zip)
- Hector Penedo, 22217 (DANdelion-0908)

### Librerías necesarias a instalar
```c
cjson
```

## ¿Cómo correr el código?

#### Crear los Compiladores
Compilador del servidor
```
gcc server/chatServer.c -o chatServer -lcjson
```
Compilador para el cliente
```
 gcc -o menu clientMain.c client/client_connection.c client/client_register.c client/client_list.c client/client_info.c -lcjson -pthread
```

#### Usar los compiladores
En terminal correr los ejecutables
Servidor
```
./chatServer [tu_puerto]
```
Cliente
```
./menu [username] [ip_servidor] [puerto_servidor]
```
- [username] puede ser cualquier nombre
- [ip_servidor] debe de ser la ip del servidor - <span style="background-color:rgb(130, 62, 255)">0.0.0.0</span> si el servidor es local
- [puerto_servidor] el puerto en el que el servidor ue levantado