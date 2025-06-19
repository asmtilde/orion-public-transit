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

    window = SDL_CreateWindow("SDL Window",
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

void destroy_window(SDL_Window* window)
{
    if (window != NULL) {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

int main(int argc, char* argv[])
{
    SDL_Window* window = create_window();
    if (window == NULL) {
        return 1; // Exit if window creation failed
    }

    // Main loop flag
    int running = 1;
    SDL_Event event;

    // Main loop
    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = 0; // Exit the loop on quit event
            }
        }
        // Here you can add rendering code if needed
    }

    destroy_window(window);
    return 0;
}