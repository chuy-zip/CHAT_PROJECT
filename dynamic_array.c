#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

typedef struct
{
    cJSON **array;
    size_t used;
    size_t size;
} Array;

void init_array(Array *a, size_t initial_size)
{
    a->array = malloc(initial_size * sizeof(cJSON *));
    if (!a->array) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    a->used = 0;
    a->size = initial_size;
}

void insert_array(Array *a,  cJSON *element)
{
    if (a->used == a->size) {
        a->size *= 2;
        cJSON **temp = realloc(a->array, a->size * sizeof(cJSON *));
        if (!temp) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        a->array = temp;
    }
    a->array[a->used++] = element;
}

void free_array(Array *a)
{    
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}