#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>  // isdigit()
#include <cjson/cJSON.h>
#include <sys/socket.h>

#include "register_response.c"
#include "dynamic_array.c"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024 
bool is_number(const char *s) {
    for (int i = 0; s[i] != '\0'; i++) {
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    Array client_list; // Defining home-made dynamic list https://stackoverflow.com/questions/3536153/c-dynamically-growing-array

    init_array(&client_list, 1);

    if (argc != 2) {
        printf("\nError: expected 2 arguments but found: %d", argc);
        printf("\nCorrect usage: ./<serverName> <port>");
        printf("\nYour input was: ");
        for (int i = 0; i < argc; i++) {
            printf("\nargv[%d]: %s", i, argv[i]);
        }
        return 1; 
    }

    if (!is_number(argv[1])) {
        printf("Error, port is not a number\n");
        return 1; 
    }

    int port = atoi(argv[1]);
    printf("Server name: %s \nListening on port: %d \n", argv[0], port);
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    

    // the socket of scokets, the server socket :o
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // a socket always needs an address and a port, we assign them here
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), ip, INET_ADDRSTRLEN);

    printf("Servidor escuchando en la dirección IP: %s, puerto: %d...\n", ip, port);

    // new conections
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (read(new_socket, buffer, BUFFER_SIZE) < 0) {
        perror("Error while reading query from client");
        close(new_socket);
        return -1;
    }
    
    printf("Received message from client: %s\n", buffer);

    cJSON *client = cJSON_Parse(buffer);
    cJSON *tipo = cJSON_GetObjectItem(client, "tipo");

    if (strcmp(tipo->valuestring, "REGISTRO") == 0) {
        if(register_response(new_socket, buffer, BUFFER_SIZE) < 0) {
            printf("Unable to register");
            cJSON_Delete(client);

        } else {
            cJSON_DeleteItemFromObject(client, "tipo");
            cJSON_AddStringToObject(client, "estado", "Activo");
            insert_array(&client_list, client);
            printf("\nUser registered successfully!\n");
            for (size_t i = 0; i < client_list.used; i++) {
                char *json_str = cJSON_Print(client_list.array[i]);
                printf("\nCliente %zu:\n%s\n", i + 1, json_str);
                free(json_str);
            }
            cJSON_Delete(client);
        }

    } else if (strcmp(tipo->valuestring, "BROADCAST") == 0) {

    } /* And so on */
    


    // client socket and closing server scoket at hte end
    close(new_socket);
    close(server_fd);
    free_array(&client_list);
    
    return 0;
}
