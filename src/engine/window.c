#include "window.h"

App* app_create(const char* title, int width, int height)
{
    // Make a pointer to the App
    App *app = (App*)malloc(sizeof(App));
    if (!app) return NULL;

    app->window = NULL;
    app->renderer = NULL;
    app->running = 0;   // 0 : not, 1 : running, 2 : paused, 3 : crash
    app->tps = 30;      // TPS is 30

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        free(app);
        return NULL; // SDL initialization failed
    }

    app->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!app->window) {
        SDL_Quit();
        free(app);
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return NULL; // Window creation failed
    }

    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) {
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        free(app);
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        return NULL; // Renderer creation failed
    }

    app->running = 1; // Set the app to running state
    return app;
}

void app_destroy(App* app)
{
    if (app) {
        if (app->renderer) {
            SDL_DestroyRenderer(app->renderer);
        }
        if (app->window) {
            SDL_DestroyWindow(app->window);
        }
        SDL_Quit();
        free(app);
    }
}

void app_change_title(App* app, const char* title)
{
    if (app && title) {
        strncpy(app->title, title, sizeof(app->title) - 1);
        app->title[sizeof(app->title) - 1] = '\0'; // Ensure null termination
        SDL_SetWindowTitle(app->window, app->title);
    }
}

void app_set_icon(App* app, const char* icon_path)
{
    if (app && icon_path) {
        SDL_Surface* icon = SDL_LoadBMP(icon_path);
        if (icon) {
            SDL_SetWindowIcon(app->window, icon);
            SDL_FreeSurface(icon);
        } else {
            fprintf(stderr, "Could not load icon: %s\n", SDL_GetError());
        }
    }
}