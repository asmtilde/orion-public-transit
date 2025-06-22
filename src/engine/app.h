#ifndef WINDOW_H
#define WINDOW_H

#include "types.h"

App *app_create();
void app_destroy(App *app);
// Code for these will be in main.c
void game_tick(App *app);
void render(App *app);

#endif