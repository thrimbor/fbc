#ifndef DISABLE_SDL2

#include "../fb_gfx.h"
#include <SDL2/SDL.h>

typedef struct SDL2_GFXDRIVER_CTX_
{
	SDL_Window *screen;
	SDL_Renderer *ren;
	SDL_Texture *canvas;
	SDL_Thread *thread;
	SDL_mutex *mutex;
	SDL_mutex *init_mutex;
	SDL_cond *init_cond;
	int init_done;
	int initialized;
	int is_running;
} SDL2_GFXDRIVER_CTX;

static SDL2_GFXDRIVER_CTX __sdl2_ctx = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, FALSE};

static int mouse_buttons = 0;
SDL_Color colors[256];

int driver_int_setup()
{
	__sdl2_ctx.screen = SDL_CreateWindow("gfxlib2 SDL2", 100, 100, __fb_gfx->w, __fb_gfx->h, SDL_WINDOW_SHOWN);
	if (__sdl2_ctx.screen == NULL)
	{
		SDL_Quit();
		return -1;
	}
	
	__sdl2_ctx.ren = SDL_CreateRenderer(__sdl2_ctx.screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (__sdl2_ctx.ren == NULL)
	{
		SDL_DestroyWindow(__sdl2_ctx.screen);
		__sdl2_ctx.screen = NULL;
		SDL_Quit();
		return -1;
	}
	
	__sdl2_ctx.canvas = SDL_CreateTexture(__sdl2_ctx.ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, __fb_gfx->w, __fb_gfx->h);
	if (__sdl2_ctx.canvas == NULL)
	{
		SDL_DestroyRenderer(__sdl2_ctx.ren);
		SDL_DestroyWindow(__sdl2_ctx.screen);
		__sdl2_ctx.screen = NULL;
		SDL_Quit();
		return -1;
	}
	
	__sdl2_ctx.mutex = SDL_CreateMutex();
	if (__sdl2_ctx.mutex == NULL)
	{
		SDL_DestroyTexture(__sdl2_ctx.canvas);
		SDL_DestroyRenderer(__sdl2_ctx.ren);
		SDL_DestroyWindow(__sdl2_ctx.screen);
		__sdl2_ctx.screen = NULL;
		SDL_Quit();
	}
	
	return 0;
}

void driver_int_cleanup ()
{
	SDL_DestroyRenderer(__sdl2_ctx.ren);
	SDL_DestroyWindow(__sdl2_ctx.screen);
	SDL_DestroyTexture(__sdl2_ctx.canvas);
	SDL_DestroyMutex(__sdl2_ctx.mutex);
	__sdl2_ctx.screen = NULL;
}

int driver_thread()
{
	/* Setup has to be done in this thread, otherwise the renderer won't work */
	driver_int_setup();
	
	/* Signal the main thread that initialization is done */
	SDL_LockMutex(__sdl2_ctx.init_mutex);
	__sdl2_ctx.init_done = TRUE;
	SDL_CondSignal(__sdl2_ctx.init_cond);
	SDL_UnlockMutex(__sdl2_ctx.init_mutex);
	
	SDL_RenderClear(__sdl2_ctx.ren);
	SDL_RenderPresent(__sdl2_ctx.ren);
	
	if (__fb_gfx->bpp != 4)
	{
		printf("!WARNING! bpp=%d is _very_ slow (and a mess) currently.\n", __fb_gfx->bpp);
	}
	
	while (__sdl2_ctx.is_running)
	{
		SDL_Event event;
		EVENT e;
		
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
				{
					printf("key pressed\n");
					__fb_gfx->key[SC_RIGHT] = TRUE;
					/*
					e.scancode = event.key.keysym.sym;
					e.ascii = 0;
					e.scancode = SC_RIGHT;
					fb_hPostKey(e.scancode);
					e.type = EVENT_KEY_PRESS;
					event.key.keysym.sym;
					fb_hPostEvent(&e);*/
					break;
				}
				
				case SDL_KEYUP:
				{
					__fb_gfx->key[SC_RIGHT] = FALSE;
					break;
				}
				
				case SDL_MOUSEBUTTONDOWN:
				{
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT: mouse_buttons |= BUTTON_LEFT; e.button = BUTTON_LEFT; break;
						case SDL_BUTTON_RIGHT: mouse_buttons |= BUTTON_RIGHT; e.button = BUTTON_RIGHT; break;
						case SDL_BUTTON_MIDDLE: mouse_buttons |= BUTTON_MIDDLE; e.button = BUTTON_MIDDLE; break;
					}
					
					e.type = EVENT_MOUSE_BUTTON_PRESS;
					fb_hPostEvent(&e);
					break;
				}
				
				case SDL_MOUSEBUTTONUP:
				{
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT: mouse_buttons &= ~BUTTON_LEFT; e.button = BUTTON_LEFT; break;
						case SDL_BUTTON_RIGHT: mouse_buttons &= ~BUTTON_RIGHT; e.button = BUTTON_RIGHT; break;
						case SDL_BUTTON_MIDDLE: mouse_buttons &= ~BUTTON_MIDDLE; e.button = BUTTON_MIDDLE; break;
					}
					
					e.type = EVENT_MOUSE_BUTTON_RELEASE;
					fb_hPostEvent(&e);
					break;
				}
			}
		}
		
		
		int err = 0;
		
		if (__fb_gfx->bpp == 4)
		{
			SDL_LockMutex(__sdl2_ctx.mutex);
			SDL_UpdateTexture(__sdl2_ctx.canvas, NULL, __fb_gfx->framebuffer, __fb_gfx->pitch);
			SDL_UnlockMutex(__sdl2_ctx.mutex);
			err = SDL_RenderCopy(__sdl2_ctx.ren, __sdl2_ctx.canvas, NULL, NULL);
			if (err != 0)
				printf("SDL_RenderCopy failed!\n");
		}
		else
		{
			SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(__fb_gfx->framebuffer, __fb_gfx->w, __fb_gfx->h, __fb_gfx->bpp*8, __fb_gfx->pitch, 0,0,0,0);
			
			SDL_SetPaletteColors(surface->format->palette, colors, 0, 255);
			
			
			SDL_Texture *tex = SDL_CreateTextureFromSurface(__sdl2_ctx.ren, surface);
			SDL_RenderCopy(__sdl2_ctx.ren, tex, NULL, NULL);
			SDL_DestroyTexture(tex);
			SDL_FreeSurface(surface);
		}
		
		SDL_RenderPresent(__sdl2_ctx.ren);
	}
	
	driver_int_cleanup();
	return 0;
}

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
		__sdl2_ctx.initialized = TRUE;
	}
	
	__sdl2_ctx.init_mutex = SDL_CreateMutex();
	__sdl2_ctx.init_cond = SDL_CreateCond();
	
	
	__sdl2_ctx.is_running = TRUE;
	__sdl2_ctx.thread = SDL_CreateThread(driver_thread, "FB.gfx.sdl2.thread", NULL);
	
	SDL_LockMutex(__sdl2_ctx.init_mutex);
	while (!__sdl2_ctx.init_done)
		SDL_CondWait(__sdl2_ctx.init_cond, __sdl2_ctx.init_mutex);
	SDL_UnlockMutex(__sdl2_ctx.init_mutex);
	
	SDL_SetWindowTitle(__sdl2_ctx.screen, title);

	return 0;
};

