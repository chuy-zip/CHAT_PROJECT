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
* @brief Responde a la solicitud "DM" de un cliente
* @param int: Socket del cliente
* @param char[]: Buffer que contiene el JSON de la soliocitud
* @return cJSON: Respuesta a la solicitud
*/
int send_response(int socket, char buffer[])
{    
    cJSON *client = cJSON_Parse(buffer);
    cJSON *response = cJSON_CreateObject();

    cJSON *accion = cJSON_GetObjectItem(client, "accion");
    cJSON *nombre_remitente = cJSON_GetObjectItem(client, "nombre_remitente");
    cJSON *nombre_destinatario = cJSON_GetObjectItem(client, "nombre_emisor");

    if (accion == NULL || nombre_destinatario == NULL || nombre_remitente == NULL) {
        printf("Incorrect user data\n");
        cJSON_AddStringToObject(response, "respuesta", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Datos inválidos");
        send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
        printf("Message: %s\n sended to client.\n", cJSON_Print(response));
        cJSON_Delete(client);
        cJSON_Delete(response);
        return -1;
    }

    // Enviando una respuesta OK al cliente
    cJSON_AddStringToObject(response, "response", "OK");

    send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
    printf("Message: %s\n sended to client.\n", cJSON_Print(response));
    cJSON_Delete(response);

    return 0;
}