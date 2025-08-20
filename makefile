CXX = g++
CXXFLAGS = -Wall -Wextra -O2 \
	$(shell pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer jansson)
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer jansson) -lyaml

SRC = src.cpp
TARGET = opt_game

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) src.o

.PHONY: all clean
