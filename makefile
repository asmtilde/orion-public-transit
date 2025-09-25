TARGET = aphelion
SOURCE = Aphelion.c

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
LIBS = -lraylib -llua -lm -ldl -lpthread

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LIBS)

debug: CFLAGS += -g -DDEBUG -O0
debug: $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all debug clean install uninstall run