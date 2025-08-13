#include "game.h"
#include "graphics.h"
#include "audio.h"
#include "yaml.h"
#include <SDL2/SDL.h>
#include "script.h"

int main(int argc, char* argv[]) {
    if (!game_init("Orion Public Transit", 1280, 960)) {
        return 1;
    }

    load_assets_yaml("assets/yml/assets.yaml");

    read_script("assets/dat/init.script");

    int current_scene = 0;


    int playerx = 100;
    int playery = 100;

    SDL_Texture* test_sprite = graphics_load_texture(search_assets(sprite_assets, "bob"));
    audio_play_music(search_assets(music_assets, "pheonix"), -1);

    SDL_Event e;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = 0;
                        break;
                    case SDLK_LEFT:
                        playerx -= 5;
                        break;
                    case SDLK_RIGHT:
                        playerx += 5;
                        break;
                    case SDLK_UP:
                        playery -= 5;
                        break;
                    case SDLK_DOWN:
                        playery += 5;
                        break;
                }
            } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                running = 0;
            }
        }
        graphics_clear();
        if (test_sprite) {
            graphics_draw_texture(test_sprite, playerx, playery, 32, 64);
        }
        graphics_present();
        SDL_Delay(16);
    }

    if (test_sprite) SDL_DestroyTexture(test_sprite);
    game_quit();
    return 0;
}