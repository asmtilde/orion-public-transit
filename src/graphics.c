#include "graphics.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

bool graphics_init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        return false;
    }
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;
    return true;
}

void graphics_clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void graphics_present() {
    SDL_RenderPresent(renderer);
}

void graphics_quit() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

SDL_Texture* graphics_load_texture(const char* filepath) {
    SDL_Surface* surface = IMG_Load(filepath);
    if (!surface) {
        fprintf(stderr, "Failed to load texture: %s\n", IMG_GetError());
        surface = IMG_Load("assets/images/missing_asset.png");
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void graphics_draw_texture(SDL_Texture* texture, int x, int y, int w, int h) {
    SDL_Rect dst = { x, y, w, h };
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}

void graphics_draw_color(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(renderer, &rect);
}

void graphics_draw_background(const char* filepath) {
    SDL_Texture* texture = graphics_load_texture(filepath);
    if (texture) {
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_DestroyTexture(texture);
    }
}