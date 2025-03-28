#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>  // isdigit()
#include <cjson/cJSON.h>
#include <pthread.h>

#include "client/client_connection.h"
#include "client/client_register.h"
#include "client/client_list.h"
#include "client/client_info.h"
#include "client/client_state.h"


#define BUFFER_SIZE_BROAD 1024

void print_users(cJSON *users_list) {
    if (users_list == NULL) {
        printf("Error: No user list available.\n");
        return;
    }

    // Obtener el array de usuarios
    cJSON *usuarios = cJSON_GetObjectItem(users_list, "usuarios");
    if (!cJSON_IsArray(usuarios)) {
        printf("Error: Invalid user list format.\n");
        return;
    }

    cJSON *usuario;
    int i = 0;
    cJSON_ArrayForEach(usuario, usuarios) {
        if (cJSON_IsString(usuario)) {
            char *user_str = usuario->valuestring;
            
            // Separar la cadena por '-'
            char *username = strtok(user_str, "-");
            char *ip = strtok(NULL, "-");

            printf("\n----------------------------\n");
            printf("%d - Username: %s\n", i, username);
            if (ip != NULL) {
                printf("     IP: %s\n", ip);
            }
            printf("----------------------------\n");

            i++;
        }
    }
}

void print_user_info(cJSON *user_info) {
    if (user_info == NULL) {
        printf("Error: No user info available.\n");
        return;
    }

    cJSON *usuario = cJSON_GetObjectItem(user_info, "usuario");
    cJSON *estado = cJSON_GetObjectItem(user_info, "estado");

    if (!usuario || !cJSON_IsString(usuario) || !estado || !cJSON_IsString(estado)) {
        printf("Error: Invalid user information format.\n");
        return;
    }

    char *user_str = usuario->valuestring;

    // Separar la cadena por '-'
    char *username = strtok(user_str, "-");
    char *ip = strtok(NULL, "-");
    char *socket = strtok(NULL, "-");

    printf("\n----------------------------\n");
    printf("%s's Data\n", username);
    if (ip != NULL) {
        printf("     - IP: %s\n", ip);
    }
    if (socket != NULL) {
        printf("     - Socket: %s\n", socket);
    }
    printf("     - Status: %s\n", estado->valuestring);
    printf("----------------------------\n");
}

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
    // printf("Sending JSON: %s\n", exit_json);
    send(client_fd, exit_json, strlen(exit_json), 0);
    free(exit_json);
}

typedef struct {
    int client_fd;
    const char *username;
    const char *to_username;
} thread_data_t;

void* receive_messages(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int client_fd = data->client_fd;
    char buffer[BUFFER_SIZE_BROAD] = {0};

    while (1) {
        int valread = read(client_fd, buffer, BUFFER_SIZE_BROAD - 1);
        
        if (valread <= 0) {
            printf("Disconnected.\n");
            break;
        }
        
        buffer[valread] = '\0';
        
        cJSON *message_json = cJSON_Parse(buffer);
        
        if (message_json == NULL) {
            printf("Error al parsear el JSON.\n");
            return NULL;
        }
        
        cJSON *accion = cJSON_GetObjectItemCaseSensitive(message_json, "accion");
        cJSON *nombre_emisor = cJSON_GetObjectItemCaseSensitive(message_json, "nombre_emisor");
        cJSON *nombre_destinatario = cJSON_GetObjectItemCaseSensitive(message_json, "nombre_destinatario");
        cJSON *mensaje = cJSON_GetObjectItemCaseSensitive(message_json, "mensaje");

        if (accion != NULL && nombre_emisor != NULL && mensaje != NULL && strcmp(accion->valuestring, "BROADCAST") == 0) {
            printf("\n%s: %s\n", nombre_emisor->valuestring, mensaje->valuestring); // show message
            printf(": ");  // ask message
            fflush(stdout);  // show 
        }
        
        cJSON_Delete(message_json);

    }

    return NULL;
}

