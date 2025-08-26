// ==================== Makefile ====================
// Alternative build system for MinGW/Cygwin

# WOFL Ultra-Minimal IDE Makefile
# For use with MinGW-w64 or Cygwin

# Configuration
CC=gcc
CFLAGS=-O2 -Wall
LIBS=-lX11 -lpthread
wofl-ide: $(OBJS)
	$(CC) -o $@ $^ $(LIBS)