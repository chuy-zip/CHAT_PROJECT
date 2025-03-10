#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>  // isdigit()

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

    int port = atoi(argv[1]);  // Convertir el puerto a entero
    printf("Server name: %s \nListening on port: %d \n", argv[0], port);
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *hello = "Hello, World!";

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

    // testing to send a hello world to a client
    send(new_socket, hello, strlen(hello), 0);
    printf("Mensaje 'Hello, World!' enviado al cliente.\n");

    // client socket and closing server scoket at hte end
    close(new_socket);
    close(server_fd);
    
    return 0;
}
