
#include <stdio.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

#include "client/client_connection.h"
#include "client/client_register.h"
#include "client/client_list.h"

void print_users(cJSON *users_list) {
    char *respuesta = users_list->valuestring;

    cJSON *respuesta_parseada = cJSON_Parse(respuesta);

    cJSON *cliente;
    cJSON_ArrayForEach(cliente, respuesta_parseada) {
        if (cJSON_IsString(cliente)) {
            cJSON *cliente_json = cJSON_Parse(cliente->valuestring);
            if (cliente_json != NULL) {
                cJSON *usuario = cJSON_GetObjectItem(cliente_json, "usuario");
                cJSON *ip = cJSON_GetObjectItem(cliente_json, "direccionIP");
                cJSON *socket = cJSON_GetObjectItem(cliente_json, "socket");
                cJSON *estado = cJSON_GetObjectItem(cliente_json, "estado");

                if (usuario && ip && socket && estado) {
                    printf("\nUsuario: %s\n", usuario->valuestring);
                    printf("IP: %s\n", ip->valuestring);
                    printf("Socket: %s\n", socket->valuestring);
                    printf("Estado: %s\n", estado->valuestring);
                    printf("----------------------\n");
                }

                cJSON_Delete(cliente_json);
            }
        }
    }    
}

int main()
{
    bool stay = true;
    int socket = client_connection(50213, "0.0.0.0");

    if (socket < 0) {
        return -1;
    }

    printf("+-----------------------------------------------------------+\n");
    printf("|                  WELCOME TO CHAT SERVER                   |\n");
    printf("+-----------------------------------------------------------+\n");

    // REGISTRAR A UN MEN
    char client_name[20];
    printf("Enter your name: ");
    scanf("%s", client_name);
    
    if(client_register(client_name, socket) == NULL) {
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
        
        switch (chatSelection) {
            case 1:
                printf("+-----------------------------------------------------------+\n");
                printf("|                        GLOBAL CHAT                        |\n");
                printf("+-----------------------------------------------------------+\n");
                // show all messages, every second refresh
                // send messages
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
                
                cJSON *main_list = cJSON_Duplicate(client_list(socket), 1);
                
                if (main_list == NULL) {
                    return -1;
                }

                print_users(cJSON_GetObjectItem(main_list, "respuesta"));

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
                stay = false;
                break;
            default:
                printf("Not an option :/\n");
                break;
        }
    }
    
    
    return 0;
}