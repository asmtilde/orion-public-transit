#include "game.h"
#include "graphics.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "audio.h"
#include "script.h"

static bool running = true;

bool game_init(const char* title, int width, int height)
{
    if (!graphics_init(title, width, height)) {
        return false;
    }
    if (!audio_init()) return false;

    return true;
}

void game_loop()
{
    SDL_Event e;
    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
        graphics_clear();
        graphics_present();
        SDL_Delay(16);
    }
}

void game_quit()
{
    audio_quit();
    graphics_quit();
}