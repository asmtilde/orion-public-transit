// ... I don't know if anyone can hear me
// ... but if you can, please come and find me
// ... I am waiting for you
// ... Best of luck, ALEPH

/*
    Orion Public Transit
    A game by Jack Spencer and WHJ Studios (c) 2025

    Congratulations! This is the source code for everything that makes this piece of software work.
    Orion Public Transit is a role-playing game that combines weird video game shower thoughts I've had
    with a love for the classic JRPG genre. The source code is written in C, using the SDL2 library for graphics and audio.

    Please note, the code and scripts for the game will contain spoilers for the game itself.
    If you want to play the game without spoilers, please play the game first before reading the source code.

    Assets and resources used in this game are not available in the source code.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <yaml.h>
#include <string.h>

// *** Configuration Constants ***

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define TILE_SIZE 32
#define FPS 60
#define MAX_ASSETS 512

// *** Structures and Global Variables ***

typedef struct {
    char name[64];
    char path[256];
} Asset;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} App;

typedef struct {
    int width[WINDOW_WIDTH / TILE_SIZE];
    int height[WINDOW_HEIGHT / TILE_SIZE];
    int tiles[WINDOW_WIDTH / TILE_SIZE][WINDOW_HEIGHT / TILE_SIZE];
} TileMap;

typedef struct {
    int x, y;
    int w, h;
    int speed;
    int velx, vely;
} Player;

typedef struct {
    int turn;               // Turn count
    bool player_turn;       // Is it the player's turn?
    bool battle_is_ambush;  // If ambushed, battle theme is BREAK FREE, otherwise play F*** IT
} BattleValues;

static Mix_Music* current_music = NULL;

static bool running = true;

Asset tile_assets[MAX_ASSETS];
Asset sprite_assets[MAX_ASSETS];
Asset music_assets[MAX_ASSETS];
Asset sfx_assets[MAX_ASSETS];

bool read_tilemap(const char* filepath, TileMap* map) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Failed to open tilemap file");
        return false;
    }

    for (int y = 0; y < WINDOW_HEIGHT / TILE_SIZE; y++) {
        for (int x = 0; x < WINDOW_WIDTH / TILE_SIZE; x++) {
            if (fscanf(file, "%d", &map->tiles[x][y]) != 1) {
                fclose(file);
                return false;
            }
        }
    }

    fclose(file);
    return true;
}

bool audio_init()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    return true;
}

void audio_quit()
{
    if (current_music) {
        Mix_FreeMusic(current_music);
        current_music = NULL;
    }
    Mix_CloseAudio();
}

bool audio_play_music(const char* filename, int loops)
{
    if (current_music) {
        Mix_FreeMusic(current_music);
        current_music = NULL;
    }
    current_music = Mix_LoadMUS(filename);
    if (!current_music) {
        printf("Failed to load music: %s\n", Mix_GetError());
        return false;
    }
    if (Mix_PlayMusic(current_music, loops) == -1) {
        printf("Failed to play music: %s\n", Mix_GetError());
        return false;
    }
    return true;
}

bool audio_is_music_playing()
{
    return Mix_PlayingMusic() != 0;
}

void audio_pause_music()
{
    if (Mix_PlayingMusic()) {
        Mix_PauseMusic();
    }
}

void audio_resume_music()
{
    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
}

void audio_stop_music()
{
    Mix_HaltMusic();
}

static inline float noise(int x, int y, int t)
{
    int n = x + y * 57 + t * 131;
    n = (n << 13) ^ n;
    int nn = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0f - ((float)nn / 1073741824.0f);
}

bool load_assets_yaml(const char* filename) {
    FILE *fh = fopen(filename, "r");
    if (!fh) {
        fprintf(stderr, "Failed to open YAML file: %s\n", filename);
        return false;
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
    return true;
}

const char* search_assets(Asset* assets, const char* name)
{
    for (int i = 0; i < MAX_ASSETS; i++)
    {
        if (strcmp(assets[i].name, name) == 0) {
            return assets[i].path;
        }
    }
    return NULL;
}

bool init_sdl(App* app, const char* title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!app->window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void close_sdl(App* app) {
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    SDL_Quit();
}

void player_controller(Player* player, const Uint8* keystate) {
    player->velx = 0;
    player->vely = 0;

    if (keystate[SDL_SCANCODE_W]) player->vely = -player->speed;
    if (keystate[SDL_SCANCODE_S]) player->vely =  player->speed;
    if (keystate[SDL_SCANCODE_A]) player->velx = -player->speed;
    if (keystate[SDL_SCANCODE_D]) player->velx =  player->speed;
}

void draw_grid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    // vertical lines
    for (int x = 0; x < WINDOW_WIDTH; x += TILE_SIZE) {
        SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
    }

    // horizontal lines
    for (int y = 0; y < WINDOW_HEIGHT; y += TILE_SIZE) {
        SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);
    }
}

int main(int argc, char* argv[]) {
    // *** Initialize SDL and Audio ***
    printf("Initializing SDL and Audio...\n");
    App app;
    if (!init_sdl(&app, "Orion Public Transit")) {
        return EXIT_FAILURE;
    }

    if (!audio_init()) {
        close_sdl(&app);
        return EXIT_FAILURE;
    }
    printf("SDL and Audio initialized successfully.\n");

    // *** Load Assets ***
    printf("Loading assets from YAML...\n");
   
    if (!load_assets_yaml("assets.yaml")) {
        audio_quit();
        close_sdl(&app);
        return EXIT_FAILURE;
    }
    printf("Assets loaded successfully.\n");

    // *** Initialize Player ***
    Player player = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE, .speed = 5, .velx = 0, .vely = 0 };
    printf("Player initialized at position (%d, %d).\n", player.x, player.y);

    // *** Main Loop ***
    printf("Entering main loop...\n");
    SDL_Event event;
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }
        player_controller(&player, keystate);
        player.x += player.velx;
        player.y += player.vely;
        if (player.x < 0) player.x = 0;
        if (player.y < 0) player.y = 0;
        if (player.x + player.w > WINDOW_WIDTH) player.x = WINDOW_WIDTH - player.w;
        if (player.y + player.h > WINDOW_HEIGHT) player.y = WINDOW_HEIGHT - player.h;
        SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app.renderer);
        draw_grid(app.renderer);
        SDL_Rect player_rect = { player.x, player.y, player.w, player.h };
        SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(app.renderer, &player_rect);
        SDL_RenderPresent(app.renderer);
        SDL_Delay(1000 / FPS);
    }
    printf("Exiting main loop...\n");
    // *** Cleanup ***
    audio_quit();
    close_sdl(&app);
    printf("Cleanup complete. Exiting program.\n");
    return EXIT_SUCCESS;
}