void* receive_messages_dm(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int client_fd = data->client_fd;
    const char *username = data->username;
    const char *to_username = data->to_username;
    char buffer[BUFFER_SIZE_BROAD] = {0};

    while (1) {
        int valread = read(client_fd, buffer, BUFFER_SIZE_BROAD - 1);
        
        if (valread <= 0) {
            printf("Disconnected.\n");
            break;
        }
        
        buffer[valread] = '\0';
        
        cJSON *message_json = cJSON_Parse(buffer);
        
        if (message_json == NULL) {
            printf("Error al parsear el JSON.\n");
            return NULL;
        }
        
        cJSON *accion = cJSON_GetObjectItemCaseSensitive(message_json, "accion");
        cJSON *nombre_emisor = cJSON_GetObjectItemCaseSensitive(message_json, "nombre_emisor");
        cJSON *nombre_destinatario = cJSON_GetObjectItemCaseSensitive(message_json, "nombre_destinatario");
        cJSON *mensaje = cJSON_GetObjectItemCaseSensitive(message_json, "mensaje");
        
        if (accion != NULL && nombre_emisor != NULL && nombre_destinatario != NULL && mensaje != NULL && strcmp(accion->valuestring, "DM") == 0) {
            if (strcmp(nombre_emisor->valuestring, to_username) == 0 && strcmp(nombre_destinatario->valuestring, username) == 0) {
                printf("\n%s: %s\n", nombre_emisor->valuestring, mensaje->valuestring); // show message
                printf(": ");  // ask message
                fflush(stdout);  // show 
            }
        }
        
        cJSON_Delete(message_json);

    }

    return NULL;
}

void handle_dm(int client_fd, const char *username, const char *to_username) {
    char buffer[1024] = {0};
    int valread;
    pthread_t receive_thread;
    thread_data_t thread_data = {client_fd, username, to_username};
    
    if (pthread_create(&receive_thread, NULL, receive_messages_dm, (void*)&thread_data) != 0) {
        perror("Error creating thread");
        return;
    }
    printf("Now connected with %s. write meissages or exit to end connection.\n", to_username);
                
    printf("Message: ");
    while (1) {
        fgets(buffer, 1024, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strcmp(buffer, "exit") == 0) {
          printf("Closing private chat.\n");
          break;
        }
        
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "accion", "DM");
        cJSON_AddStringToObject(root, "nombre_emisor", username);
        cJSON_AddStringToObject(root, "nombre_destinatario", to_username);
        cJSON_AddStringToObject(root, "mensaje", buffer);
        char *broadcast_json = cJSON_Print(root);
        cJSON_Delete(root);

        // sending data for broadcast
        // printf("Sending JSON: %s\n", broadcast_json);
        send(client_fd, broadcast_json, strlen(broadcast_json), 0);
        free(broadcast_json);
    }

    pthread_cancel(receive_thread);  // cancel thread
    pthread_join(receive_thread, NULL);  // but gracefully, waiting for it to finish
}

void handle_broadcast_global(int client_fd, const char *username) {
    char buffer[1024] = {0};
    int valread;
    pthread_t receive_thread;
    thread_data_t thread_data = {client_fd};
    
    if (pthread_create(&receive_thread, NULL, receive_messages, (void*)&thread_data) != 0) {
        perror("Error creating thread");
        return;
    }
    
    printf("Now connected. write meissages or exit to end connection.\n");
                
    printf("Message: ");
    while (1) {
        fgets(buffer, 1024, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strcmp(buffer, "exit") == 0) {
          printf("Closing global chat.\n");
          break;
        }
        
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "accion", "BROADCAST");
        cJSON_AddStringToObject(root, "nombre_emisor", username);
        cJSON_AddStringToObject(root, "mensaje", buffer);
        char *broadcast_json = cJSON_Print(root);
        cJSON_Delete(root);

        // sending data for broadcast
        // printf("Sending JSON: %s\n", broadcast_json);
        send(client_fd, broadcast_json, strlen(broadcast_json), 0);
        free(broadcast_json);
        
        //send(client_fd, buffer, strlen(buffer), 0);

    }
    pthread_cancel(receive_thread);  // cancel thread
    pthread_join(receive_thread, NULL);  // but gracefully, waiting for it to finish
}

