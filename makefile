CC = gcc
CFLAGS = -Wall -Wextra -O2 \
	`sdl2-config --cflags` \
	`pkg-config --cflags SDL2_image SDL2_ttf SDL2_mixer jansson`
LDFLAGS = `sdl2-config --libs` \
	`pkg-config --libs SDL2_image SDL2_ttf SDL2_mixer jansson`

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
