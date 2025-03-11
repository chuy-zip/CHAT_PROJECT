#include <stdio.h>
#include <cjson/cJSON.h>
#include <stdlib.h>

int main() {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "nombre", "Juan");
    cJSON_AddNumberToObject(root, "edad", 30);
    cJSON_AddBoolToObject(root, "estudiante", 1);

    char *json_string = cJSON_Print(root);
    printf("JSON creado: %s\n", json_string);

    const char *json_to_parse = "{\"ciudad\":\"Madrid\",\"pais\":\"España\"}";
    cJSON *parsed_json = cJSON_Parse(json_to_parse);
    if (parsed_json == NULL) {
        printf("Error al parsear JSON\n");
        return 1;
    }

    cJSON *ciudad = cJSON_GetObjectItemCaseSensitive(parsed_json, "ciudad");
    cJSON *pais = cJSON_GetObjectItemCaseSensitive(parsed_json, "pais");
    printf("Ciudad: %s, País: %s\n", ciudad->valuestring, pais->valuestring);

    cJSON_Delete(root);
    cJSON_Delete(parsed_json);
    free(json_string);

    return 0;
}
