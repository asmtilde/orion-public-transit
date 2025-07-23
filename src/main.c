#include "game.h"
#include "graphics.h"
#include "audio.h"
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    if (!game_init("Orion Public Transit", 1280, 960)) {
        return 1;
    }

    // TEMP: Load a test sprite (32x32 PNG in assets/sprites/player.png)
    SDL_Texture* test_sprite = graphics_load_texture("assets/img/bob.png");
    audio_play_music("assets/ost/pheonix.mp3", -1);

    SDL_Event e;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }
        graphics_clear();
        if (test_sprite) {
            graphics_draw_texture(test_sprite, 100, 100, 32, 64);
        }
        graphics_present();
        SDL_Delay(16);
    }

    if (test_sprite) SDL_DestroyTexture(test_sprite);
    game_quit();
    return 0;
}