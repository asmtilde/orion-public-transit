#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Drawing functions
static int l_draw_rectangle(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    int r = luaL_optinteger(L, 5, 255);
    int g = luaL_optinteger(L, 6, 255);
    int b = luaL_optinteger(L, 7, 255);
    int a = luaL_optinteger(L, 8, 255);
    
    DrawRectangle(x, y, width, height, (Color){r, g, b, a});
    return 0;
}

static int l_draw_circle(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    int r = luaL_optinteger(L, 4, 255);
    int g = luaL_optinteger(L, 5, 255);
    int b = luaL_optinteger(L, 6, 255);
    int a = luaL_optinteger(L, 7, 255);
    
    DrawCircle(x, y, radius, (Color){r, g, b, a});
    return 0;
}

static int l_draw_text(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int size = luaL_optinteger(L, 4, 20);
    int r = luaL_optinteger(L, 5, 0);
    int g = luaL_optinteger(L, 6, 0);
    int b = luaL_optinteger(L, 7, 0);
    int a = luaL_optinteger(L, 8, 255);
    
    DrawText(text, x, y, size, (Color){r, g, b, a});
    return 0;
}

static int l_draw_line(lua_State *L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    int r = luaL_optinteger(L, 5, 0);
    int g = luaL_optinteger(L, 6, 0);
    int b = luaL_optinteger(L, 7, 0);
    int a = luaL_optinteger(L, 8, 255);
    
    DrawLine(x1, y1, x2, y2, (Color){r, g, b, a});
    return 0;
}

// Input functions
static int l_is_key_pressed(lua_State *L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsKeyPressed(key));
    return 1;
}

static int l_is_key_down(lua_State *L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsKeyDown(key));
    return 1;
}

static int l_get_mouse_position(lua_State *L) {
    Vector2 pos = GetMousePosition();
    lua_pushinteger(L, (int)pos.x);
    lua_pushinteger(L, (int)pos.y);
    return 2;
}

static int l_is_mouse_button_pressed(lua_State *L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsMouseButtonPressed(button));
    return 1;
}

// Texture functions
static int l_load_texture(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    Texture2D *texture = malloc(sizeof(Texture2D));
    *texture = LoadTexture(filename);
    
    if (texture->id == 0) {
        free(texture);
        lua_pushnil(L);
    } else {
        lua_pushlightuserdata(L, texture);
    }
    return 1;
}

