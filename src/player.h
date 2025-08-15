#pragma once
#include <math.h>

typedef struct 
{
    int x;
    int y;
    int w;
    int h;
    int speed;
    int maxSpeed;
    int vel_x;
    int vel_y;
} OverworldPlayer;

void overworld_player_controller(OverworldPlayer* player, char input);