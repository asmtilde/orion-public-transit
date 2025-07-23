#pragma once
#include <stdbool.h>

bool audio_init();
void audio_quit();
bool audio_play_music(const char* filepath, int loops);
void audio_stop_music();