void request_user_list(int client_fd) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "accion", "LISTA");
    
    char *request_json = cJSON_Print(root);
    cJSON_Delete(root);

    // Enviar la solicitud al servidor
    printf("Solicitando lista de usuarios\n");
    send(client_fd, request_json, strlen(request_json), 0);
    free(request_json);

    // Recibir respuesta del servidor
    char buffer[1024] = {0};
    int valread = read(client_fd, buffer, 1023);
    
    if (valread <= 0) {
        printf("Error al recibir la lista de usuarios.\n");
        return;
    }

    buffer[valread] = '\0';  // Asegurar terminación de cadena
    printf("Respuesta recibida: %s\n", buffer);

    // Parsear el JSON recibido
    cJSON *response = cJSON_Parse(buffer);
    if (response == NULL) {
        printf("Error al parsear la respuesta del servidor.\n");
        return;
    }

    printf("Usuarios conectados:\n");
    cJSON *item;
    cJSON_ArrayForEach(item, response) {
        printf("- %s\n", item->valuestring);
    }

    cJSON_Delete(response);
}

int main(int argc, char const *argv[]) {
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

    int port = atoi(argv[3]);
    int socket = client_connection(port, argv[2]);

    if (socket < 0) {
        return -1;
    }
    
    bool stay = true;
    printf("+-----------------------------------------------------------+\n");
    printf("|                  WELCOME TO CHAT SERVER                   |\n");
    printf("+-----------------------------------------------------------+\n");
    
    // REGISTRAR A UN MEN    
    if(client_register(argv[1], socket) == NULL) {
        return -1;
    }

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
  
                handle_broadcast_global(socket, argv[1]);
                
                break;
            case 2:
                printf("+-----------------------------------------------------------+\n");
                printf("|                       PRIVATE CHAT                        |\n");
                printf("+-----------------------------------------------------------+\n");
                
                cJSON *connected_users = cJSON_Duplicate(client_list(socket, argv[1]), 1);
                char user[20];

                if (connected_users == NULL) {
                    return -1;
                }

                print_users(connected_users); // To-Do: Fix this function to receive cJSON Array
                
                printf("Type the name of the user to chat with: ");
                scanf("%20s", &user);
                
                cJSON *user_to_dm = client_info(user, socket);
                if (user_to_dm == NULL) {
                    printf("\nError getting user");
                
                } else {
                    char *username_dm = user;
                    handle_dm(socket, argv[1], username_dm);
                } 


                break;
            case 3:
                printf("+-----------------------------------------------------------+\n");
                printf("|                       CHANGE STATUS                       |\n");
                printf("+-----------------------------------------------------------+\n");
                // can change from active, ocuppied or inactive
                int new_state;
                char selected_state[10];
                printf("Select your new status:\n1. Active\n2. Bussy\n");
                scanf("%d", &new_state);
                strcpy(selected_state, (new_state == 1) ? "Activo" : "Ocupado");
                client_state(argv[1], selected_state, socket);
                break;
            case 4:
                printf("+-----------------------------------------------------------+\n");
                printf("|                 LIST ALL USERS CONNECTED                  |\n");
                printf("+-----------------------------------------------------------+\n");
                cJSON *main_list = cJSON_Duplicate(client_list(socket, argv[1]), 1);
                
                if (main_list == NULL) {
                    return -1;
                }

                print_users(main_list);


                break;
            case 5:
                printf("+-----------------------------------------------------------+\n");
                printf("|                   SEE INFO ABOUT A USER                   |\n");
                printf("+-----------------------------------------------------------+\n");
                char str[100];

                cJSON *info_list = cJSON_Duplicate(client_list(socket, argv[1]), 1);
                
                if (info_list == NULL) {
                    return -1;
                }
                printf("\n Select a User");
                print_users(info_list);
                
                printf("\n Write the username\n  -> ");
                scanf("%[^\n]s",str);
                
                cJSON *user_info = client_info(str, socket);
                if (user_info == NULL) {
                    printf("\n");
                } 


                print_user_info(user_info);
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
                handle_exit(socket, argv[1]);
                stay = false;
                break;
            default:
                printf("Not an option :/\n");
        }
    }
    close(socket);
    return 0;
}