static int l_draw_texture(lua_State *L) {
    Texture2D *texture = (Texture2D*)lua_touserdata(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int r = luaL_optinteger(L, 4, 255);
    int g = luaL_optinteger(L, 5, 255);
    int b = luaL_optinteger(L, 6, 255);
    int a = luaL_optinteger(L, 7, 255);
    
    if (texture && texture->id > 0) {
        DrawTexture(*texture, x, y, (Color){r, g, b, a});
    }
    return 0;
}

static int l_unload_texture(lua_State *L) {
    Texture2D *texture = (Texture2D*)lua_touserdata(L, 1);
    if (texture) {
        UnloadTexture(*texture);
        free(texture);
    }
    return 0;
}

// Utility functions
static int l_get_time(lua_State *L) {
    lua_pushnumber(L, GetTime());
    return 1;
}

static int l_get_fps(lua_State *L) {
    lua_pushinteger(L, GetFPS());
    return 1;
}

static int l_get_screen_width(lua_State *L) {
    lua_pushinteger(L, GetScreenWidth());
    return 1;
}

static int l_get_screen_height(lua_State *L) {
    lua_pushinteger(L, GetScreenHeight());
    return 1;
}

// Sound functions (basic)
static int l_load_sound(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    Sound *sound = malloc(sizeof(Sound));
    *sound = LoadSound(filename);
    
    if (sound->stream.buffer == NULL) {
        free(sound);
        lua_pushnil(L);
    } else {
        lua_pushlightuserdata(L, sound);
    }
    return 1;
}

static int l_play_sound(lua_State *L) {
    Sound *sound = (Sound*)lua_touserdata(L, 1);
    if (sound) {
        PlaySound(*sound);
    }
    return 0;
}

static int l_unload_sound(lua_State *L) {
    Sound *sound = (Sound*)lua_touserdata(L, 1);
    if (sound) {
        UnloadSound(*sound);
        free(sound);
    }
    return 0;
}

Engine *engine_init(void) {
    Engine *engine = malloc(sizeof(Engine));
    engine->L = luaL_newstate();
    luaL_openlibs(engine->L);
    
    InitAudioDevice();
    
    engine_register_functions(engine);
    
    return engine;
}

void engine_shutdown(Engine *engine) {
    if (engine) {
        CloseAudioDevice();
        lua_close(engine->L);
        free(engine);
    }
}

int engine_run_script(Engine *engine, const char *filename) {
    int result = luaL_dofile(engine->L, filename);
    if (result != LUA_OK) {
        printf("Lua Error: %s\n", lua_tostring(engine->L, -1));
        lua_pop(engine->L, 1);
        return 0;
    }
    return 1;
}

void engine_register_functions(Engine *engine) {
    lua_State *L = engine->L;
    
    // Drawing functions
    lua_pushcfunction(L, l_draw_rectangle);
    lua_setglobal(L, "draw_rectangle");
    
    lua_pushcfunction(L, l_draw_circle);
    lua_setglobal(L, "draw_circle");
    
    lua_pushcfunction(L, l_draw_text);
    lua_setglobal(L, "draw_text");
    
    lua_pushcfunction(L, l_draw_line);
    lua_setglobal(L, "draw_line");
    
    // Input functions
    lua_pushcfunction(L, l_is_key_pressed);
    lua_setglobal(L, "is_key_pressed");
    
    lua_pushcfunction(L, l_is_key_down);
    lua_setglobal(L, "is_key_down");
    
    lua_pushcfunction(L, l_get_mouse_position);
    lua_setglobal(L, "get_mouse_position");
    
    lua_pushcfunction(L, l_is_mouse_button_pressed);
    lua_setglobal(L, "is_mouse_button_pressed");
    
    // Texture functions
    lua_pushcfunction(L, l_load_texture);
    lua_setglobal(L, "load_texture");
    
    lua_pushcfunction(L, l_draw_texture);
    lua_setglobal(L, "draw_texture");
    
    lua_pushcfunction(L, l_unload_texture);
    lua_setglobal(L, "unload_texture");
    
    // Sound functions
    lua_pushcfunction(L, l_load_sound);
    lua_setglobal(L, "load_sound");
    
    lua_pushcfunction(L, l_play_sound);
    lua_setglobal(L, "play_sound");
    
    lua_pushcfunction(L, l_unload_sound);
    lua_setglobal(L, "unload_sound");
    
    // Utility functions
    lua_pushcfunction(L, l_get_time);
    lua_setglobal(L, "get_time");
    
    lua_pushcfunction(L, l_get_fps);
    lua_setglobal(L, "get_fps");
    
    lua_pushcfunction(L, l_get_screen_width);
    lua_setglobal(L, "get_screen_width");
    
    lua_pushcfunction(L, l_get_screen_height);
    lua_setglobal(L, "get_screen_height");
    
    // Key constants
    lua_pushinteger(L, KEY_W); lua_setglobal(L, "KEY_W");
    lua_pushinteger(L, KEY_A); lua_setglobal(L, "KEY_A");
    lua_pushinteger(L, KEY_S); lua_setglobal(L, "KEY_S");
    lua_pushinteger(L, KEY_D); lua_setglobal(L, "KEY_D");
    lua_pushinteger(L, KEY_SPACE); lua_setglobal(L, "KEY_SPACE");
    lua_pushinteger(L, KEY_ENTER); lua_setglobal(L, "KEY_ENTER");
    lua_pushinteger(L, KEY_UP); lua_setglobal(L, "KEY_UP");
    lua_pushinteger(L, KEY_DOWN); lua_setglobal(L, "KEY_DOWN");
    lua_pushinteger(L, KEY_LEFT); lua_setglobal(L, "KEY_LEFT");
    lua_pushinteger(L, KEY_RIGHT); lua_setglobal(L, "KEY_RIGHT");
    
    // Mouse button constants
    lua_pushinteger(L, MOUSE_BUTTON_LEFT); lua_setglobal(L, "MOUSE_LEFT");
    lua_pushinteger(L, MOUSE_BUTTON_RIGHT); lua_setglobal(L, "MOUSE_RIGHT");
    lua_pushinteger(L, MOUSE_BUTTON_MIDDLE); lua_setglobal(L, "MOUSE_MIDDLE");
}