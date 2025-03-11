#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>  // isdigit()
#include <cjson/cJSON.h>

// checking if string is number
bool is_number(const char *s) {
    for (int i = 0; s[i] != '\0'; i++) {
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

char* create_exit_json(const char* username) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "tipo", "EXIT");
    cJSON_AddStringToObject(root, "usuario", username);
    cJSON_AddStringToObject(root, "estado", "");

    char *json_string = cJSON_Print(root);
    cJSON_Delete(root);

    return json_string;
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
    printf("client: %s\n", argv[0]);
    printf("username: %s\n", argv[1]);
    printf("Connecting to server in IP: %s, port: %d...\n", argv[2], port);

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    //char* hello = "Hello from client";
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

    printf("Now connected. write meissages or exit to end connection.\n");

    while (1) {
      printf("Message: ");
      fgets(buffer, 1024, stdin);
      buffer[strcspn(buffer, "\n")] = 0;

      if (strcmp(buffer, "exit") == 0) {
          // exit json
          char* exit_json = create_exit_json(argv[1]);
          printf("Sending JSON: %s\n", exit_json);  
          send(client_fd, exit_json, strlen(exit_json), 0);
          free(exit_json); 

          printf("Closing connection.\n");
          break;
      }

      // client sending
      send(client_fd, buffer, strlen(buffer), 0);

      // receiving f
      valread = read(client_fd, buffer, 1024 - 1);

      if (valread <= 0) {
          printf("Disconnected.\n");
          break;
      }

      buffer[valread] = '\0';
      printf("Server responded with: %s\n", buffer);
      memset(buffer, 0, sizeof(buffer));
  }

    close(client_fd);
    return 0;
}
