#ifndef APP_H
#define APP_H

/*
    TODO:

    - Implement a error handling system
    - Add support for multiple audio formats (WAV, OGG, MP3)
    - Implement a resource manager that reads from a assets/manifest file
    - Add support for loading and saving game state
    - Implement a simple scripting system for game logic
    - Add support for loading and rendering 3D models, possibly using OpenGL
    - Split the code into multiple files for better organization
    - Add seperate window and renderer management
    - Implement a simple event system for handling user input
    - Have everything use a single SDL_Renderer instance
    
    Palettes should be loaded from a .pal file format, and should be able to be applied to textures.
    Specifically for the 2D map tiles, which will be rendered using a palette.
    The palette should be able to be changed at runtime, and should be able to be applied to textures.

    Party should be managed by a party manager, which will handle the party members, their stats, and their inventory.
    Save data should be stored in a format that allows for easy loading and saving of game state.

    Z-ordering should be implemented for actors, so that actors can be rendered in the correct order.
    This will allow for actors to be rendered in the correct order, and will allow for actors to be rendered in the correct order based on their position in the world.

    ARGHH WHY DOES CODING HAVE TO BE SO DAMN COMPLICATED
    I just want to make a game, not a fucking engine.

    ZzZzZzZz

    asmtilde
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WINDOW_TITLE "Orion Public Transit"

#define MS_PER_TICK 16
#define TICKS_PER_SECOND 30

#define MAX_TEXT_LENGTH 512
#define MAX_TEXT_LINES 4
#define MAX_FONTS 16
#define MAX_ACTORS 64
#define MAX_ACTOR_NAME_LENGTH 64

#define PALETTE_SIZE 16
#define TILE_SIZE 32

#define MAP_WIDTH (WINDOW_WIDTH / TILE_SIZE)
#define MAP_HEIGHT (WINDOW_HEIGHT / TILE_SIZE)

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFER_SIZE 2048
#define AUDIO_MAX_VOLUME 100

typedef struct {
    Uint8 r, g, b, a;
} Color;

typedef struct {
    Color colors[PALETTE_SIZE];
} Palette;

typedef struct {
    TTF_Font* font;
    char name[128];
    int size;
} Font;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int running;
    int tps;
    Color background_color;
} App;

typedef struct {
    int x, y, width, height;
} Bounding_Box;

typedef struct {
    int data[MAP_HEIGHT][MAP_WIDTH];
} Map;

typedef struct {
    int x, y, width, height;
    SDL_Texture* texture;
    Bounding_Box bounding_box;
    int visible;
} Actor;

/* Function Prototypes */


// App Functions
App* app_create();
void app_destroy(App* app);
void app_set_background_color(App* app, Color color);
void app_clear(App* app);
void app_present(App* app);
void app_set_title(App* app, const char* title);
void app_set_icon(App* app, const char* icon_path);

// Font Functions
int font_load(const char* path, int size, const char* font_name);
void font_unload_all();
int font_set_active(const char* font_name);
TTF_Font* font_get_active();
void draw_text(App* app, const char* text, int x, int y, SDL_Color color);
void draw_text_multiline(App* app, const char* text, int x, int y, SDL_Color color);

// Actor Functions
void actor_init(Actor* actor, int x, int y, int w, int h, SDL_Texture* tex);
void actor_render(App* app, const Actor* actor);
void actor_move(Actor* actor, int dx, int dy);
int actor_collides(const Actor* a, const Actor* b);
void actor_set_visibility(Actor* actor, int visible);

// Map Functions
void map_init(Map* map, int value);
void map_render(App* app, const Map* map, SDL_Texture* tile_texture);
void map_set_tile(Map* map, int x, int y, int value);
int map_get_tile(const Map* map, int x, int y);

// Texture Functions
SDL_Texture* load_texture(App* app, const char* path);
void texture_unload(SDL_Texture* texture);

// Render Functions
void render_clear(App* app);
void render_set_palette(App* app, const Palette* palette);

// Audio Functions
int audio_init();
void audio_quit();
void audio_play_sound(const char* path);
void audio_set_volume(int volume);

// Utility Functions
void log_error(const char* message);
void log_info(const char* message);
void log_debug(const char* message);

#endif