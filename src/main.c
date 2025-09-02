#include <raylib.h>
#include "engine.h"

int main(void) {
    InitWindow(800, 600, "Lua-Driven RPG");
    SetTargetFPS(60);
    
    Engine *engine = engine_init();
    
    // Load the main Lua script
    if (!engine_run_script(engine, "resources/scripts/main.lua")) {
        printf("Failed to load main.lua\n");
        engine_shutdown(engine);
        CloseWindow();
        return -1;
    }
    
    // Check if Lua has an init function and call it
    lua_getglobal(engine->L, "init");
    if (lua_isfunction(engine->L, -1)) {
        if (lua_pcall(engine->L, 0, 0, 0) != LUA_OK) {
            printf("Lua Error in init(): %s\n", lua_tostring(engine->L, -1));
            lua_pop(engine->L, 1);
        }
    } else {
        lua_pop(engine->L, 1);
    }
    
    // Main game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Call Lua's update function
        lua_getglobal(engine->L, "update");
        if (lua_isfunction(engine->L, -1)) {
            lua_pushnumber(engine->L, GetFrameTime()); // Pass delta time
            if (lua_pcall(engine->L, 1, 0, 0) != LUA_OK) {
                printf("Lua Error in update(): %s\n", lua_tostring(engine->L, -1));
                lua_pop(engine->L, 1);
            }
        } else {
            lua_pop(engine->L, 1);
        }
        
        // Call Lua's draw function
        lua_getglobal(engine->L, "draw");
        if (lua_isfunction(engine->L, -1)) {
            if (lua_pcall(engine->L, 0, 0, 0) != LUA_OK) {
                printf("Lua Error in draw(): %s\n", lua_tostring(engine->L, -1));
                lua_pop(engine->L, 1);
            }
        } else {
            lua_pop(engine->L, 1);
        }
        
        EndDrawing();
    }
    
    // Call Lua cleanup if it exists
    lua_getglobal(engine->L, "cleanup");
    if (lua_isfunction(engine->L, -1)) {
        if (lua_pcall(engine->L, 0, 0, 0) != LUA_OK) {
            printf("Lua Error in cleanup(): %s\n", lua_tostring(engine->L, -1));
            lua_pop(engine->L, 1);
        }
    } else {
        lua_pop(engine->L, 1);
    }
    
    engine_shutdown(engine);
    CloseWindow();
    
    return 0;
}