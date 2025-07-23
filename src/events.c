#include "events.h"
#include <jansson.h>
#include <stdio.h>
#include <string.h>

void load_events(const char* filename) {
    json_error_t error;
    json_t* root = json_load_file(filename, 0, &error);
    if (!root) {
        fprintf(stderr, "JSON error: %s\n", error.text);
        return;
    }

    json_t* events = json_object_get(root, "events");
    if (json_is_array(events)) {
        size_t i;
        json_t* event;
        json_array_foreach(events, i, event) {
            const char* type = json_string_value(json_object_get(event, "type"));
            if (strcmp(type, "dialog") == 0) {
                const char* text = json_string_value(json_object_get(event, "text"));
                printf("Dialog: %s\n", text);
            }
            // ... handle other event types
        }
    }
    json_decref(root);
}