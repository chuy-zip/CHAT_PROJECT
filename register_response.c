#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
# include <sys/socket.h>
# include <netinet/in.h>

int register_response(int socket, char buffer[], int buffer_size)
{    
    cJSON *client = cJSON_Parse(buffer);
    cJSON *response = cJSON_CreateObject();

    cJSON *tipo = cJSON_GetObjectItem(client, "tipo");
    cJSON *usuario = cJSON_GetObjectItem(client, "usuario");
    cJSON *direccion = cJSON_GetObjectItem(client, "direccionIP");

    if (tipo == NULL || usuario == NULL || direccion == NULL) {
        printf("Incorrect client data\n");
        cJSON_AddStringToObject(response, "response", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Datos de usuario inválidos");
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

    return 0;
}