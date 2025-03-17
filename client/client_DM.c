#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cjson/cJSON.h>

#define BUFFER_SIZE 1024

/*
* @brief Obtiene la información de todos los usuarios conectados.
* @param int: client_socket: Socket del cliente.
* @return cJSON*: Objeto JSON con los datos de los clientes.
* @note El JSON de respuesta tiene el siguiente formato: {"respuesta": {"0": {"nombre": "example", "direccionIP": "0.0.0.0.", ...}, "1": {"nombre": "example2", ...}}}
* @return NULL: Error.
*/
cJSON* client_DM(int client_socket, char sender_name[], char receiver_name[], char message[])
{
    char server_response[BUFFER_SIZE];
    
    // Añadiendo datos al objeto cliente
    cJSON *client = cJSON_CreateObject();
    cJSON_AddStringToObject(client, "accion", "DM");
    cJSON_AddStringToObject(client, "nombre_emisor", sender_name);
    cJSON_AddStringToObject(client, "nombre_destinatario", receiver_name);
    cJSON_AddStringToObject(client, "mensaje", message);

    char *client_json = cJSON_Print(client);

    // Enviando mensaje al server
    if (send(client_socket, client_json, strlen(client_json), 0) < 0) {
        perror("Unable to send message to server");
        free(client_json);
        cJSON_Delete(client);
        close(client_socket);
        return NULL;
    }

    // Obteniendo respuesta del server
    if (recv(client_socket, server_response, BUFFER_SIZE, 0) < 0) {
        perror("Error while receiving response from server");
        free(client_json);
        cJSON_Delete(client);
        close(client_socket);
        return NULL;
    }
    
    // Obteniendo respuesta del server
    cJSON *server = cJSON_Parse(server_response);
    printf("\nServer response: %s\n", cJSON_Print(server));

    // Verificando que no esté vacía
    if (server == NULL) {
        perror("Error while parsing server response\n");
        free(client_json);
        cJSON_Delete(client);
        cJSON_Delete(server);
        close(client_socket);
        return NULL;
    }

    // Obteniendo el status del server
    cJSON *respuesta = cJSON_GetObjectItem(server, "respuesta");
    cJSON *razon = cJSON_GetObjectItem(server, "razon");

    // Manejando error
    if (respuesta != NULL && strcmp(respuesta->valuestring, "ERROR") == 0) {
        printf("ERROR: %s", cJSON_Print(razon));
        cJSON_Delete(server);
        close(client_socket);
        free(client_json);
        cJSON_Delete(client);
        return NULL;
    }

    cJSON_Delete(server);
    free(client_json);
    
    // Éxito
    printf("Message sended successfully", cJSON_Print(respuesta));

    return client;
}