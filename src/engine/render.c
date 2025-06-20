#include "render.h"

void change_background_color(App *app, Color color)
{
    if (app && app->renderer) {
        SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, 255);
        SDL_RenderClear(app->renderer);
        SDL_RenderPresent(app->renderer);
    }
}