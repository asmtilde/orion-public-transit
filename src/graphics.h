#pragma once
#include <stdbool.h>
#include <SDL2/SDL.h>

bool graphics_init(const char* title, int width, int height);
void graphics_clear();
void graphics_present();
void graphics_quit();

// New functions for sprites
SDL_Texture* graphics_load_texture(const char* filepath);
void graphics_draw_texture(SDL_Texture* texture, int x, int y, int w, int h);
void graphics_draw_color(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void graphics_draw_background(const char* filepath);