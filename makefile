CFLAGS = `sdl2-config --cflags` -Wall -Wextra -Ofast
LDFLAGS = `sdl2-config --libs`

wireworld: wireworld.c

clean:
	rm -f wireworld
