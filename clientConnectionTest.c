#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>  // isdigit()

// checking if string is number
bool is_number(const char *s) {
    for (int i = 0; s[i] != '\0'; i++) {
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

int main(int argc, char const* argv[]) {
    // arguments check
    if (argc != 4) {
        printf("\nError: expected 4 arguments but found: %d", argc);
        printf("\nCorrect usage: ./<clientName> <username> <serverIP> <serverPort>");
        printf("\nYour input was: ");
        for (int i = 0; i < argc; i++) {
            printf("\nargv[%d]: %s", i, argv[i]);
        }
        printf("\n");
        return 1; 
    }

    if (!is_number(argv[3])) {
        printf("Error, port is not a number\n");
        return 1; 
    }

    // port to number
    int port = atoi(argv[3]);

    // user data
    printf("Nombre del cliente: %s\n", argv[0]);
    printf("Nombre de usuario: %s\n", argv[1]);
    printf("Conectando al servidor en la dirección IP: %s, puerto: %d...\n", argv[2], port);

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };

    // client socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error \n");
        return -1;
    }

    // server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);  

    // ip texto to binary
    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // connect
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // communication wit srever
    send(client_fd, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    valread = read(client_fd, buffer, 1024 - 1);  // Leer hasta 1023 caracteres
    printf("Respuesta del servidor: %s\n", buffer);

    // client scoket
    close(client_fd);
    return 0;
}
