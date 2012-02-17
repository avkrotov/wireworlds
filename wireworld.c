#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "SDL.h"
#include "SDL_gfxPrimitives.h" 

enum {
	WIDTH = 1024,
	HEIGHT = 1024
};

static SDL_Surface *screen;
static char map[2][HEIGHT + 2][WIDTH + 2];
static char neighbors[2][HEIGHT + 2][WIDTH + 2];
static int start[HEIGHT], end[HEIGHT];

static int h, w, z;

static void count(int x, int y, int nz) {
	neighbors[nz][y-1][x-1]++;
	neighbors[nz][y-1][x]++;
	neighbors[nz][y-1][x+1]++;
	neighbors[nz][y][x-1]++;
	neighbors[nz][y][x+1]++;
	neighbors[nz][y+1][x-1]++;
	neighbors[nz][y+1][x]++;
	neighbors[nz][y+1][x+1]++;
}

static void scanmap(char *path) {
	FILE *f;
	int x, y;

	memset(map, ' ', sizeof map);
	f = fopen(path, "r");
	if(f == NULL)
		err(1, "fopen");

	h = 1;

	while(fgets(map[0][h] + 1, WIDTH, f) != NULL && h < HEIGHT)
		h++;
	w = strlen(map[0][1] + 1);
	fclose(f);

	for(y = 1; y < h; y++) {
		start[y] = w;
		end[y] = 0;
		for(x = 1; x < w; x++) {
			if(map[0][y][x] != ' ') {
				if(start[y] > x)
					start[y] = x;
				end[y] = x + 1;
			}
			if(map[0][y][x] == '@')
				count(x, y, 0);
		}
	}
}

static void draw(void) {
	int x, y;

	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	for(y = 1; y < h; y++)
		for(x = start[y]; x < end[y]; x++)
			switch(map[z][y][x]) {
			case '@':
				pixelColor(screen, x, y, 0x0000ffff); 
				break;
			case '~':
				pixelColor(screen, x, y, 0xff0000ff); 
				break;
			case '#':
				pixelColor(screen, x, y, 0xffffffff);
			}
}


static void evolve(void) {
	int nz, x, y;

	nz = (z + 1) % 2;

	for(y = 1; y < h; y++)
		for(x = start[y]; x < end[y]; x++)
			neighbors[nz][y][x] = 0;

	for(y = 1; y < h; y++)
		for(x = start[y]; x < end[y]; x++)
			switch(map[z][y][x]) {
			case '@':
				map[nz][y][x] = '~';
				break;
			case '~':
				map[nz][y][x] = '#';
				break;
			case '#':
				if(neighbors[z][y][x] == 1 || neighbors[z][y][x] == 2) {
					map[nz][y][x] = '@';
					count(x, y, nz);
				} else
					map[nz][y][x] = '#';
			}

	z = nz;
}

int main(int argc, char *argv[]) {
	SDL_Event event;

	scanmap("1.txt");
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
		errx(1, "SDL_Init: %s", SDL_GetError());

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(w, h, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if(screen == NULL)
		errx(1, "Unable to set video: %s\n", SDL_GetError());

	while(1) {
		if(SDL_PollEvent(&event) && event.type == SDL_QUIT)
			exit(0);

		evolve();
		draw();
		SDL_Flip(screen);
	} 
}
