CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic

all: chip8

chip8:
	$(CC) $(CFLAGS) main.c chip8.c sdl.c sdl_audio.c -lSDL3 -lm -o chip8

clean:
	rm -rf chip8