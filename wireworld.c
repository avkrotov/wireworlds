#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "SDL.h"
#include "SDL_gfxPrimitives.h" 

enum {
	WIDTH = 1024,
	HEIGHT = 1024,
	THREADS = 2
};

static SDL_Surface *screen;
static char map[2][HEIGHT + 2][WIDTH + 2];
static unsigned char neighbors[2][HEIGHT + 2][WIDTH + 2];
static unsigned h, w, z, start[HEIGHT], end[HEIGHT];

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
	unsigned x, y;

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

static void process(int x, int y, int nz) {
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
}


static int thread_func(void *data) {
	unsigned *range;
	unsigned x, y;

	range = data;

	for(y = range[0]; y < range[1]; y++)
		for(x = start[y]; x < end[y]; x++)
			neighbors[!z][y][x] = 0;

	for(y = range[0]; y < range[1]; y++)
		for(x = start[y]; x < end[y]; x++)
			process(x, y, !z);
	
	return 0;
}

static unsigned range[THREADS][2];

static void evolve(void) {
	unsigned x, y, i;
	SDL_Thread *t[2];

	range[0][0] = 0;
	for(i = 0; i < THREADS - 1; i++) {
		range[i][1] = range[i][0] + h / THREADS - 1;
		range[i + 1][0] = range[i][1] + 1;
	}
	range[i][1] = h;

	y = h / 2 - 1;
	for(x = start[y]; x < end[y]; x++)
		neighbors[!z][y][x] = 0;

	for(i = 0; i < 2; i++)
		t[i] = SDL_CreateThread(thread_func, &range[i]);

	for(i = 0; i < 2; i++)
		SDL_WaitThread(t[i], NULL);

	for(x = start[y]; x < end[y]; x++)
		process(x, y, !z);

	z = !z;
}

static void draw(void) {
	unsigned x, y;

	for(y = 1; y < h; y++)
		for(x = start[y]; x < end[y]; x++)
			switch(map[z][y][x]) {
			case '@':
				pixelColor(screen, x, y, 0xff00ffff); 
				break;
			case '~':
				pixelColor(screen, x, y, 0xff00ffff); 
				break;
			case '#':
				pixelColor(screen, x, y, 0x0f0fffff);
			}
}

int main(int argc, char *argv[]) {
	SDL_Event event;
	Uint32 frames;

	if(argc != 2)
		return 1;
	scanmap(argv[1]);
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
		errx(1, "SDL_Init: %s", SDL_GetError());

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(w, h, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if(screen == NULL)
		errx(1, "Unable to set video: %s\n", SDL_GetError());

	frames = 0;

	do {
		evolve();
		evolve();
		evolve();
		draw();
		frames++;
		SDL_Flip(screen);
	} while(!SDL_PollEvent(&event) || event.type != SDL_QUIT);
	printf("%f\n", (double)SDL_GetTicks() / frames);
	return 0;
}
