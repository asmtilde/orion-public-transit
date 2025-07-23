#pragma once 
#include <stdbool.h>

bool game_init(const char* title, int width, int height);
void game_loop();
void game_quit();