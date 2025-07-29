// Script loader
#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

static char* ltrim(char* s);
static char* grab_first_word(char* line);
bool read_script(const char* filepath);