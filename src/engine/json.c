#include "json.h"

void json_read(const char* filename, json_t** root) {
    json_error_t error;
    *root = json_load_file(filename, 0, &error);
    if (!*root) {
        fprintf(stderr, "Error reading JSON file %s: %s\n", filename, error.text);
        exit(EXIT_FAILURE);
    }
}

void json_write(const char* filename, json_t* root) {
    if (json_dump_file(root, filename, JSON_INDENT(4)) < 0) {
        fprintf(stderr, "Error writing JSON file %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void json_free(json_t* root) {
    if (root) {
        json_decref(root);
    }
}