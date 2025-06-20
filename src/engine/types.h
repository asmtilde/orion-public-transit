#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define TICKS_PER_SECOND 30
#define MS_PER_TICK (1000 / TICKS_PER_SECOND)

#define PALETTE_SIZE 256
#define TILE_SIZE 32
#define MAP_WIDTH (WINDOW_WIDTH / TILE_SIZE)
#define MAP_HEIGHT (WINDOW_HEIGHT / TILE_SIZE)

#define MAXIMUM_TEXT_LENGTH 512
#define MAXIMUM_ACTORS 100

#define PI 3.14159265358979323846

typedef struct
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

typedef struct
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} Color_RGBA;

typedef struct
{
    Color_RGBA colors[PALETTE_SIZE];
    char *name[MAXIMUM_TEXT_LENGTH];
} Palette;

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    int running;
    int tps;
} App;

typedef struct
{
    int data [MAP_HEIGHT][MAP_WIDTH];
} Map;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} Bounding_Box;

typedef struct 
{
    int x;
    int y;
    int width;
    int height;
    SDL_Texture* texture;
    Bounding_Box bounding_box;
} Actor;

#endif