# Makefile for WOFL IDE SDL2 port
# Compiles modularized source files into the wofl_ide executable

CC = gcc
CFLAGS = -Wall -O2 -I. $(shell pkg-config --cflags sdl2 SDL2_ttf)
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_ttf)
TARGET = wofl_ide

# Source and object files
SRCS = main.c gap_buffer.c file_ops.c language.c rendering.c cursor.c editing.c find.c sdl_utils.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies for each object file
main.o: main.c app.h gap_buffer.h file_ops.h language.h rendering.h input.h sdl_utils.h
gap_buffer.o: gap_buffer.c gap_buffer.h app.h
file_ops.o: file_ops.c file_ops.h gap_buffer.h app.h
language.o: language.c language.h app.h
rendering.o: rendering.c rendering.h gap_buffer.h app.h
cursor.o: cursor.c cursor.h gap_buffer.h app.h
editing.o: editing.c editing.h gap_buffer.h cursor.h app.h
find.o: find.c find.h gap_buffer.h cursor.h app.h
input.o: input.c input.h file_ops.h language.h find.h cursor.h editing.h app.h
sdl_utils.o: sdl_utils.c sdl_utils.h gap_buffer.h app.h

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean