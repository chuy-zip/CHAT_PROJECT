#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cjson/cJSON.h>

#include "client_register.c"

/*
* @brief Establece la conexión con el servidor.
* @param int: server_port: Puerto del servidor.
* @param char[8]: server_ip_address: Dirección IP del servidor.
* @return int: Socket del cliente.
* @return -1: Error.
* @note Se debe cerrar el socket con la función "close()" cuando ya no se necesite.
*/
int client_connection(int server_port, char server_ip_address[8])
{   
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
    server_address.sin_port = htons(server_port);  // Puerto del servidor
    server_address.sin_addr.s_addr = inet_addr(server_ip_address);  // Dirección IP del servidor
    
    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to server failed");
        close(client_socket);
        return -1;
    }
    
    printf("\nConnected to the server successfully!\n");

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_address.sin_addr), client_ip, INET_ADDRSTRLEN);

    return client_socket;
}

int main(int argc, char const *argv[])
{
    int socket = client_connection(50213, "0.0.0.0");

    cJSON *test = client_register("Dandelion", socket);

    printf("\nTest: %s\n", cJSON_Print(test));

    return 0;
}
