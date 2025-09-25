/**
 * Aphelion Game Engine
 * 
 * A single-file game engine with Lua scripting support and comprehensive 2D graphics,
 * audio, input handling, and resource management capabilities.
 * 
 * Copyright (c) 2025 Jack Spencer
 * 
 * Features:
 * - Lua scripting integration with comprehensive API
 * - 2D graphics primitives and texture rendering
 * - Audio playback and management
 * - Input handling (keyboard, mouse, gamepad)
 * - Configuration file parsing (.aphdat format)
 * - Resource management with automatic cleanup
 * - Built-in math utilities and timing functions
 * - Collision detection helpers
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <math.h>

#include <raylib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define APHELION_VERSION "alpha6"
#define APHELION_VERSION_MAJOR 0
#define APHELION_VERSION_MINOR 6

typedef struct {
    char title[256];
    char author[256];
    int width;
    int height;
    int fps;
    char api_version[32];
    char script[512];
    int exit_key;
    bool fullscreen;
    bool resizable;
    bool vsync;
} APHConfig;

typedef struct {
    lua_State *L;
    int should_quit;
    APHConfig config;
    int debug;
    int ignore_not_found;
} Engine;

static void aph_config_defaults(APHConfig *cfg) {
    strcpy(cfg->title, "Aphelion Game");
    strcpy(cfg->author, "Unknown");
    cfg->width = 1280;
    cfg->height = 960;
    cfg->fps = 60;
    strcpy(cfg->api_version, APHELION_VERSION);
    cfg->script[0] = '\0';
    cfg->exit_key = KEY_DELETE;
    cfg->fullscreen = false;
    cfg->resizable = true;
    cfg->vsync = true;
}

static char *trim_inplace(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

static int parse_key_value(const char *line, char *key_out, char *val_out) {
    const char *p = line;
    while (*p && isspace((unsigned char)*p)) p++;
    
    if (*p == '#' || *p == '\0') return 0;
    
    const char *eq = strchr(p, '=');
    if (!eq) return 0;
    
    size_t klen = eq - p;
    while (klen > 0 && isspace((unsigned char)p[klen-1])) klen--;
    strncpy(key_out, p, klen);
    key_out[klen] = '\0';
    
    const char *vstart = eq + 1;
    while (*vstart && isspace((unsigned char)*vstart)) vstart++;
    strcpy(val_out, vstart);
    
    char *vtrim = val_out + strlen(val_out) - 1;
    while (vtrim >= val_out && isspace((unsigned char)*vtrim)) {
        *vtrim = '\0';
        vtrim--;
    }
    
    char *ktrim = trim_inplace(key_out);
    memmove(key_out, ktrim, strlen(ktrim)+1);
    char *vtrim2 = trim_inplace(val_out);
    memmove(val_out, vtrim2, strlen(vtrim2)+1);
    
    return 1;
}

static bool parse_bool(const char *val) {
    return (strncmp(val, "true", 4) == 0 || 
            strncmp(val, "yes", 3) == 0 || 
            strncmp(val, "1", 1) == 0 ||
            strncmp(val, "on", 2) == 0);
}

static void aph_config_load_from_file(APHConfig *cfg, const char *filename) {
    aph_config_defaults(cfg);
    
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Warning: couldn't open config file '%s', using defaults\n", filename);
        return;
    }
    
    char line[1024];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        char key[256] = {0}, val[768] = {0};
        
        if (!parse_key_value(line, key, val)) continue;
        
        for (char *p = key; *p; ++p) *p = tolower((unsigned char)*p);
        
        if (strcmp(key, "title") == 0) {
            strncpy(cfg->title, val, sizeof(cfg->title)-1);
            cfg->title[sizeof(cfg->title)-1] = '\0';
        } else if (strcmp(key, "width") == 0) {
            int w = atoi(val);
            cfg->width = (w > 0) ? w : cfg->width;
        } else if (strcmp(key, "height") == 0) {
            int h = atoi(val);
            cfg->height = (h > 0) ? h : cfg->height;
        } else if (strcmp(key, "fps") == 0) {
            int fps = atoi(val);
            cfg->fps = (fps > 0) ? fps : cfg->fps;
        } else if (strcmp(key, "script") == 0) {
            strncpy(cfg->script, val, sizeof(cfg->script)-1);
            cfg->script[sizeof(cfg->script)-1] = '\0';
        } else if (strcmp(key, "exit_key") == 0) {
            if (strncmp(val, "DELETE", 6) == 0) cfg->exit_key = KEY_DELETE;
            else if (strncmp(val, "ESCAPE", 6) == 0) cfg->exit_key = KEY_ESCAPE;
            else if (strncmp(val, "SPACE", 5) == 0) cfg->exit_key = KEY_SPACE;
            else if (strncmp(val, "ENTER", 5) == 0) cfg->exit_key = KEY_ENTER;
            else if (strncmp(val, "BACKSPACE", 9) == 0) cfg->exit_key = KEY_BACKSPACE;
            else cfg->exit_key = atoi(val);
        } else if (strcmp(key, "fullscreen") == 0) {
            cfg->fullscreen = parse_bool(val);
        } else if (strcmp(key, "author") == 0) {
            strncpy(cfg->author, val, sizeof(cfg->author)-1);
            cfg->author[sizeof(cfg->author)-1] = '\0';
        } else if (strcmp(key, "api_version") == 0) {
            strncpy(cfg->api_version, val, sizeof(cfg->api_version)-1);
            cfg->api_version[sizeof(cfg->api_version)-1] = '\0';
        } else if (strcmp(key, "resizable") == 0) {
            cfg->resizable = parse_bool(val);
        } else if (strcmp(key, "vsync") == 0) {
            cfg->vsync = parse_bool(val);
        } else {
            fprintf(stderr, "Warning: Unknown config key '%s' on line %d\n", key, line_num);
        }
    }
    
    fclose(f);
}

typedef struct { 
    Texture2D tex; 
    bool valid;
} LuaTexture;

typedef struct { 
    Sound snd; 
    bool valid;
} LuaSound;

typedef struct { 
    Music mus; 
    bool valid;
} LuaMusic;

static int l_draw_rectangle(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int w = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);
    int r = luaL_optinteger(L, 5, 255);
    int g = luaL_optinteger(L, 6, 255);
    int b = luaL_optinteger(L, 7, 255);
    int a = luaL_optinteger(L, 8, 255);
    
    DrawRectangle(x, y, w, h, (Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
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
    
    DrawCircle(x, y, (float)radius, (Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
    return 0;
}

static int l_draw_ellipse(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radiusH = luaL_checkinteger(L, 3);
    int radiusV = luaL_checkinteger(L, 4);
    int r = luaL_optinteger(L, 5, 255);
    int g = luaL_optinteger(L, 6, 255);
    int b = luaL_optinteger(L, 7, 255);
    int a = luaL_optinteger(L, 8, 255);
    
    DrawEllipse(x, y, (float)radiusH, (float)radiusV, (Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
    return 0;
}

static int l_draw_triangle(lua_State *L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    int x3 = luaL_checkinteger(L, 5);
    int y3 = luaL_checkinteger(L, 6);
    int r = luaL_optinteger(L, 7, 255);
    int g = luaL_optinteger(L, 8, 255);
    int b = luaL_optinteger(L, 9, 255);
    int a = luaL_optinteger(L, 10, 255);
    
    Vector2 v1 = {(float)x1, (float)y1};
    Vector2 v2 = {(float)x2, (float)y2};
    Vector2 v3 = {(float)x3, (float)y3};
    
    DrawTriangle(v1, v2, v3, (Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
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
    
    DrawText(text, x, y, size, (Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
    return 0;
}

static int l_draw_text_centered(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int size = luaL_optinteger(L, 4, 20);
    int r = luaL_optinteger(L, 5, 0);
    int g = luaL_optinteger(L, 6, 0);
    int b = luaL_optinteger(L, 7, 0);
    int a = luaL_optinteger(L, 8, 255);
    
    int tw = MeasureText(text, size);
    DrawText(text, x - tw/2, y, size, (Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
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
    
    DrawLine(x1, y1, x2, y2, (Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
    return 0;
}

static int l_draw_background(lua_State *L) {
    int r = luaL_optinteger(L, 1, 255);
    int g = luaL_optinteger(L, 2, 255);
    int b = luaL_optinteger(L, 3, 255);
    int a = luaL_optinteger(L, 4, 255);
    
    ClearBackground((Color){
        (unsigned char)CLAMP(r, 0, 255),
        (unsigned char)CLAMP(g, 0, 255),
        (unsigned char)CLAMP(b, 0, 255),
        (unsigned char)CLAMP(a, 0, 255)
    });
    return 0;
}

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

static int l_is_key_released(lua_State *L) {
    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsKeyReleased(key));
    return 1;
}

static int l_get_mouse_position(lua_State *L) {
    Vector2 p = GetMousePosition();
    lua_pushinteger(L, (int)p.x);
    lua_pushinteger(L, (int)p.y);
    return 2;
}

static int l_is_mouse_button_pressed(lua_State *L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsMouseButtonPressed(button));
    return 1;
}

static int l_is_mouse_button_down(lua_State *L) {
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsMouseButtonDown(button));
    return 1;
}

static int l_get_mouse_wheel_move(lua_State *L) {
    lua_pushnumber(L, GetMouseWheelMove());
    return 1;
}

static int l_is_any_key_pressed(lua_State *L) {
    for (int k = 32; k < 349; ++k) {
        if (IsKeyPressed(k)) {
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    lua_pushboolean(L, 0);
    return 1;
}

static int l_is_gamepad_available(lua_State *L) {
    int gamepad = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsGamepadAvailable(gamepad));
    return 1;
}

static int l_is_gamepad_button_pressed(lua_State *L) {
    int gamepad = luaL_checkinteger(L, 1);
    int button = luaL_checkinteger(L, 2);
    lua_pushboolean(L, IsGamepadButtonPressed(gamepad, button));
    return 1;
}

static int l_load_texture(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    LuaTexture *ud = (LuaTexture*)lua_newuserdata(L, sizeof(LuaTexture));
    
    ud->tex = LoadTexture(filename);
    ud->valid = (ud->tex.id != 0);
    
    if (!ud->valid) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return 1;
    }
    
    luaL_getmetatable(L, "engine.texture");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_draw_texture(lua_State *L) {
    LuaTexture *ud = (LuaTexture*)luaL_checkudata(L, 1, "engine.texture");
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int r = luaL_optinteger(L, 4, 255);
    int g = luaL_optinteger(L, 5, 255);
    int b = luaL_optinteger(L, 6, 255);
    int a = luaL_optinteger(L, 7, 255);
    
    if (ud->valid) {
        DrawTexture(ud->tex, x, y, (Color){
            (unsigned char)CLAMP(r, 0, 255),
            (unsigned char)CLAMP(g, 0, 255),
            (unsigned char)CLAMP(b, 0, 255),
            (unsigned char)CLAMP(a, 0, 255)
        });
    }
    return 0;
}

static int l_draw_texture_rec(lua_State *L) {
    LuaTexture *ud = (LuaTexture*)luaL_checkudata(L, 1, "engine.texture");
    int sx = luaL_checkinteger(L, 2);
    int sy = luaL_checkinteger(L, 3);
    int sw = luaL_checkinteger(L, 4);
    int sh = luaL_checkinteger(L, 5);
    int dx = luaL_checkinteger(L, 6);
    int dy = luaL_checkinteger(L, 7);
    int r = luaL_optinteger(L, 8, 255);
    int g = luaL_optinteger(L, 9, 255);
    int b = luaL_optinteger(L, 10, 255);
    int a = luaL_optinteger(L, 11, 255);
    
    if (ud->valid) {
        Rectangle src = {(float)sx, (float)sy, (float)sw, (float)sh};
        Vector2 pos = {(float)dx, (float)dy};
        DrawTextureRec(ud->tex, src, pos, (Color){
            (unsigned char)CLAMP(r, 0, 255),
            (unsigned char)CLAMP(g, 0, 255),
            (unsigned char)CLAMP(b, 0, 255),
            (unsigned char)CLAMP(a, 0, 255)
        });
    }
    return 0;
}

static int l_draw_texture_scaled(lua_State *L) {
    LuaTexture *ud = (LuaTexture*)luaL_checkudata(L, 1, "engine.texture");
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    float scale = (float)luaL_checknumber(L, 4);
    int r = luaL_optinteger(L, 5, 255);
    int g = luaL_optinteger(L, 6, 255);
    int b = luaL_optinteger(L, 7, 255);
    int a = luaL_optinteger(L, 8, 255);
    
    if (ud->valid) {
        DrawTextureEx(ud->tex, (Vector2){(float)x, (float)y}, 0.0f, scale, (Color){
            (unsigned char)CLAMP(r, 0, 255),
            (unsigned char)CLAMP(g, 0, 255),
            (unsigned char)CLAMP(b, 0, 255),
            (unsigned char)CLAMP(a, 0, 255)
        });
    }
    return 0;
}

static int l_draw_texture_rotated(lua_State *L) {
    LuaTexture *ud = (LuaTexture*)luaL_checkudata(L, 1, "engine.texture");
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    float rotation = (float)luaL_checknumber(L, 4);
    int r = luaL_optinteger(L, 5, 255);
    int g = luaL_optinteger(L, 6, 255);
    int b = luaL_optinteger(L, 7, 255);
    int a = luaL_optinteger(L, 8, 255);
    
    if (ud->valid) {
        DrawTextureEx(ud->tex, (Vector2){(float)x, (float)y}, rotation, 1.0f, (Color){
            (unsigned char)CLAMP(r, 0, 255),
            (unsigned char)CLAMP(g, 0, 255),
            (unsigned char)CLAMP(b, 0, 255),
            (unsigned char)CLAMP(a, 0, 255)
        });
    }
    return 0;
}

static int l_get_texture_size(lua_State *L) {
    LuaTexture *ud = (LuaTexture*)luaL_checkudata(L, 1, "engine.texture");
    if (ud->valid) {
        lua_pushinteger(L, ud->tex.width);
        lua_pushinteger(L, ud->tex.height);
        return 2;
    }
    lua_pushinteger(L, 0);
    lua_pushinteger(L, 0);
    return 2;
}

static int l_texture_gc(lua_State *L) {
    LuaTexture *ud = (LuaTexture*)luaL_checkudata(L, 1, "engine.texture");
    if (ud->valid) {
        UnloadTexture(ud->tex);
        ud->valid = false;
    }
    return 0;
}

static int l_load_sound(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    LuaSound *ud = (LuaSound*)lua_newuserdata(L, sizeof(LuaSound));
    
    ud->snd = LoadSound(filename);
    ud->valid = (ud->snd.stream.buffer != NULL);
    
    if (!ud->valid) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return 1;
    }
    
    luaL_getmetatable(L, "engine.sound");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_play_sound(lua_State *L) {
    LuaSound *ud = (LuaSound*)luaL_checkudata(L, 1, "engine.sound");
    if (ud->valid) {
        PlaySound(ud->snd);
    }
    return 0;
}

static int l_stop_sound(lua_State *L) {
    LuaSound *ud = (LuaSound*)luaL_checkudata(L, 1, "engine.sound");
    if (ud->valid) {
        StopSound(ud->snd);
    }
    return 0;
}

static int l_set_sound_volume(lua_State *L) {
    LuaSound *ud = (LuaSound*)luaL_checkudata(L, 1, "engine.sound");
    float volume = (float)luaL_checknumber(L, 2);
    if (ud->valid) {
        SetSoundVolume(ud->snd, CLAMP(volume, 0.0f, 1.0f));
    }
    return 0;
}

static int l_sound_gc(lua_State *L) {
    LuaSound *ud = (LuaSound*)luaL_checkudata(L, 1, "engine.sound");
    if (ud->valid) {
        UnloadSound(ud->snd);
        ud->valid = false;
    }
    return 0;
}

static int l_load_music(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    LuaMusic *ud = (LuaMusic*)lua_newuserdata(L, sizeof(LuaMusic));
    
    ud->mus = LoadMusicStream(filename);
    ud->valid = (ud->mus.stream.buffer != NULL);
    
    if (!ud->valid) {
        lua_pop(L, 1);
        lua_pushnil(L);
        return 1;
    }
    
    luaL_getmetatable(L, "engine.music");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_play_music(lua_State *L) {
    LuaMusic *ud = (LuaMusic*)luaL_checkudata(L, 1, "engine.music");
    if (ud->valid) {
        PlayMusicStream(ud->mus);
    }
    return 0;
}

static int l_stop_music(lua_State *L) {
    LuaMusic *ud = (LuaMusic*)luaL_checkudata(L, 1, "engine.music");
    if (ud->valid) {
        StopMusicStream(ud->mus);
    }
    return 0;
}

static int l_update_music(lua_State *L) {
    LuaMusic *ud = (LuaMusic*)luaL_checkudata(L, 1, "engine.music");
    if (ud->valid) {
        UpdateMusicStream(ud->mus);
    }
    return 0;
}

static int l_set_music_volume(lua_State *L) {
    float volume = (float)luaL_checknumber(L, 1);
    SetMasterVolume(CLAMP(volume, 0.0f, 1.0f));
    return 0;
}

static int l_music_gc(lua_State *L) {
    LuaMusic *ud = (LuaMusic*)luaL_checkudata(L, 1, "engine.music");
    if (ud->valid) {
        UnloadMusicStream(ud->mus);
        ud->valid = false;
    }
    return 0;
}

static int l_get_time(lua_State *L) { 
    lua_pushnumber(L, GetTime()); 
    return 1; 
}

static int l_get_fps(lua_State *L) { 
    lua_pushinteger(L, GetFPS()); 
    return 1; 
}

static int l_get_frame_time(lua_State *L) { 
    lua_pushnumber(L, GetFrameTime()); 
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

static int l_change_screen_title(lua_State *L) { 
    const char *title = luaL_checkstring(L, 1); 
    SetWindowTitle(title); 
    return 0; 
}

static int l_request_quit(lua_State *L) {
    Engine *engine = (Engine*)lua_touserdata(L, lua_upvalueindex(1));
    if (engine) engine->should_quit = 1;
    return 0;
}

static int l_math_lerp(lua_State *L) {
    float a = (float)luaL_checknumber(L, 1);
    float b = (float)luaL_checknumber(L, 2);
    float t = (float)luaL_checknumber(L, 3);
    lua_pushnumber(L, a + t * (b - a));
    return 1;
}

static int l_math_clamp(lua_State *L) {
    float value = (float)luaL_checknumber(L, 1);
    float min = (float)luaL_checknumber(L, 2);
    float max = (float)luaL_checknumber(L, 3);
    lua_pushnumber(L, CLAMP(value, min, max));
    return 1;
}

static int l_math_distance(lua_State *L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float x2 = (float)luaL_checknumber(L, 3);
    float y2 = (float)luaL_checknumber(L, 4);
    
    float dx = x2 - x1;
    float dy = y2 - y1;
    lua_pushnumber(L, sqrtf(dx * dx + dy * dy));
    return 1;
}

static int l_math_angle(lua_State *L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float x2 = (float)luaL_checknumber(L, 3);
    float y2 = (float)luaL_checknumber(L, 4);
    
    float dx = x2 - x1;
    float dy = y2 - y1;
    lua_pushnumber(L, atan2f(dy, dx));
    return 1;
}

static int l_check_collision_recs(lua_State *L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float w1 = (float)luaL_checknumber(L, 3);
    float h1 = (float)luaL_checknumber(L, 4);
    float x2 = (float)luaL_checknumber(L, 5);
    float y2 = (float)luaL_checknumber(L, 6);
    float w2 = (float)luaL_checknumber(L, 7);
    float h2 = (float)luaL_checknumber(L, 8);
    
    Rectangle rec1 = {x1, y1, w1, h1};
    Rectangle rec2 = {x2, y2, w2, h2};
    
    lua_pushboolean(L, CheckCollisionRecs(rec1, rec2));
    return 1;
}

static int l_check_collision_circles(lua_State *L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float r1 = (float)luaL_checknumber(L, 3);
    float x2 = (float)luaL_checknumber(L, 4);
    float y2 = (float)luaL_checknumber(L, 5);
    float r2 = (float)luaL_checknumber(L, 6);
    
    Vector2 center1 = {x1, y1};
    Vector2 center2 = {x2, y2};
    
    lua_pushboolean(L, CheckCollisionCircles(center1, r1, center2, r2));
    return 1;
}

static int l_check_collision_point_rec(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float rx = (float)luaL_checknumber(L, 3);
    float ry = (float)luaL_checknumber(L, 4);
    float rw = (float)luaL_checknumber(L, 5);
    float rh = (float)luaL_checknumber(L, 6);
    
    Vector2 point = {x, y};
    Rectangle rec = {rx, ry, rw, rh};
    
    lua_pushboolean(L, CheckCollisionPointRec(point, rec));
    return 1;
}

static int l_does_file_exist(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    lua_pushboolean(L, FileExists(filename));
    return 1;
}

static void engine_register_functions(Engine *engine) {
    lua_State *L = engine->L;
    engine->should_quit = 0;

    luaL_newmetatable(L, "engine.texture");
    lua_pushcfunction(L, l_texture_gc); 
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "engine.sound");
    lua_pushcfunction(L, l_sound_gc); 
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "engine.music");
    lua_pushcfunction(L, l_music_gc); 
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    lua_newtable(L);

    lua_pushcfunction(L, l_draw_rectangle); lua_setfield(L, -2, "draw_rectangle");
    lua_pushcfunction(L, l_draw_circle); lua_setfield(L, -2, "draw_circle");
    lua_pushcfunction(L, l_draw_ellipse); lua_setfield(L, -2, "draw_ellipse");
    lua_pushcfunction(L, l_draw_triangle); lua_setfield(L, -2, "draw_triangle");
    lua_pushcfunction(L, l_draw_text); lua_setfield(L, -2, "draw_text");
    lua_pushcfunction(L, l_draw_text_centered); lua_setfield(L, -2, "draw_text_centered");
    lua_pushcfunction(L, l_draw_line); lua_setfield(L, -2, "draw_line");
    lua_pushcfunction(L, l_draw_background); lua_setfield(L, -2, "draw_background");

    lua_pushcfunction(L, l_is_key_pressed); lua_setfield(L, -2, "is_key_pressed");
    lua_pushcfunction(L, l_is_key_down); lua_setfield(L, -2, "is_key_down");
    lua_pushcfunction(L, l_is_key_released); lua_setfield(L, -2, "is_key_released");
    lua_pushcfunction(L, l_is_any_key_pressed); lua_setfield(L, -2, "is_any_key_pressed");
    lua_pushcfunction(L, l_get_mouse_position); lua_setfield(L, -2, "get_mouse_position");
    lua_pushcfunction(L, l_is_mouse_button_pressed); lua_setfield(L, -2, "is_mouse_button_pressed");
    lua_pushcfunction(L, l_is_mouse_button_down); lua_setfield(L, -2, "is_mouse_button_down");
    lua_pushcfunction(L, l_get_mouse_wheel_move); lua_setfield(L, -2, "get_mouse_wheel_move");
    lua_pushcfunction(L, l_is_gamepad_available); lua_setfield(L, -2, "is_gamepad_available");
    lua_pushcfunction(L, l_is_gamepad_button_pressed); lua_setfield(L, -2, "is_gamepad_button_pressed");

    lua_pushcfunction(L, l_load_texture); lua_setfield(L, -2, "load_texture");
    lua_pushcfunction(L, l_draw_texture); lua_setfield(L, -2, "draw_texture");
    lua_pushcfunction(L, l_draw_texture_rec); lua_setfield(L, -2, "draw_texture_rec");
    lua_pushcfunction(L, l_draw_texture_scaled); lua_setfield(L, -2, "draw_texture_scaled");
    lua_pushcfunction(L, l_draw_texture_rotated); lua_setfield(L, -2, "draw_texture_rotated");
    lua_pushcfunction(L, l_get_texture_size); lua_setfield(L, -2, "get_texture_size");

    lua_pushcfunction(L, l_load_sound); lua_setfield(L, -2, "load_sound");
    lua_pushcfunction(L, l_play_sound); lua_setfield(L, -2, "play_sound");
    lua_pushcfunction(L, l_stop_sound); lua_setfield(L, -2, "stop_sound");
    lua_pushcfunction(L, l_set_sound_volume); lua_setfield(L, -2, "set_sound_volume");
    lua_pushcfunction(L, l_load_music); lua_setfield(L, -2, "load_music");
    lua_pushcfunction(L, l_play_music); lua_setfield(L, -2, "play_music");
    lua_pushcfunction(L, l_stop_music); lua_setfield(L, -2, "stop_music");
    lua_pushcfunction(L, l_update_music); lua_setfield(L, -2, "update_music");
    lua_pushcfunction(L, l_set_music_volume); lua_setfield(L, -2, "set_music_volume");

    lua_pushcfunction(L, l_get_time); lua_setfield(L, -2, "get_time");
    lua_pushcfunction(L, l_get_fps); lua_setfield(L, -2, "get_fps");
    lua_pushcfunction(L, l_get_frame_time); lua_setfield(L, -2, "get_frame_time");
    lua_pushcfunction(L, l_get_screen_width); lua_setfield(L, -2, "get_screen_width");
    lua_pushcfunction(L, l_get_screen_height); lua_setfield(L, -2, "get_screen_height");
    lua_pushcfunction(L, l_change_screen_title); lua_setfield(L, -2, "change_screen_title");
    lua_pushcfunction(L, l_does_file_exist); lua_setfield(L, -2, "does_file_exist");

    lua_pushcfunction(L, l_math_lerp); lua_setfield(L, -2, "math_lerp");
    lua_pushcfunction(L, l_math_clamp); lua_setfield(L, -2, "math_clamp");
    lua_pushcfunction(L, l_math_distance); lua_setfield(L, -2, "math_distance");
    lua_pushcfunction(L, l_math_angle); lua_setfield(L, -2, "math_angle");

    lua_pushcfunction(L, l_check_collision_recs); lua_setfield(L, -2, "check_collision_recs");
    lua_pushcfunction(L, l_check_collision_circles); lua_setfield(L, -2, "check_collision_circles");
    lua_pushcfunction(L, l_check_collision_point_rec); lua_setfield(L, -2, "check_collision_point_rec");

    lua_pushlightuserdata(L, engine);
    lua_pushcclosure(L, l_request_quit, 1);
    lua_setfield(L, -2, "quit");

    lua_pushinteger(L, KEY_SPACE); lua_setfield(L, -2, "KEY_SPACE");
    lua_pushinteger(L, KEY_ESCAPE); lua_setfield(L, -2, "KEY_ESCAPE");
    lua_pushinteger(L, KEY_ENTER); lua_setfield(L, -2, "KEY_ENTER");
    lua_pushinteger(L, KEY_TAB); lua_setfield(L, -2, "KEY_TAB");
    lua_pushinteger(L, KEY_BACKSPACE); lua_setfield(L, -2, "KEY_BACKSPACE");
    lua_pushinteger(L, KEY_INSERT); lua_setfield(L, -2, "KEY_INSERT");
    lua_pushinteger(L, KEY_DELETE); lua_setfield(L, -2, "KEY_DELETE");
    lua_pushinteger(L, KEY_RIGHT); lua_setfield(L, -2, "KEY_RIGHT");
    lua_pushinteger(L, KEY_LEFT); lua_setfield(L, -2, "KEY_LEFT");
    lua_pushinteger(L, KEY_DOWN); lua_setfield(L, -2, "KEY_DOWN");
    lua_pushinteger(L, KEY_UP); lua_setfield(L, -2, "KEY_UP");
    lua_pushinteger(L, KEY_PAGE_UP); lua_setfield(L, -2, "KEY_PAGE_UP");
    lua_pushinteger(L, KEY_PAGE_DOWN); lua_setfield(L, -2, "KEY_PAGE_DOWN");
    lua_pushinteger(L, KEY_HOME); lua_setfield(L, -2, "KEY_HOME");
    lua_pushinteger(L, KEY_END); lua_setfield(L, -2, "KEY_END");
    lua_pushinteger(L, KEY_CAPS_LOCK); lua_setfield(L, -2, "KEY_CAPS_LOCK");
    lua_pushinteger(L, KEY_SCROLL_LOCK); lua_setfield(L, -2, "KEY_SCROLL_LOCK");
    lua_pushinteger(L, KEY_NUM_LOCK); lua_setfield(L, -2, "KEY_NUM_LOCK");
    lua_pushinteger(L, KEY_PRINT_SCREEN); lua_setfield(L, -2, "KEY_PRINT_SCREEN");
    lua_pushinteger(L, KEY_PAUSE); lua_setfield(L, -2, "KEY_PAUSE");
    lua_pushinteger(L, KEY_F1); lua_setfield(L, -2, "KEY_F1");
    lua_pushinteger(L, KEY_F2); lua_setfield(L, -2, "KEY_F2");
    lua_pushinteger(L, KEY_F3); lua_setfield(L, -2, "KEY_F3");
    lua_pushinteger(L, KEY_F4); lua_setfield(L, -2, "KEY_F4");
    lua_pushinteger(L, KEY_F5); lua_setfield(L, -2, "KEY_F5");
    lua_pushinteger(L, KEY_F6); lua_setfield(L, -2, "KEY_F6");
    lua_pushinteger(L, KEY_F7); lua_setfield(L, -2, "KEY_F7");
    lua_pushinteger(L, KEY_F8); lua_setfield(L, -2, "KEY_F8");
    lua_pushinteger(L, KEY_F9); lua_setfield(L, -2, "KEY_F9");
    lua_pushinteger(L, KEY_F10); lua_setfield(L, -2, "KEY_F10");
    lua_pushinteger(L, KEY_F11); lua_setfield(L, -2, "KEY_F11");
    lua_pushinteger(L, KEY_F12); lua_setfield(L, -2, "KEY_F12");
    lua_pushinteger(L, KEY_LEFT_SHIFT); lua_setfield(L, -2, "KEY_LEFT_SHIFT");
    lua_pushinteger(L, KEY_LEFT_CONTROL); lua_setfield(L, -2, "KEY_LEFT_CONTROL");
    lua_pushinteger(L, KEY_LEFT_ALT); lua_setfield(L, -2, "KEY_LEFT_ALT");
    lua_pushinteger(L, KEY_LEFT_SUPER); lua_setfield(L, -2, "KEY_LEFT_SUPER");
    lua_pushinteger(L, KEY_RIGHT_SHIFT); lua_setfield(L, -2, "KEY_RIGHT_SHIFT");
    lua_pushinteger(L, KEY_RIGHT_CONTROL); lua_setfield(L, -2, "KEY_RIGHT_CONTROL");
    lua_pushinteger(L, KEY_RIGHT_ALT); lua_setfield(L, -2, "KEY_RIGHT_ALT");
    lua_pushinteger(L, KEY_RIGHT_SUPER); lua_setfield(L, -2, "KEY_RIGHT_SUPER");

    lua_pushinteger(L, KEY_A); lua_setfield(L, -2, "KEY_A");
    lua_pushinteger(L, KEY_B); lua_setfield(L, -2, "KEY_B");
    lua_pushinteger(L, KEY_C); lua_setfield(L, -2, "KEY_C");
    lua_pushinteger(L, KEY_D); lua_setfield(L, -2, "KEY_D");
    lua_pushinteger(L, KEY_E); lua_setfield(L, -2, "KEY_E");
    lua_pushinteger(L, KEY_F); lua_setfield(L, -2, "KEY_F");
    lua_pushinteger(L, KEY_G); lua_setfield(L, -2, "KEY_G");
    lua_pushinteger(L, KEY_H); lua_setfield(L, -2, "KEY_H");
    lua_pushinteger(L, KEY_I); lua_setfield(L, -2, "KEY_I");
    lua_pushinteger(L, KEY_J); lua_setfield(L, -2, "KEY_J");
    lua_pushinteger(L, KEY_K); lua_setfield(L, -2, "KEY_K");
    lua_pushinteger(L, KEY_L); lua_setfield(L, -2, "KEY_L");
    lua_pushinteger(L, KEY_M); lua_setfield(L, -2, "KEY_M");
    lua_pushinteger(L, KEY_N); lua_setfield(L, -2, "KEY_N");
    lua_pushinteger(L, KEY_O); lua_setfield(L, -2, "KEY_O");
    lua_pushinteger(L, KEY_P); lua_setfield(L, -2, "KEY_P");
    lua_pushinteger(L, KEY_Q); lua_setfield(L, -2, "KEY_Q");
    lua_pushinteger(L, KEY_R); lua_setfield(L, -2, "KEY_R");
    lua_pushinteger(L, KEY_S); lua_setfield(L, -2, "KEY_S");
    lua_pushinteger(L, KEY_T); lua_setfield(L, -2, "KEY_T");
    lua_pushinteger(L, KEY_U); lua_setfield(L, -2, "KEY_U");
    lua_pushinteger(L, KEY_V); lua_setfield(L, -2, "KEY_V");
    lua_pushinteger(L, KEY_W); lua_setfield(L, -2, "KEY_W");
    lua_pushinteger(L, KEY_X); lua_setfield(L, -2, "KEY_X");
    lua_pushinteger(L, KEY_Y); lua_setfield(L, -2, "KEY_Y");
    lua_pushinteger(L, KEY_Z); lua_setfield(L, -2, "KEY_Z");

    lua_pushinteger(L, KEY_ZERO); lua_setfield(L, -2, "KEY_0");
    lua_pushinteger(L, KEY_ONE); lua_setfield(L, -2, "KEY_1");
    lua_pushinteger(L, KEY_TWO); lua_setfield(L, -2, "KEY_2");
    lua_pushinteger(L, KEY_THREE); lua_setfield(L, -2, "KEY_3");
    lua_pushinteger(L, KEY_FOUR); lua_setfield(L, -2, "KEY_4");
    lua_pushinteger(L, KEY_FIVE); lua_setfield(L, -2, "KEY_5");
    lua_pushinteger(L, KEY_SIX); lua_setfield(L, -2, "KEY_6");
    lua_pushinteger(L, KEY_SEVEN); lua_setfield(L, -2, "KEY_7");
    lua_pushinteger(L, KEY_EIGHT); lua_setfield(L, -2, "KEY_8");
    lua_pushinteger(L, KEY_NINE); lua_setfield(L, -2, "KEY_9");

    lua_pushinteger(L, MOUSE_BUTTON_LEFT); lua_setfield(L, -2, "MOUSE_LEFT");
    lua_pushinteger(L, MOUSE_BUTTON_RIGHT); lua_setfield(L, -2, "MOUSE_RIGHT");
    lua_pushinteger(L, MOUSE_BUTTON_MIDDLE); lua_setfield(L, -2, "MOUSE_MIDDLE");

    lua_pushinteger(L, GAMEPAD_BUTTON_UNKNOWN); lua_setfield(L, -2, "GAMEPAD_UNKNOWN");
    lua_pushinteger(L, GAMEPAD_BUTTON_LEFT_FACE_UP); lua_setfield(L, -2, "GAMEPAD_DPAD_UP");
    lua_pushinteger(L, GAMEPAD_BUTTON_LEFT_FACE_RIGHT); lua_setfield(L, -2, "GAMEPAD_DPAD_RIGHT");
    lua_pushinteger(L, GAMEPAD_BUTTON_LEFT_FACE_DOWN); lua_setfield(L, -2, "GAMEPAD_DPAD_DOWN");
    lua_pushinteger(L, GAMEPAD_BUTTON_LEFT_FACE_LEFT); lua_setfield(L, -2, "GAMEPAD_DPAD_LEFT");
    lua_pushinteger(L, GAMEPAD_BUTTON_RIGHT_FACE_UP); lua_setfield(L, -2, "GAMEPAD_Y");
    lua_pushinteger(L, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT); lua_setfield(L, -2, "GAMEPAD_B");
    lua_pushinteger(L, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); lua_setfield(L, -2, "GAMEPAD_A");
    lua_pushinteger(L, GAMEPAD_BUTTON_RIGHT_FACE_LEFT); lua_setfield(L, -2, "GAMEPAD_X");

    lua_pushstring(L, APHELION_VERSION); lua_setfield(L, -2, "VERSION");
    lua_pushinteger(L, APHELION_VERSION_MAJOR); lua_setfield(L, -2, "VERSION_MAJOR");
    lua_pushinteger(L, APHELION_VERSION_MINOR); lua_setfield(L, -2, "VERSION_MINOR");

    lua_setglobal(L, "engine");
}

static Engine *engine_init(const APHConfig *config) {
    Engine *engine = (Engine*)malloc(sizeof(Engine));
    if (!engine) return NULL;
    
    engine->L = luaL_newstate();
    if (!engine->L) { 
        free(engine); 
        return NULL; 
    }
    
    engine->config = *config;
    engine->debug = 0;
    
    luaL_openlibs(engine->L);
    
    InitAudioDevice();
    
    engine_register_functions(engine);
    
    engine->should_quit = 0;
    return engine;
}

static void engine_shutdown(Engine *engine) {
    if (!engine) return;
    
    CloseAudioDevice();
    if (engine->L) lua_close(engine->L);
    free(engine);
}

static int engine_run_script(Engine *engine, const char *filename) {
    if (!engine || !engine->L) return 0;
    
    int result = luaL_dofile(engine->L, filename);
    if (result != LUA_OK) {
        const char *error = lua_tostring(engine->L, -1);
        fprintf(stderr, "Lua Error while loading '%s': %s\n", 
                filename, error ? error : "(no message)");
        lua_pop(engine->L, 1);
        return 0;
    }
    return 1;
}

static void engine_call_lua_function(Engine *engine, const char *func_name) {
    lua_getglobal(engine->L, func_name);
    if (lua_isfunction(engine->L, -1)) {
        if (lua_pcall(engine->L, 0, 0, 0) != LUA_OK) {
            const char *error = lua_tostring(engine->L, -1);
            fprintf(stderr, "Lua Error in %s: %s\n", func_name, 
                    error ? error : "(no message)");
            lua_pop(engine->L, 1);
        }
    } else {
        lua_pop(engine->L, 1);
    }
}

static void engine_call_lua_function_arg(Engine *engine, const char *func_name, double arg) {
    lua_getglobal(engine->L, func_name);
    if (lua_isfunction(engine->L, -1)) {
        lua_pushnumber(engine->L, arg);
        if (lua_pcall(engine->L, 1, 0, 0) != LUA_OK) {
            const char *error = lua_tostring(engine->L, -1);
            fprintf(stderr, "Lua Error in %s: %s\n", func_name, 
                    error ? error : "(no message)");
            lua_pop(engine->L, 1);
        }
    } else {
        lua_pop(engine->L, 1);
    }
}

static void print_usage(const char *program_name) {
    printf("Aphelion Game Engine v%s\n", APHELION_VERSION);
    printf("A single-file game engine with Lua scripting support.\n\n");
    printf("Usage: %s [options] [config.aphdat]\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n\n");
    printf("If no config file is provided, 'game.aphdat' will be used.\n");
    printf("If that doesn't exist, default settings will be applied.\n\n");
    printf("Config file format (.aphdat):\n");
    printf("  title = My Game Title\n");
    printf("  author = Your Name\n");
    printf("  api_version = %s\n", APHELION_VERSION);
    printf("  width = 1280\n");
    printf("  height = 960\n");
    printf("  fps = 60\n");
    printf("  script = main.lua\n");
    printf("  exit_key = DELETE\n");
    printf("  fullscreen = false\n");
    printf("  resizable = true\n");
    printf("  vsync = true\n\n");
    printf("Lua script should define these functions:\n");
    printf("  game_init()     - Called once at startup\n");
    printf("  game_update()   - Called every frame\n");
    printf("  game_draw()     - Called every frame for rendering\n");
    printf("  game_shutdown() - Called at exit\n\n");
    printf("For more information and examples, visit:\n");
    printf("  https://github.com/asmtilde/aphelion\n");
}

static void print_version(void) {
    printf("Aphelion Game Engine %s\n", APHELION_VERSION);
    printf("Built with Raylib and Lua\n");
    printf("Copyright (c) 2025 Jack Spencer\n");
}

int main(int argc, char **argv) {
    const char *config_path = ".aphdat";

    SetTraceLogLevel(LOG_WARNING);
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (argv[i][0] != '-') {
            config_path = argv[i];
        } else if (strcmp(argv[i], "--debug") == 0) {
        } else if (strcmp(argv[i], "--inf") == 0) {
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Use %s --help for usage information.\n", argv[0]);
            return 1;
        }
    }
    
    APHConfig config;
    aph_config_load_from_file(&config, config_path);
    
    if (config.width <= 0 || config.height <= 0) {
        fprintf(stderr, "Error: Invalid window dimensions (%dx%d)\n", 
                config.width, config.height);
        return 1;
    }
    
    if (config.fps <= 0) {
        fprintf(stderr, "Error: Invalid FPS value (%d)\n", config.fps);
        return 1;
    }
    
    if (strlen(config.script) == 0) {
        fprintf(stderr, "Warning: No script specified in '%s'. "
                       "Add 'script = main.lua' to your config file.\n", config_path);
    }

    if (strlen(config.api_version) == 0 || 
        strcmp(config.api_version, APHELION_VERSION) != 0) {
        fprintf(stderr, "Warning: API version mismatch. "
                        "Config version: '%s', Engine version: '%s'\n", 
                        config.api_version, APHELION_VERSION);
    }
    
    unsigned int flags = 0;
    if (config.resizable) flags |= FLAG_WINDOW_RESIZABLE;
    if (config.vsync) flags |= FLAG_VSYNC_HINT;
    
    if (flags != 0) SetConfigFlags(flags);
    
    InitWindow(config.width, config.height, config.title);
    
    if (config.fullscreen) {
        ToggleFullscreen();
    }
    
    SetTargetFPS(config.fps);
    SetExitKey(config.exit_key);
    
    Engine *engine = engine_init(&config);
    if (!engine) {
        fprintf(stderr, "Error: Failed to initialize engine.\n");
        CloseWindow();
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            engine->debug = 1;
            SetTraceLogLevel(LOG_DEBUG);
            printf("Debug mode enabled.\n");
        } else if (strcmp(argv[i], "--inf") == 0) {
            engine->ignore_not_found = 1;
        }
    }
    
    printf("Aphelion Engine %s ~ <3\n", APHELION_VERSION);
    printf("%s: %dx%d @ %d FPS\n", config.title, config.width, config.height, config.fps);

    bool script_loaded = false;
    if (strlen(config.script) > 0) {
        if (engine_run_script(engine, config.script)) {
            script_loaded = true;
            printf("Script '%s' loaded successfully.\n", config.script);
        } else {
            fprintf(stderr, "Error: Failed to load script '%s'\n", config.script);
        }
    }
    
    if (script_loaded) {
        printf("Calling game_init()...\n");
        engine_call_lua_function(engine, "game_init");
    }
    
    printf("Starting main loop...\n");
    
    while (!WindowShouldClose() && !engine->should_quit) {
        if (script_loaded) {
            engine_call_lua_function_arg(engine, "game_update", GetFrameTime());
        }
        
        BeginDrawing();
        
        ClearBackground(RAYWHITE);
        
        if (script_loaded) {
            engine_call_lua_function(engine, "game_draw");
        } else if (engine->ignore_not_found == 0) {
            DrawText("Aphelion Engine", 10, 10, 20, DARKGRAY);
            DrawText("No script loaded. Check your .aphdat configuration.", 10, 40, 16, GRAY);
            DrawText("Press DELETE to quit.", 10, 70, 16, GRAY);
        }
        
        if (engine->debug) {
            DrawText("DEBUG BUILD DO NOT DISTRIBUTE", 0, 0, 20, RED);
            DrawFPS(0, 20);
            DrawText(config.title, 0, 40, 20, DARKBLUE);
            DrawText(TextFormat("Resolution: %dx%d", GetScreenWidth(), GetScreenHeight()), 0, 60, 20, DARKBLUE);
            DrawText(TextFormat("Frame Time: %.3f ms", 1000.0f / (float)GetFPS()), 0, 80, 20, DARKBLUE);
            DrawText(TextFormat("Lua Memory: %.2f KB", (float)lua_gc(engine->L, LUA_GCCOUNT, 0)), 0, 100, 20, DARKBLUE);
        }
        
        EndDrawing();
    }
    
    if (script_loaded) {
        printf("Calling game_shutdown()...\n");
        engine_call_lua_function(engine, "game_shutdown");
    }
    
    printf("Shutting down engine...\n");
    engine_shutdown(engine);
    CloseWindow();
    
    printf("Aphelion: Goodbye!\n");
    return 0;
}
