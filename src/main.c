#include <SDL2/SDL.h>
#include <stdio.h>

/*
    And you don't seem to understand

    The pencil may be mightier than the sword,
    But the keyboard has the developer smashing his head into it.

    This is the entry point for Orion Public Transit.
*/

#include "types.h"

SDL_Window* create_window()
{
    SDL_Window* window = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return NULL;
    } 

    window = SDL_CreateWindow("Orion Public Transit",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN);

    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    return window;
}

SDL_Renderer* create_renderer(SDL_Window* window)
{
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }
    return renderer;
}

void destroy_window(SDL_Window* window)
{
    if (window != NULL) {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

void render(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
    SDL_Window* window = create_window();

    if (window == NULL) {
        return 1;
    }

    SDL_Renderer* renderer = create_renderer(window);

    if (renderer == NULL) {
        destroy_window(window);
        return 1;
    }

    int running = 1;
    SDL_Event event;

    // Main loop
    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        render(renderer);
    }

    destroy_window(window);
    return 0;
}