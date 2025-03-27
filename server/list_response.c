#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdbool.h>

/*
* @brief Responde a la solicitud "LISTA" de un cliente
* @param int: Socket del cliente
* @param char[]: Buffer que contiene el JSON de la solicitud
* @param int: Tamaño del buffer
* @return cJSON: Respuesta a la solicitud
*/
int list_response(int socket, cJSON *users_to_return)
{    
    cJSON *response = cJSON_CreateObject();

    // Enviando una respuesta con el usuario encontrado
    cJSON_AddItemToObject(response, "accion", cJSON_CreateString("LISTA"));
    cJSON_AddItemToObject(response, "usuarios", users_to_return);
    

    send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
    printf("Message: %s\n sended to client.\n", cJSON_Print(response));
    cJSON_Delete(response);

    return 0;
}