#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[64];
    char path[256];
} Asset;

#define MAX_ASSETS 512
extern Asset tile_assets[MAX_ASSETS];
extern Asset sprite_assets[MAX_ASSETS];
extern Asset music_assets[MAX_ASSETS];
extern Asset sfx_assets[MAX_ASSETS];

void load_assets_yaml(const char* filename);
const char* search_assets(Asset* assets, const char* name);