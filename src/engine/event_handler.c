#include "event_handler.h"
#include <stdlib.h>
#include <string.h>

typedef struct EventCallbackNode {
    Uint32 event_type;
    EventCallback callback;
    void* user_data;
    struct EventCallbackNode* next;
} EventCallbackNode;

struct EventHandler {
    EventCallbackNode* head;
    void* app;
};

EventHandler* event_handler_create(void* app) {
    EventHandler* handler = (EventHandler*)malloc(sizeof(EventHandler));
    handler->head = NULL;
    handler->app = app;
    return handler;
}

void event_handler_register(EventHandler* handler, Uint32 event_type, EventCallback callback, void* user_data) {
    EventCallbackNode* node = (EventCallbackNode*)malloc(sizeof(EventCallbackNode));
    node->event_type = event_type;
    node->callback = callback;
    node->user_data = user_data;
    node->next = handler->head;
    handler->head = node;
}

void event_handler_dispatch(EventHandler* handler, SDL_Event* event) {
    EventCallbackNode* node = handler->head;
    while (node) {
        if (node->event_type == event->type) {
            node->callback(event, node->user_data);
        }
        node = node->next;
    }
}

void event_handler_destroy(EventHandler* handler) {
    EventCallbackNode* node = handler->head;
    while (node) {
        EventCallbackNode* next = node->next;
        free(node);
        node = next;
    }
    free(handler);
}