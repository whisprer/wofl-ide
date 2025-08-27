CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pthread
LIBS=-lSDL2 -lSDL2_ttf
TARGET=wofl-sdl2

SOURCES=main.c gap_buffer.c cursor.c editing.c find.c file_ops.c language.c rendering.c input.c sdl_utils.c syntax.c
OBJECTS=$(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean