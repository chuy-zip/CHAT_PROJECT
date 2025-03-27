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
* @brief Responde a la solicitud "ESTADO" de un cliente
* @param int: Socket del cliente
* @param char[]: Buffer que contiene el JSON de la solicitud
* @param bool: flag que indica si se encontró al usuario
* @return cJSON: Respuesta a la solicitud
*/
int state_response(int socket, char buffer[], bool user_flag)
{    
    cJSON *client = cJSON_Parse(buffer);
    cJSON *response = cJSON_CreateObject();

    cJSON *tipo = cJSON_GetObjectItem(client, "tipo");
    cJSON *usuario = cJSON_GetObjectItem(client, "usuario");

    if (tipo == NULL || usuario == NULL) {
        printf("Incorrect user data\n");
        cJSON_AddStringToObject(response, "respuesta", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Estado inválido");
        send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
        printf("Message: %s\n sended to client.\n", cJSON_Print(response));
        cJSON_Delete(client);
        cJSON_Delete(response);
        close(socket);
        return -1;
    
    } else if (user_flag == false) {
        printf("\nUser not found\n");
        cJSON_AddStringToObject(response, "respuesta", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Usuario no encontrado");
        send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
        printf("Message: %s\n sended to client.\n", cJSON_Print(response));
        cJSON_Delete(client);
        cJSON_Delete(response);
        close(socket);
        return -1;
    }

    // Enviando una respuesta OK al cliente
    cJSON_AddStringToObject(response, "response", "OK");

    send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
    printf("Message: %s\n sended to client.\n", cJSON_Print(response));
    cJSON_Delete(response);

    return 0;
}