#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
# include <sys/socket.h>
# include <netinet/in.h>

/*
* @brief Responde a la solicitud "REGISTRO" de un cliente
* @param int: Socket del cliente
* @param char[]: Buffer que contiene el JSON de la soliocitud
* @param int: Tamaño del buffer
* @return cJSON: Respuesta a la solicitud
*/
int register_response(int socket, char buffer[], int buffer_size, bool repeated_flag)
{    
    cJSON *client = cJSON_Parse(buffer);
    cJSON *response = cJSON_CreateObject();

    cJSON *tipo = cJSON_GetObjectItem(client, "tipo");
    cJSON *usuario = cJSON_GetObjectItem(client, "usuario");
    cJSON *direccion = cJSON_GetObjectItem(client, "direccionIP");

    if (tipo == NULL || usuario == NULL || direccion ) {
        printf("Incorrect client data\n");
        cJSON_AddStringToObject(response, "respuesta", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Datos de usuario inválidos");
        send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
        printf("Message: %s\n sended to client.\n", cJSON_Print(response));
        cJSON_Delete(client);
        cJSON_Delete(response);
        close(socket);
        return -1;
    
    } else if (repeated_flag) {
        printf("\nUsername and/or ip address already logged in\n");
        cJSON_AddStringToObject(response, "respuesta", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Nombre y/o dirección duplicados");
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