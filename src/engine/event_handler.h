#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <SDL2/SDL.h>

typedef struct EventHandler EventHandler;

typedef void (*EventCallback)(SDL_Event* event, void* user_data);

void event_handler_register(EventHandler* handler, Uint32 event_type, EventCallback callback, void* user_data);

void event_handler_dispatch(EventHandler* handler, SDL_Event* event);
EventHandler* event_handler_create(void* app);
void event_handler_destroy(EventHandler* handler);

#endif