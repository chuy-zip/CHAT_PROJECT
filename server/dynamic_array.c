typedef struct {
    cJSON **array; // Arreglo de punteros a cJSON
    size_t used;
    size_t size;
} Array;

void init_array(Array *a, size_t initial_size) {
    a->array = malloc(initial_size * sizeof(cJSON *));
    a->used = 0;
    a->size = initial_size;
}

void insert_array(Array *a, cJSON *element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(cJSON *));
    }
    a->array[a->used] = cJSON_Duplicate(element, 1);
    a->used++;
}

void free_array(Array *a) {
    for (size_t i = 0; i < a->used; i++) {
        cJSON_Delete(a->array[i]);
    }
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

void remove_array(Array *a, size_t index) {
    if (index >= a->used) {
        fprintf(stderr, "Error: Índice fuera de rango.\n");
        return;
    }

    cJSON_Delete(a->array[index]);

    for (size_t i = index; i < a->used - 1; i++) {
        a->array[i] = a->array[i + 1];
    }

    a->used--;
}

void remove_client(Array* client_list, int client_socket) {
    for (size_t i = 0; i < client_list->used; i++) {
        cJSON *client = client_list->array[i];
        cJSON *socket_json = cJSON_GetObjectItem(client, "socket");
        if (socket_json != NULL && socket_json->valueint == client_socket) {
            remove_array(client_list, i);  
            break;
        }
    }
}
