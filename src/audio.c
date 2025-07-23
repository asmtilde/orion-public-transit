#include "audio.h"
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

static Mix_Music* current_music = NULL;

bool audio_init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    return true;
}

void audio_quit() {
    if (current_music) {
        Mix_FreeMusic(current_music);
        current_music = NULL;
    }
    Mix_CloseAudio();
}

bool audio_play_music(const char* filename, int loops) {
    if (current_music) {
        Mix_FreeMusic(current_music);
        current_music = NULL;
    }
    current_music = Mix_LoadMUS(filename);
    if (!current_music) {
        printf("Failed to load music: %s\n", Mix_GetError());
        return false;
    }
    if (Mix_PlayMusic(current_music, loops) == -1) {
        printf("Failed to play music: %s\n", Mix_GetError());
        return false;
    }
    return true;
}

void audio_stop_music() {
    Mix_HaltMusic();
}