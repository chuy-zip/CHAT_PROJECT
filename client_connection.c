#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 50213
#define SERVER_IP_ADDRESS "" // TBD

int client_connection()
{
    char client_name[11], server_response[BUFFER_SIZE];

    int client_socket;
    struct sockaddr_in server_address;

    // Crear socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Unable to create a socket");
        return -1;
    }

    // Configurar dirección del servidor
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);  // Puerto del servidor
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);  // Dirección IP del servidor

    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to server failed");
        close(client_socket);
        return -1;
    }

    printf("Connected to the server successfully!\n");

    // Definiendo nombre de usuario
    printf("Type your user name (Max. 10 characters): ");
    fgets(client_name, sizeof(client_name), stdin);
    client_name[strcspn(client_name, "\n")] = 0;

    // Enviando nombre de usuario al server
    if (send(client_socket, client_name, strlen(client_name), "REGISTRO") < 0) {
        perror("Unable to send user name");
        close(client_socket);
        return -1;
    }

    // Obteniendo respuesta del server
    memset(server_response, 0, BUFFER_SIZE);
    if (recv(client_socket, server_response, BUFFER_SIZE, 0) < 0) {
        perror("Error while receiving response from server");
        close(client_socket);
        return -1;
    }

    // Manejando error "Usuario repetido"
    if (strcmp(server_response, "ERROR") == 0) {
        printf("User %s already logged in\n", client_name);
        close(client_socket);
        return -1;
    }
    
    // Éxito
    printf("Welcome, %s\n", client_name);
    
    /* Movidas */

    // Cerrar socket al final de todo
    close(client_socket);

    return 0;
}