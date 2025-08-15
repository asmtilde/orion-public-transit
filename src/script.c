#include "script.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

// Helper: trim leading whitespace
static char* ltrim(char* s) {
    while(isspace((unsigned char)*s)) s++;
    return s;
}

// Helper: extract first word, returns pointer into line or NULL
static char* grab_first_word(char* line) {
    char* start = ltrim(line);
    if (*start == '\0' || *start == '#') return NULL;
    return start;
}

bool read_script(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Failed to open script file");
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline, if any
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

        char* first = grab_first_word(line);
        if (!first) continue; // empty or comment

        if (strncmp(first, "PRINT", 5) == 0 && 
            (first[5] == ' ' || first[5] == '\0')) {
            char* msg = first + 5;
            msg = ltrim(msg);
            if (*msg)
                printf("%s\n", msg);
            continue;
        }

        fprintf(stderr, "Unknown command: %.*s\n", (int)strcspn(first, " \t"), first);
    }

    fclose(file);
    return true;
}