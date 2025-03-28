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
* @brief Responde a la solicitud "MOSTRAR" de un cliente
* @param int: Socket del cliente
* @param char[]: Buffer que contiene el JSON de la solicitud
* @param int: Tamaño del buffer
* @return cJSON: Respuesta a la solicitud
*/
int info_response(int socket, char buffer[], int buffer_size, bool user_flag, cJSON *user_to_return)
{    
    cJSON *client = cJSON_Parse(buffer);
    cJSON *response = cJSON_CreateObject();

    cJSON *usuario = cJSON_GetObjectItem(client, "usuario");

    if (usuario == NULL) {
        printf("Incorrect user name\n");
        cJSON_AddStringToObject(response, "respuesta", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Datos de usuario inválidos");
        send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
        printf("Message: %s\n sended to client.\n", cJSON_Print(response));
        printf("Incorrect user name\n");
        cJSON_Delete(client);
        cJSON_Delete(response);
        return -1;
    
    } else if (user_flag == false) {
        printf("\nCouldn't find mentioned user\n");
        cJSON_AddStringToObject(response, "respuesta", "ERROR");
        cJSON_AddStringToObject(response, "razon", "Usuario no encontrado");
        send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
        printf("Message: %s\n sended to client.\n", cJSON_Print(response));
        cJSON_Delete(client);
        cJSON_Delete(response);
        printf("Incorrect user name\n");
        return -1;
    }

    // Enviando una respuesta con el usuario encontrado
    cJSON *user_array = cJSON_CreateArray();
    char formatted_string[50];

    cJSON *username_to_list = cJSON_GetObjectItem(user_to_return, "usuario");
    cJSON *direccion_to_list = cJSON_GetObjectItem(user_to_return, "direccionIP");
    cJSON *estado_to_list = cJSON_GetObjectItem(user_to_return, "estado");
    cJSON *socket_to_list = cJSON_GetObjectItem(user_to_return, "socket");

    sprintf(formatted_string, "%s-%s-%d", username_to_list->valuestring, direccion_to_list->valuestring, socket_to_list->valueint);
    cJSON_AddItemToArray(user_to_return, cJSON_CreateString(formatted_string));

    cJSON_AddStringToObject(response, "tipo", "MOSTRAR");
    cJSON_AddStringToObject(response, "usuario", formatted_string);
    cJSON_AddStringToObject(response, "estado", cJSON_GetObjectItem(user_to_return, "estado")->valuestring);

    send(socket, cJSON_Print(response), strlen(cJSON_Print(response)), 0);
    printf("Message: %s\n sended to client.\n", cJSON_Print(response));
    cJSON_Delete(response);

    return 0;
}