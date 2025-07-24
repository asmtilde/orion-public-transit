#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[64];
    char path[256];
} Asset;

#define MAX_ASSETS 512
Asset tile_assets[MAX_ASSETS];
Asset sprite_assets[MAX_ASSETS];
Asset music_assets[MAX_ASSETS];
Asset sfx_assets[MAX_ASSETS];

void load_assets_yaml(const char* filename) {
    FILE *fh = fopen(filename, "r");
    if (!fh) {
        perror("Failed to open yaml");
        return;
    }

    yaml_parser_t parser;
    yaml_event_t event;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, fh);

    enum { NONE, TILES, SPRITES, MUSIC } section = NONE;
    Asset* current_array = NULL;
    int asset_idx = 0;
    char last_key[64] = "";

    while (1) {
        yaml_parser_parse(&parser, &event);

        if (event.type == YAML_STREAM_END_EVENT)
            break;

        if (event.type == YAML_MAPPING_START_EVENT)
            continue;

        if (event.type == YAML_SCALAR_EVENT) {
            char* val = (char*)event.data.scalar.value;
            if (strcmp(val, "tiles") == 0) {
                section = TILES; asset_idx = 0; current_array = tile_assets;
            } else if (strcmp(val, "sprites") == 0) {
                section = SPRITES; asset_idx = 0; current_array = sprite_assets;
            } else if (strcmp(val, "music") == 0) {
                section = MUSIC; asset_idx = 0; current_array = music_assets;
            } else if (strcmp(val, "sfx") == 0) {
                section = NONE; asset_idx = 0; current_array = sfx_assets;
            } else if (section == NONE) {
                continue;
            } else if (current_array) {
                if (strlen(last_key) == 0) {
                    strncpy(last_key, val, sizeof(last_key));
                } else {
                    strncpy(current_array[asset_idx].name, last_key, sizeof(current_array[asset_idx].name));
                    strncpy(current_array[asset_idx].path, val, sizeof(current_array[asset_idx].path));
                    asset_idx++;
                    last_key[0] = '\0';
                }
            }
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(fh);
}

const char* search_assets(Asset* assets, const char* name) {
    for (int i = 0; i < MAX_ASSETS; i++) {
        if (strcmp(assets[i].name, name) == 0) {
            return assets[i].path;
        }
    }
    return NULL;
}