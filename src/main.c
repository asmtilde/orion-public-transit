#include "engine/window.h"
#include "engine/json.h"

int main(int argc, char* argv[])
{
    json_t* config = NULL;
    json_read("assets/config.json", &config);

    json_t* welcome_message = json_object_get(config, "welcome_message");
    if (json_is_string(welcome_message)) {
        printf("%s\n", json_string_value(welcome_message));
    } else {
        fprintf(stderr, "Error: welcome_message is not a string or is missing.\n");
    }

    json_free(config);

    return 0;
}