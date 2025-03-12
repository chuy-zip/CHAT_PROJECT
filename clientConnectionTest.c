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

void handle_exit(int client_fd, const char *username) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "tipo", "EXIT");
    cJSON_AddStringToObject(root, "usuario", username);
    cJSON_AddStringToObject(root, "estado", "");
    char *exit_json = cJSON_Print(root);
    cJSON_Delete(root);

    // sending data for exit
    printf("Sending JSON: %s\n", exit_json);
    send(client_fd, exit_json, strlen(exit_json), 0);
    free(exit_json);
}


void handle_broadcast_global(int client_fd) {
    char buffer[1024] = {0};
    int valread;
    
    printf("Now connected. write meissages or exit to end connection.\n");
                
    while (1) {
        printf("Message: ");
        fgets(buffer, 1024, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strcmp(buffer, "exit") == 0) {
          printf("Closing global chat.\n");
          break;
        }

        send(client_fd, buffer, strlen(buffer), 0);

        valread = read(client_fd, buffer, 1024 - 1);

        if (valread <= 0) {
            printf("Disconnected.\n");
            break;
        }
        
        buffer[valread] = '\0';
        printf("Server responded with: %s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
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

    int status, client_fd;
    struct sockaddr_in serv_addr;
    

    // client socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error \n");
        return -1;
    }

    // server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);  

    // ip text to binary
    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // connect
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    bool stay = true;
    printf("+-----------------------------------------------------------+\n");
    printf("|                  WELCOME TO CHAT SERVER                   |\n");
    printf("+-----------------------------------------------------------+\n");
    while(stay){
        int chatSelection;
        printf("\nSelect an action \n");
        printf(" 1. Chat globally\n");
        printf(" 2. Chat privately\n");
        printf(" 3. Change status\n");
        printf(" 4. List all users connected\n");
        printf(" 5. Get information about a user\n");
        printf(" 6. Help\n");
        printf(" 7. Exit\n");
        printf(" -> ");
        scanf("%d", &chatSelection);
        printf("%d\n", chatSelection);
        
        while (getchar() != '\n'); 
        
        switch (chatSelection) {
            case 1:
                printf("+-----------------------------------------------------------+\n");
                printf("|                        GLOBAL CHAT                        |\n");
                printf("+-----------------------------------------------------------+\n");
  
                handle_broadcast_global(client_fd);
                
                break;
            case 2:
                printf("+-----------------------------------------------------------+\n");
                printf("|                       PRIVATE CHAT                        |\n");
                printf("+-----------------------------------------------------------+\n");
                // list all users available to chat
                // select one user somehow
                // send and receive chats
                break;
            case 3:
                printf("+-----------------------------------------------------------+\n");
                printf("|                       CHANGE STATUS                       |\n");
                printf("+-----------------------------------------------------------+\n");
                // can change from active, ocuppied or inactive
                break;
            case 4:
                printf("+-----------------------------------------------------------+\n");
                printf("|                 LIST ALL USERS CONNECTED                  |\n");
                printf("+-----------------------------------------------------------+\n");
                // self explanatory
                break;
            case 5:
                printf("+-----------------------------------------------------------+\n");
                printf("|                   SEE INFO ABOUT A USER                   |\n");
                printf("+-----------------------------------------------------------+\n");
                // list all users
                // select one
                // show username and ip address
                break;
            case 6:
                int stillNeedsHelp = true;
                while(stillNeedsHelp){
                    int helpSelection;
                    printf("\n+-----------------------------------------------------------+\n");
                    printf("|                       HELP SECTION                        |\n");
                    printf("+-----------------------------------------------------------+\n");
                    printf("Select the option where you want to get info about\n");
                    printf(" 1. Chat globally\n");
                    printf(" 2. Chat privately\n");
                    printf(" 3. Change status\n");
                    printf(" 4. List all users connected\n");
                    printf(" 5. Get information about a user\n");
                    printf(" 6. Help\n");
                    printf(" 7. Exit\n");
                    printf(" -> ");
                    scanf("%d", &helpSelection);
                    
                    switch(helpSelection){
                        case 1:
                            printf("\nGLOBAL CHAT -----------------------------------------------\n");
                            printf("  Here you will be able to send an receive messages form\n");
                            printf("  and to all the users connected in this global chat.\n");
                            printf("  You can exit writing EXIT in the send message space.\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 2:
                            printf("\nPRIVATE CHAT ----------------------------------------------\n");
                            printf("  Here you will be able to chat with privately with a user\n");
                            printf("  that you can select from an available list.\n");
                            printf("  You can exit writing EXIT in the send message space.\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 3:
                            printf("\nCHANGE STATUS ---------------------------------------------\n");
                            printf("  Here you will be able to change your status between the\n");
                            printf("  options of ACTIVE, OCUPPIED or INACTIVE.\n");
                            printf("  NOTE: Your default status is ACTIVE\n");
                            printf("  NOTE: After some time innactive your status will change\n");
                            printf("        to INACTIVE\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 4:
                            printf("\nLIST ALL USERS CONNECTED ----------------------------------\n");
                            printf("  Here you will be able to see all users connected to the\n");
                            printf("  server\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 5:
                            printf("\nSEE INFO ABOUT A USER -------------------------------------\n");
                            printf("  Here you will be able to select an user, and we will\n");
                            printf("  show you the USERNAME and IP ADDRES of that user.\n");
                            printf("  Maybe in the future we won't dox our users :)\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 6:
                            printf("\nHELP SECTION ----------------------------------------------\n");
                            printf("  Here you get information about the various things you\n");
                            printf("  cand do on our server.\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 7:
                            printf("\nEXIT ------------------------------------------------------\n");
                            printf("  Here is where you LOG OUT, once you have logged out  all\n");
                            printf("  your chats will dissapear. You can log in again, with\n");
                            printf("  the same username or a different one to start chats\n");
                            printf("  from zero\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        default:
                            printf("Not an option :/\n");
                    }
                    
                    int needsHelp;
                    printf("Do you still need help?\n 1. Yes\n 2. No\n -> ");
                    scanf("%d", &needsHelp);
                    if(needsHelp == 2){
                        stillNeedsHelp = false;
                    }
                }
                
                break;
            case 7:
                printf("+-----------------------------------------------------------+\n");
                printf("|                          LOG OUT                          |\n");
                printf("+-----------------------------------------------------------+\n");
                printf("Bye\n");
                handle_exit(client_fd, argv[1]);
                stay = false;
                break;
            default:
                printf("Not an option :/\n");
        }
    }
    close(client_fd);
    return 0;
}
