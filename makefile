CFLAGS = `sdl-config --cflags` -Wall -Wextra -O3
LDFLAGS = `sdl-config --libs` -l SDL_gfx

wireworld: wireworld.c

clean:
	rm -f wireworld
