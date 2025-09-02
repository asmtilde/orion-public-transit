#ifndef ENGINE_H
#define ENGINE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <raylib.h>

typedef struct {
    lua_State *L;
} Engine;

Engine *engine_init(void);
void engine_shutdown(Engine *engine);
int engine_run_script(Engine *engine, const char *filename);
void engine_register_functions(Engine *engine);

#endif