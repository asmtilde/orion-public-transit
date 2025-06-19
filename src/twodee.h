/*
    TwoDee holds all of the code for 2D rendering sections, such as:
        - top down exploration
        - side scrolling loading animations
        - side scrolling exploration
        - RPG battle animations
        - interactive menus
        - and anything else that requires 2D rendering.
*/

#ifndef TWODEE_H
#define TWODEE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "types.h"

int create_scene(SDL_Renderer *renderer);

#endif