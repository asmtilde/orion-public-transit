#ifndef TTF_H
#define TTF_H

#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "types.h"

#define MAX_FONTS 16

extern TTF_Font* loaded_fonts[MAX_FONTS];
extern int num_loaded_fonts;

int load_fonts(const char* font_path) 
{
    TTF_Font* font = TTF_OpenFont(font_path, 24);
    if (!font)
    {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        return 0;
    }

    if (num_loaded_fonts < MAX_FONTS) 
    {
        loaded_fonts[num_loaded_fonts++] = font;
        return 1;
    } 
    else 
    {
        fprintf(stderr, "Maximum number of fonts loaded.\n");
        TTF_CloseFont(font);
        return 0;
    }
}

void render_text(SDL_Renderer* renderer, const char* text, int x, int y, int font_index, SDL_Color color) 
{
    if (font_index < 0 || font_index >= num_loaded_fonts) 
    {
        fprintf(stderr, "Invalid font index: %d\n", font_index);
        return;
    }

    TTF_Font* font = loaded_fonts[font_index];
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) 
    {
        fprintf(stderr, "Failed to create text surface: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) 
    {
        fprintf(stderr, "Failed to create texture from surface: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dst_rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dst_rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void free_fonts() 
{
    for (int i = 0; i < num_loaded_fonts; i++) 
    {
        if (loaded_fonts[i]) 
        {
            TTF_CloseFont(loaded_fonts[i]);
            loaded_fonts[i] = NULL;
        }
    }
    num_loaded_fonts = 0;
}

#endif