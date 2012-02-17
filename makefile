CFLAGS = `sdl-config --cflags` -Wall -Wextra
LDFLAGS = `sdl-config --libs` -l SDL_gfx

wireworld: wireworld.c

clean:
	rm -f wireworld
