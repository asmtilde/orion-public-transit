#include "engine/app.h"
#include "engine/render.h"

Color colors[]= {
    {255, 0, 0},   // Red
    {0, 255, 0},   // Green
    {0, 0, 255},   // Blue
    {255, 255, 0}, // Yellow
    {255, 165, 0}, // Orange
    {128, 0, 128}  // Purple
};

void game_tick(App *app)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                app->running = 0;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    app->running = 0;
                }
                break;
            default:
                break;
        }
    }
}

void render(App *app)
{
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    change_background_color(app, colors[2]);

    SDL_RenderPresent(app->renderer);
}

/*
    Main function to initialize the application, handle events, update game state, and render frames.
*/
int main(int argc, char *argv[])
{
    App *app = app_create();
    if (!app) return 1;

    while (app->running)
    {
        game_tick(app);
        render(app);
        SDL_Delay(MS_PER_TICK);
    }

    app_destroy(app);
    return 0;
}