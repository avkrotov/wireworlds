CFLAGS = `sdl-config --cflags` -Wall -Wextra -Ofast
LDFLAGS = `sdl-config --libs` -l SDL_gfx

wireworld: wireworld.c

clean:
	rm -f wireworld
