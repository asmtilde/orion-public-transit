#ifndef JSON_H
#define JSON_h

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

void json_read(const char* filename, json_t** root);
void json_write(const char* filename, json_t* root);
void json_free(json_t* root);

#endif