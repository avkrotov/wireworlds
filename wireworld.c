#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <SDL.h>

enum {
	WIDTH = 1024,
	HEIGHT = 1024,
	THREADS = 4
};

static SDL_Window *window;
static SDL_Renderer *renderer;
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

static void tick(int draw) {
	unsigned x, y, i;
	SDL_Thread *t[THREADS];

	range[0][0] = 0;
	for(i = 0; i < THREADS - 1; i++) {
		range[i][1] = range[i][0] + h / THREADS - 1;
		y = range[i][1];
		for(x = start[y]; x < end[y]; x++)
			neighbors[!z][y][x] = 0;
		range[i + 1][0] = y + 1;
	}
	range[i][1] = h;

	for(i = 0; i < THREADS; i++)
		t[i] = SDL_CreateThread(thread_func, "", &range[i]);

	if(draw)
		for(y = 1; y < h; y++)
			for(x = start[y]; x < end[y]; x++) {
				switch(map[z][y][x]) {
				case '@':
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					break;
				case '~':
					SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
					break;
				case '#':
					SDL_SetRenderDrawColor(renderer, 15, 15, 255, 255);
					break;
				default:
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				}
				SDL_RenderDrawPoint(renderer, x, y);
			}

	for(i = 0; i < THREADS; i++)
		SDL_WaitThread(t[i], NULL);

	for(i = 0; i < THREADS - 1; i++) {
		y = range[i][1];
		for(x = start[y]; x < end[y]; x++)
			process(x, y, !z);
	}

	z = !z;
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

	window = SDL_CreateWindow("Wireworlds", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
	if(window == NULL)
		errx(1, "SDL_CreateWindow: %s\n", SDL_GetError());
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

	frames = 0;

	do {
		tick(1);
		frames++;
		SDL_RenderPresent(renderer);
	} while(!SDL_PollEvent(&event) || event.type != SDL_QUIT);
	printf("%f\n", (double)frames / SDL_GetTicks());
	return 0;
}
