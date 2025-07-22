#ifndef WINDOW_H
#define WINDOW_H

#pragma once    // We only want to include this header once, specifically in app.h

#include <SDL2/SDL.h>
#include <stdio.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int running;
    int tps;
    char title[256];
} App;

App* app_create(const char* title, int width, int height);
void app_destroy(App* app);
void app_change_title(App* app, const char* title);
void app_set_icon(App* app, const char* icon_path);

#endif