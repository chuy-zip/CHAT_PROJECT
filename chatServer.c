#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>  // isdigit()
#include <pthread.h>
#include <cjson/cJSON.h>

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

typedef struct {
    int socket;
    struct sockaddr_in address;
    char username[50]; 
} client_info_t;

void* handle_client(void* arg) {
    client_info_t* client_info = (client_info_t*)arg;
    int client_socket = client_info->socket;
    char buffer[BUFFER_SIZE] = {0};
    char* welcome_message = "Welcome to the chat server!\n";

    send(client_socket, welcome_message, strlen(welcome_message), 0);

    while (1) {
        // reading user 
        int valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        // raw message
        printf("Received raw message: %s\n", buffer);

        // json parsing
        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
            printf("Error al parsear JSON. \n");
            continue;
        }

        // printing json
        char *json_str = cJSON_Print(json);
        printf("Parsed JSON: %s\n", json_str);
        free(json_str);

        // checking tipo of json
        cJSON *tipo = cJSON_GetObjectItemCaseSensitive(json, "tipo");
        if (tipo != NULL && strcmp(tipo->valuestring, "EXIT") == 0) {
            // free the user
            cJSON *usuario = cJSON_GetObjectItemCaseSensitive(json, "usuario");
            if (usuario != NULL) {
                printf("User: %s requested to exit.\n", usuario->valuestring);
            }
            cJSON_Delete(json); 
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        cJSON_Delete(json);
    }

    // closing socket
    close(client_socket);
    free(client_info);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
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
    char *hello = "Hello, World from chat server";

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

    printf("Server listening on IP: %s, port: %d...\n", ip, port);

    // new conections
    while (1) {
        // Aceptar una nueva conexión
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // showing new connection
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("New client connected from: %s\n", client_ip);

        client_info_t* client_info = (client_info_t*)malloc(sizeof(client_info_t));
        client_info->socket = new_socket;
        client_info->address = address;

        // thread for handling connect
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)client_info) != 0) {
            perror("Error creating thread");
            close(new_socket);
            free(client_info);
        }

        // detaching thread at the end
        pthread_detach(thread_id);
    }

    // Closing server
    close(server_fd);
    
    return 0;
}
