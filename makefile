CC = gcc
CFLAGS = -Wall -Wextra -O2 \
	`pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer jansson`
LDFLAGS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer jansson` -lyaml

SRC := $(shell find src -name '*.c')
OBJ := $(SRC:.c=.o)
TARGET := opt_game

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)
	find src -name '*.o' -delete

.PHONY: all clean