CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lraylib -llua -lm

TARGET = optgame
SOURCES = src/main.c src/engine.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET)

install-deps-ubuntu:
	sudo apt-get install libraylib-dev liblua5.4-dev

install-deps-arch:
	sudo pacman -S raylib lua

install-deps-macos:
	brew install raylib lua

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean install-deps-ubuntu install-deps-arch install-deps-macos run