static void driver_exit(void)
{
	if (__sdl2_ctx.initialized)
	{
		__sdl2_ctx.is_running = FALSE;
		SDL_WaitThread(__sdl2_ctx.thread, NULL);
		SDL_DestroyCond(__sdl2_ctx.init_cond);
		SDL_DestroyMutex(__sdl2_ctx.init_mutex);
		__sdl2_ctx.init_done = FALSE;
		
		SDL_Quit();
	}
}

static void driver_lock(void)
{
	SDL_LockMutex(__sdl2_ctx.mutex);
}

static void driver_unlock(void)
{
	SDL_UnlockMutex(__sdl2_ctx.mutex);
}

static void driver_set_palette(int index, int r, int g, int b)
{
	colors[index].r = r;
	colors[index].g = g;
	colors[index].b = b;
}

static void driver_wait_vsync(void)
{
	
}

static int driver_get_mouse(int *x, int *y, int *z, int *buttons, int *clip)
{
	// FIXME: what about z-coordinate?
	// FIXME: what about clip?
	// FIXME: shouldn't we return -1 when the mouse is outside of the window
	
	int t = SDL_GetMouseState(x,y);
	
	*buttons = 0;
	if (t & SDL_BUTTON(SDL_BUTTON_LEFT)) *buttons |= BUTTON_LEFT;
	if (t & SDL_BUTTON(SDL_BUTTON_RIGHT)) *buttons |= BUTTON_RIGHT;
	if (t & SDL_BUTTON(SDL_BUTTON_MIDDLE)) *buttons |= BUTTON_MIDDLE;
	return 0;
}

/* functionality not enabled because it doesn't work as expected
 * (for example it creates a mouse move event)
 */
/*	 
static void driver_set_mouse(int x, int y, int cursor, int clip)
{
	SDL_WarpMouseInWindow(__sdl2_ctx.screen, x, y);
}
*/

static void driver_set_window_title(char *title)
{
	SDL_SetWindowTitle(__sdl2_ctx.screen, title);
}

static int *driver_fetch_modes(int depth, int *size)
{
	return 0;
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
	NULL,	/* void (*set_mouse)(int x, int y, int cursor, int clip); */
	driver_set_window_title,	/* void (*set_window_title)(char *title); */
	NULL,	/* int (*set_window_pos)(int x, int y); */
	driver_fetch_modes,	/* int *(*fetch_modes)(void); */
	NULL,			/* void (*flip)(void); */
	driver_poll_events			/* void (*poll_events)(void); */
};


#endif 
