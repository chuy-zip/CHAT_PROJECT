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