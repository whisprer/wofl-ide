// ==================== Makefile ====================
// Alternative build system for MinGW/Cygwin

# WOFL Ultra-Minimal IDE Makefile
# For use with MinGW-w64 or Cygwin

# Configuration
CC = gcc
OUTPUT_DIR = dist
EXE_NAME = wofl-ide.exe
CFLAGS = -Wall -Wextra -O2 -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN
LDFLAGS = -mwindows
LIBS = -lgdi32 -luser32 -lcomdlg32 -lshell32 -lole32

# Source files
CORE_SOURCES = src/main_win32.c \
               src/editor_buffer.c \
               src/editor_render.c

SYNTAX_SOURCES = src/syntax_core.c \
                 src/syntax_c.c \
                 src/syntax_py.c \
                 src/syntax_js.c \
                 src/syntax_html.c \
                 src/syntax_css.c \
                 src/syntax_json.c \
                 src/syntax_md.c \
                 src/syntax_go.c \
                 src/syntax_rs.c \
                 src/syntax_sh.c \
                 src/syntax_lua.c

FEATURE_SOURCES = src/find.c \
                  src/cmd_palette.c \
                  src/config.c \
                  src/run_exec_win32.c

ALL_SOURCES = $(CORE_SOURCES) $(SYNTAX_SOURCES) $(FEATURE_SOURCES)
OBJECTS = $(ALL_SOURCES:.c=.o)

# Targets
.PHONY: all clean debug

all: $(OUTPUT_DIR)/$(EXE_NAME)

$(OUTPUT_DIR)/$(EXE_NAME): $(OBJECTS)
	@mkdir -p $(OUTPUT_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)
	@echo "Build successful: $@"
	@ls -lh $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = -Wall -Wextra -g -O0 -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -D_DEBUG
debug: $(OUTPUT_DIR)/$(EXE_NAME)

clean:
	rm -f src/*.o $(OUTPUT_DIR)/$(EXE_NAME)

# Dependencies
src/main_win32.o: src/editor.h
src/editor_buffer.o: src/editor.h
src/editor_render.o: src/editor.h
src/syntax_core.o: src/editor.h
src/find.o: src/editor.h
src/cmd_palette.o: src/editor.h
src/config.o: src/editor.h
src/run_exec_win32.o: src/editor.h
