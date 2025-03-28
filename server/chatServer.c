#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>  // isdigit()
#include <pthread.h>
#include <cjson/cJSON.h>
#include <sys/socket.h>
#include <time.h>

#include "register_response.c"
#include "info_response.c"
#include "state_response.c"
#include "list_response.c"
#include "send_response.c"
#include "receive_response.c"

#include "dynamic_array.c"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024 
pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    cJSON *username;
    time_t last_active;
    Array *client_array;
} client_info_t;

void* handle_inactivity(void* arg) {
    client_info_t* client_info = (client_info_t*)arg;
    int client_socket = client_info->socket;
    Array *client_list = client_info->client_array;
    
    while (1) {
        sleep(1);
        
        time_t last_active = client_info->last_active;
        time_t current_time = time(NULL);

        if (current_time - last_active > 10) {
            bool user_found = false;
            int user_index = -1;

            for (size_t i = 0; i < client_list->used; i++) {
                cJSON *client_list_name = cJSON_GetObjectItem(client_list->array[i], "usuario");

                if (strcmp(client_info->username->valuestring, client_list_name->valuestring) == 0) {
                    user_found = true;
                    user_index = i;
                    break;
                }
            }

            if (!user_found) {
                printf("Unable to find user: %s\n", client_info->username->valuestring);
            
            } else if (strcmp(cJSON_GetObjectItem(client_list->array[user_index], "estado")->valuestring, "Inactivo") != 0) {
                printf("Inactive user found\n: %s\n", client_info->username->valuestring);
                pthread_mutex_lock(&client_list_mutex);
                cJSON_ReplaceItemInObjectCaseSensitive(
                    client_list->array[user_index], 
                    "estado", 
                    cJSON_CreateString("Inactivo")
                );
                pthread_mutex_unlock(&client_list_mutex);
            }
        }
    }
    return NULL;
}


void* handle_client(void* arg) {
    client_info_t* client_info = (client_info_t*)arg;
    int client_socket = client_info->socket;
    Array *client_list = client_info->client_array;
    char buffer[BUFFER_SIZE] = {0};
    char* welcome_message = "\nServer got message\n";

    while (1) {
        // reading user 
        int valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("\nClient disconnected.\n");
            pthread_mutex_lock(&client_list_mutex);
            remove_client(client_list, client_socket);
            pthread_mutex_unlock(&client_list_mutex);
            break;
        }
        
        // raw message
        printf("\nReceived raw message: %s\n", buffer);
        
        for (size_t i = 0; i < client_list->used; i++) {
            char *client_list_str = cJSON_Print(client_list->array[i]);
            printf("\nCliente %zu:\n%s\n", i + 1, client_list_str);
            free(client_list_str);
        }
        
        // json parsing
        cJSON *client = cJSON_Parse(buffer);
        if (client == NULL) {
            printf("Error al parsear JSON. \n");
            continue;
        }
        
        cJSON_AddNumberToObject(client, "socket", client_socket);
        
        // printing json
        char *json_str = cJSON_Print(client);
        printf("\nParsed JSON: %s\n", json_str);
        free(json_str);
        
        // checking tipo of json
        cJSON *tipo = cJSON_GetObjectItemCaseSensitive(client, "tipo");
        cJSON *client_name = cJSON_GetObjectItem(client, "usuario");
        cJSON *client_ip = cJSON_GetObjectItem(client, "direccionIP");
        
        // Verificando "endpoints"
        if (tipo != NULL && strcmp(tipo->valuestring, "REGISTRO") == 0) {
            bool repeated_flag = false;
            
            for (size_t i = 0; i < client_list->used; i++) {
                cJSON *client_list_name = cJSON_GetObjectItem(client_list->array[i], "usuario");
                cJSON *client_list_ip = cJSON_GetObjectItem(client_list->array[i], "direccionIP");
                
                if (strcmp(client_name->valuestring, client_list_name->valuestring) == 0 || strcmp(client_ip->valuestring, client_list_ip->valuestring) == 0) {
                    repeated_flag = true;
                    break;
                }
            }
            
            if(register_response(client_socket, buffer, repeated_flag) < 0 || repeated_flag == true) {
                printf("Unable to register");
                
            } else {
                cJSON_DeleteItemFromObject(client, "tipo");
                cJSON_AddStringToObject(client, "estado", "Activo");
                pthread_mutex_lock(&client_list_mutex);
                insert_array(client_list, client);
                pthread_mutex_unlock(&client_list_mutex);
                printf("\nUser registered successfully!\n");
                client_info->username = cJSON_Duplicate(client_name, 1);
                pthread_t inactive_thread;
                if (pthread_create(&inactive_thread, NULL, handle_inactivity, (void*)client_info) != 0) {
                    perror("Error creating thread");
                    free(client_info);
                }
                for (size_t i = 0; i < client_list->used; i++) {
                    char *client_json_str = cJSON_Print(client_list->array[i]);
                    printf("\nCliente %zu:\n%s\n", i + 1, client_json_str);
                    free(client_json_str);
                }
            }

            client_info->last_active = time(NULL);
    
        }   else if (tipo != NULL && strcmp(tipo->valuestring, "EXIT") == 0) {
                // free the user
                cJSON *usuario = cJSON_GetObjectItemCaseSensitive(client, "usuario");
                if (usuario != NULL) {
                    printf("User: %s requested to exit.\n", usuario->valuestring);
                }
                pthread_mutex_lock(&client_list_mutex);
                remove_client(client_list, client_socket);
                pthread_mutex_unlock(&client_list_mutex);
                cJSON_Delete(client); 
                break;
        
        }   else if (tipo != NULL && strcmp(tipo->valuestring, "MOSTRAR") == 0) {
                bool user_flag = false;
                cJSON *user_to_return = cJSON_CreateObject();

                for (size_t i = 0; i < client_list->used; i++) {
                    cJSON *client_list_name = cJSON_GetObjectItem(client_list->array[i], "usuario");

                    if (strcmp(client_name->valuestring, client_list_name->valuestring) == 0) {
                        user_flag = true;
                        user_to_return = cJSON_Duplicate(client_list->array[i], 1);
                        break;
                    }
                }
                
                if(info_response(client_socket, buffer, BUFFER_SIZE, user_flag, user_to_return) < 0 || user_flag == false) {
                    printf("Unable to find user");   
                
                } else {
                    printf("User found\n");
                }

                client_info->last_active = time(NULL);
        
            }   else if (tipo != NULL && strcmp(tipo->valuestring, "ESTADO") == 0) {
                bool user_flag = false;
                int user_to_change_index;

                for (size_t i = 0; i < client_list->used; i++) {
                    cJSON *client_list_name = cJSON_GetObjectItem(client_list->array[i], "usuario");

                    if (strcmp(client_name->valuestring, client_list_name->valuestring) == 0) {
                        user_flag = true;
                        user_to_change_index = i;
                        break;
                    }
                }

                cJSON *state_to_change = cJSON_Parse(buffer);
                cJSON *state = cJSON_GetObjectItem(client, "estado");

                if(state_response(client_socket, buffer, user_flag) < 0 || user_flag == false) {
                    printf("Unable to find user");   
                
                } else {
                    printf("User found\n");
                    cJSON *state_to_change = cJSON_GetObjectItem(client_list->array[user_to_change_index], "estado");
                    cJSON_ReplaceItemInObjectCaseSensitive(client_list->array[user_to_change_index], "estado", cJSON_CreateString(state->valuestring));
                }

                client_info->last_active = time(NULL);
        
            }
        
        cJSON *accion = cJSON_GetObjectItemCaseSensitive(client, "accion");
        
        if (accion != NULL && strcmp(accion->valuestring, "BROADCAST") == 0) {
            // free the user
            cJSON *nombre_emisor = cJSON_GetObjectItemCaseSensitive(client, "nombre_emisor");
            cJSON *mensaje = cJSON_GetObjectItemCaseSensitive(client, "mensaje");
            if (nombre_emisor != NULL && mensaje != NULL) {
                printf("User: %s just send the message: %s\n", nombre_emisor->valuestring, mensaje->valuestring);
                //send(client_socket, welcome_message, strlen(welcome_message), 0);
                
                cJSON *response = cJSON_CreateObject();
                cJSON_AddStringToObject(response, "accion", "BROADCAST");
                cJSON_AddStringToObject(response, "nombre_emisor", nombre_emisor->valuestring);
                cJSON_AddStringToObject(response, "mensaje", mensaje->valuestring);

                // Convertir el JSON a una cadena
                char *response_str = cJSON_Print(response);
                printf("Broadcasting message: %s\n", response_str);
                
                for (size_t i = 0; i < client_list->used; i++){
                    cJSON *client_json = client_list->array[i];
                    cJSON *client_socket_json = cJSON_GetObjectItemCaseSensitive(client_json, "socket");
                    
                    if (client_socket_json != NULL) {
                        int client_socket = client_socket_json->valueint;
                        send(client_socket, response_str, strlen(response_str), 0);
                    }
                }
                
                cJSON_Delete(response);
                free(response_str);
            }

            client_info->last_active = time(NULL);

        } else if (accion != NULL && strcmp(accion->valuestring, "DM") == 0) {
            cJSON *nombre_emisor = cJSON_GetObjectItemCaseSensitive(client, "nombre_emisor");
            cJSON *nombre_destinatario = cJSON_GetObjectItemCaseSensitive(client, "nombre_destinatario");
            cJSON *mensaje = cJSON_GetObjectItemCaseSensitive(client, "mensaje");
            if (nombre_emisor != NULL && mensaje != NULL) {
                printf("User: %s just send the message: %s to user: %s\n", nombre_emisor->valuestring, mensaje->valuestring, nombre_destinatario->valuestring);
                
                cJSON *response = cJSON_CreateObject();
                cJSON_AddStringToObject(response, "accion", "DM");
                cJSON_AddStringToObject(response, "nombre_emisor", nombre_emisor->valuestring);
                cJSON_AddStringToObject(response, "nombre_destinatario", nombre_destinatario->valuestring);
                cJSON_AddStringToObject(response, "mensaje", mensaje->valuestring);

                // Convertir el JSON a una cadena
                char *response_str = cJSON_Print(response);
                printf("Broadcasting message: %s\n", response_str);
                
                for (size_t i = 0; i < client_list->used; i++){
                    cJSON *client_json = client_list->array[i];
                    cJSON *client_socket_json = cJSON_GetObjectItemCaseSensitive(client_json, "socket");
                    cJSON *client_user_json = cJSON_GetObjectItemCaseSensitive(client_json, "usuario");
                    
                    if (strcmp(client_user_json->valuestring, nombre_destinatario->valuestring) == 0) {
                        int client_socket = client_socket_json->valueint;
                        send(client_socket, response_str, strlen(response_str), 0);
                    }
                }
                
                cJSON_Delete(response);
                free(response_str);
            }
        } 
        else if (accion != NULL && strcmp(accion->valuestring, "LISTA") == 0) {
            cJSON *users_list = cJSON_CreateArray();
            
            for (size_t i = 0; i < client_list->used; i++) {
                cJSON *username_to_list = cJSON_GetObjectItem(client_list->array[i], "usuario");
                cJSON *direccion_to_list = cJSON_GetObjectItem(client_list->array[i], "direccionIP");
                cJSON *estado_to_list = cJSON_GetObjectItem(client_list->array[i], "estado");
                char formatted_string[50];

                sprintf(formatted_string, "%s-%s-%s", username_to_list->valuestring, direccion_to_list->valuestring, estado_to_list->valuestring);
                cJSON_AddItemToArray(users_list, cJSON_CreateString(formatted_string));
            }

            if (list_response(client_socket, users_list) < 0) {
                printf("Unable to get users list");
            } else {
                printf("Success");
            }

            client_info->last_active = time(NULL);

        }
         

        
        memset(buffer, 0, BUFFER_SIZE);
        cJSON_Delete(client);
    }

    // closing socket
    close(client_socket);
    free(client_info);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    Array client_list; // Defining home-made dynamic list
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
    printf("\nServer name: %s \nListening on port: %d \n", argv[0], port);
    
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

    printf("\nServer listening on IP: %s, port: %d...\n", ip, port);

    while (1) {
        // Aceptar una nueva conexión
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // showing new connection
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("\nNew client connected from: %s\n", client_ip);

        client_info_t* client_info = (client_info_t*)malloc(sizeof(client_info_t));
        client_info->socket = new_socket;
        client_info->address = address;
        client_info->client_array = &client_list;
        client_info->last_active = time(NULL);

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

    // client socket and closing server scoket at hte end
    close(new_socket);
    close(server_fd);
    free_array(&client_list);
    
    return 0;
}
