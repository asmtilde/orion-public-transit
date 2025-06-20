#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define TICKS_PER_SECOND 30
#define MS_PER_TICK (1000 / TICKS_PER_SECOND)

#define TILE_SIZE 32
#define MAP_WIDTH (WINDOW_WIDTH / TILE_SIZE)
#define MAP_HEIGHT (WINDOW_HEIGHT / TILE_SIZE)

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
} Color;

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    int running;
    int tps;
} App;

typedef struct {
    int data [MAP_HEIGHT][MAP_WIDTH];
} Map;

struct Entity {
    int x;
    int y;
    int facing;
    SDL_Texture* texture;
};

typedef struct {
    struct Entity* entities;
    int entity_count;
} EntityManager;

#endif