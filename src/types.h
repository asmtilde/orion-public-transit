#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define TILE_SIZE 32
#define MAP_WIDTH (WINDOW_WIDTH / TILE_SIZE)
#define MAP_HEIGHT (WINDOW_HEIGHT / TILE_SIZE)

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