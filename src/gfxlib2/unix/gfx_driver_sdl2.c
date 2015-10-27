#ifndef DISABLE_SDL2

#include "../fb_gfx.h"
#include <SDL2/SDL.h>

typedef struct SDL2_GFXDRIVER_CTX_
{
	SDL_Window *screen;
	SDL_Renderer *ren;
	SDL_Texture *canvas;
	int initialized;
} SDL2_GFXDRIVER_CTX;

static SDL2_GFXDRIVER_CTX __sdl2_ctx = {NULL, NULL, NULL, FALSE};

static int driver_init(char *title, int w, int h, int depth, int refresh_rate, int flags)
{
	if (w==0 || h==0 || depth==0)
		return 0;
	
	/* initialize SDL on first call */
	if (!__sdl2_ctx.initialized)
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			return -1;
		}
	}
	
	// FIXME: We should clean up old state here
	
	__sdl2_ctx.screen = SDL_CreateWindow(title, 100, 100, w, h, SDL_WINDOW_SHOWN);
	if (__sdl2_ctx.screen == NULL)
	{
		SDL_Quit();
		return -1;
	}
	
	__sdl2_ctx.ren = SDL_CreateRenderer(__sdl2_ctx.screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (__sdl2_ctx.ren == NULL)
	{
		SDL_DestroyWindow(__sdl2_ctx.screen);
		SDL_Quit();
		return -1;
	}
	
	__sdl2_ctx.canvas = SDL_CreateTexture(__sdl2_ctx.ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (__sdl2_ctx.canvas == NULL)
	{
		SDL_DestroyRenderer(__sdl2_ctx.ren);
		SDL_DestroyWindow(__sdl2_ctx.screen);
		SDL_Quit();
		return -1;
	}
	
	SDL_RenderClear(__sdl2_ctx.ren);
	SDL_RenderPresent(__sdl2_ctx.ren);
	
	__sdl2_ctx.initialized = TRUE;
	return 0;
};

static void driver_exit(void)
{
	if (__sdl2_ctx.initialized)
	{
		if (__sdl2_ctx.screen != NULL)
		{
			SDL_DestroyRenderer(__sdl2_ctx.ren);
			SDL_DestroyWindow(__sdl2_ctx.screen);
			SDL_DestroyTexture(__sdl2_ctx.canvas);
			__sdl2_ctx.screen = NULL;
		}
		
		SDL_Quit();
	}
	
	return 0;
}

static void driver_lock(void)
{
	
}

static void driver_unlock(void)
{
	
}

static void driver_set_palette(int index, int r, int g, int b)
{
	
}

static void driver_wait_vsync(void)
{
	
}

static int driver_get_mouse(int *x, int *y, int *z, int *buttons, int *clip)
{
	
}

static void driver_set_mouse(int x, int y, int cursor, int clip)
{
	
}

static int *driver_fetch_modes(int depth, int *size)
{
	
}

static void driver_poll_events(void)
{
	
}


GFXDRIVER fb_gfxDriverSDL2 =
{
	"SDL2",				/* char *name; */
	driver_init,		/* int (*init)(int w, int h, char *title, int fullscreen); */
	driver_exit,		/* void (*exit)(void); */
	driver_lock,		/* void (*lock)(void); */
	driver_unlock,		/* void (*unlock)(void); */
	driver_set_palette,	/* void (*set_palette)(int index, int r, int g, int b); */
	driver_wait_vsync,	/* void (*wait_vsync)(void); */
	driver_get_mouse,	/* int (*get_mouse)(int *x, int *y, int *z, int *buttons, int *clip); */
	driver_set_mouse,	/* void (*set_mouse)(int x, int y, int cursor, int clip); */
	NULL,	/* void (*set_window_title)(char *title); */
	NULL,	/* int (*set_window_pos)(int x, int y); */
	driver_fetch_modes,	/* int *(*fetch_modes)(void); */
	NULL,			/* void (*flip)(void); */
	driver_poll_events			/* void (*poll_events)(void); */
};


#endif 
