/*
 *  libgfx2 - FreeBASIC's alternative gfx library
 *	Copyright (C) 2005 Angelo Mottola (a.mottola@libero.it)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 * fb_gfx.h -- internal gfx definitions
 *
 * chng: jan/2005 written [lillo]
 *
 */

#ifndef __FB_GFX_H__
#define __FB_GFX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../rtlib/fb.h"
#include "../rtlib/fb_error.h"
#include "../rtlib/fb_scancodes.h"

#ifdef PI
#undef PI
#endif
#define PI			3.1415926535897932384626

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b)		((a) < (b) ? (a) : (b))

#ifdef MAX
#undef MAX
#endif
#define MAX(a,b)		((a) > (b) ? (a) : (b))

#ifdef MID
#undef MID
#endif
#define MID(a,b,c)		MIN(MAX((a), (b)), (c))

#ifdef SWAP
#undef SWAP
#endif
#define SWAP(a,b)		((a) ^= (b), (b) ^= (a), (a) ^= (b))

#ifdef TARGET_X86
 #define RORW(num, bits) __asm__ __volatile__("rorw %1, %0" : "=m"(num) : "c"(bits) : "memory")
 #define RORW1(num)      __asm__ __volatile__("rorw $1, %0" : "=m"(bit) : : "memory");
#else
 #define RORW(num, bits) num = ( (num) >> (bits) ) | (num << (16 - bits) )
 #define RORW1(num)      RORW(num, 1)
#endif

#define BYTES_PER_PIXEL(d)	(((d) + 7) / 8)

#define DRIVER_LOCK()		{ if (!(fb_mode->flags & (SCREEN_LOCKED | SCREEN_AUTOLOCKED))) { fb_mode->driver->lock(); fb_mode->flags |= SCREEN_LOCKED | SCREEN_AUTOLOCKED; } }
#define DRIVER_UNLOCK()		{ if (fb_mode->flags & SCREEN_AUTOLOCKED) { fb_mode->driver->unlock(); fb_mode->flags &= ~(SCREEN_LOCKED | SCREEN_AUTOLOCKED); } }
#define SET_DIRTY(y,h)		{ if (fb_mode->framebuffer == fb_mode->line[0]) fb_hMemSet(fb_mode->dirty + (y), TRUE, (h)); }

#define DRIVER_NULL		-1
#define DRIVER_FULLSCREEN	0x00000001
#define DRIVER_OPENGL		0x00000002
#define DRIVER_NO_SWITCH	0x00000004
#define DRIVER_NO_FRAME		0x00000008
#define DRIVER_SHAPED_WINDOW	0x00000010
#define DRIVER_OPENGL_OPTIONS	0x000F0000
#define HAS_STENCIL_BUFFER	0x00010000
#define HAS_ACCUMULATION_BUFFER	0x00020000

#define HAS_MMX			0x01000000
#define SCREEN_EXIT		0x80000000
#define DEFAULT_COLOR		0xFEFF00FF
#define WINDOW_ACTIVE		0x00000001
#define WINDOW_SCREEN		0x00000002
#define VIEW_SCREEN		0x00000004
#define BUFFER_SET		0x00000008
#define SCREEN_LOCKED		0x00000010
#define SCREEN_AUTOLOCKED	0x00000020
#define PRINT_SCROLL_WAS_OFF	0x00000040
#define VIEW_PORT_SET		0x00000080

#define COORD_TYPE_AA		0
#define COORD_TYPE_AR		1
#define COORD_TYPE_RA		2
#define COORD_TYPE_RR		3
#define COORD_TYPE_A		4
#define COORD_TYPE_R		5

#define LINE_TYPE_LINE		0
#define LINE_TYPE_B		1
#define LINE_TYPE_BF		2

#define PAINT_TYPE_FILL		0
#define PAINT_TYPE_PATTERN	1

#define MASK_COLOR_32		0xFF00FF
#define MASK_COLOR_16		0xF81F

#define MASK_RB_32			0x00FF00FF
#define MASK_G_32			0x0000FF00

#define MASK_RB_16			0xF81F
#define MASK_R_16			0xF800
#define MASK_G_16			0x07E0
#define MASK_B_16			0x001F

#define PUT_MODE_TRANS		0
#define PUT_MODE_PSET		1
#define PUT_MODE_PRESET		2
#define PUT_MODE_AND		3
#define PUT_MODE_OR		4
#define PUT_MODE_XOR		5
#define PUT_MODE_ALPHA		6
#define PUT_MODE_ADD		7
#define PUT_MODE_CUSTOM		8

#define KEY_BUFFER_LEN		16

#define KEY_QUIT		0x100
#define KEY_UP			0x101
#define KEY_DOWN		0x102
#define KEY_LEFT		0x103
#define KEY_RIGHT		0x104
#define KEY_INS			0x105
#define KEY_DEL			0x106
#define KEY_HOME		0x107
#define KEY_END			0x108
#define KEY_PAGE_UP		0x109
#define KEY_PAGE_DOWN		0x10A
#define KEY_F(n)		(0x10A + (n))
#define KEY_MAX_SPECIALS	(KEY_F(10) - 0x100 + 1)

#define WINDOW_TITLE_SIZE	128

typedef struct _GFX_CHAR_CELL {
    FB_WCHAR ch;
    unsigned fg, bg;
} GFX_CHAR_CELL;

typedef struct MODE
{
    int mode_num;				/* Current mode number */
    unsigned char **page;			/* Pages memory */
    int num_pages;				/* Number of requested pages */
    int work_page;				/* Current work page number */
    unsigned char *framebuffer;			/* Our current visible framebuffer */
    unsigned char **line;			/* Line pointers into current active framebuffer */
    int pitch;					/* Width of a framebuffer line in bytes */
    int target_pitch;				/* Width of current target buffer line in bytes */
    void *last_target;				/* Last target buffer set */
    int max_h;					/* Max registered height of target buffer */
    int bpp;					/* Bytes per pixel */
    unsigned int *palette;			/* Current RGB color values for each palette index */
    unsigned int *device_palette;		/* Current RGB color values of visible device palette */
    unsigned char *color_association;		/* Palette color index associations for CGA/EGA emulation */
    char *dirty;				/* Dirty lines buffer */
    const struct GFXDRIVER *driver;		/* Gfx driver in use */
    int w, h;					/* Current mode width and height */
    int depth;					/* Current mode depth */
    int color_mask;				/* Color bit mask for colordepth emulation */
    const struct PALETTE *default_palette;	/* Default palette for current mode */
    int scanline_size;				/* Vertical size of a single scanline in pixels */
    unsigned int fg_color, bg_color;		/* Current foreground and background colors */
    float last_x, last_y;			/* Last pen position */
    int cursor_x, cursor_y;			/* Current graphical text cursor position (in chars, 0 based) */
    const struct FONT *font;			/* Current font */
    int view_x, view_y, view_w, view_h;		/* VIEW coordinates */
    float win_x, win_y, win_w, win_h;		/* WINDOW coordinates */
    int text_w, text_h;				/* Graphical text console size in characters */
    char *key;					/* Keyboard states */
    int refresh_rate;				/* Driver refresh rate */
    int flags;					/* Status flags */
    GFX_CHAR_CELL **con_pages;                  /* Character information for all pages */
} MODE;


typedef struct GFXDRIVER
{
	char *name;
	int (*init)(char *title, int w, int h, int depth, int refresh_rate, int flags);
	void (*exit)(void);
	void (*lock)(void);
	void (*unlock)(void);
	void (*set_palette)(int index, int r, int g, int b);
	void (*wait_vsync)(void);
	int (*get_mouse)(int *x, int *y, int *z, int *buttons);
	void (*set_mouse)(int x, int y, int cursor);
	void (*set_window_title)(char *title);
	int *(*fetch_modes)(int depth, int *size);
	void (*flip)(void);
} GFXDRIVER;


typedef struct PALETTE
{
	const int colors;
	const unsigned char *data;
} PALETTE;


typedef struct FONT
{
    const int w;
    const int h;
	const unsigned char *data;
} FONT;


typedef void (BLITTER)(unsigned char *, int);
typedef FBCALL unsigned int (BLENDER)(unsigned int, unsigned int, void *);
typedef void (PUTTER)(unsigned char *, unsigned char *, int, int, int, int, BLENDER *, void *);

/* Global variables */
extern MODE *fb_mode;
extern const GFXDRIVER *fb_gfx_driver_list[];
extern const GFXDRIVER fb_gfxDriverNull;
extern void *(*fb_hMemCpy)(void *dest, const void *src, size_t size);
extern void *(*fb_hMemSet)(void *dest, int value, size_t size);
extern void (*fb_hPutPixel)(int x, int y, unsigned int color);
extern unsigned int (*fb_hGetPixel)(int x, int y);
extern void *(*fb_hPixelCpy)(void *dest, const void *src, size_t size);
extern void *(*fb_hPixelSet)(void *dest, int color, size_t size);
extern unsigned int *fb_color_conv_16to32;
#include "fb_gfx_data.h"

/* Internal functions */
extern void fb_hSetupFuncs(void);
extern void fb_hSetupData(void);
extern FBCALL int fb_hEncode(const unsigned char *in_buffer, int in_size, unsigned char *out_buffer, int *out_size);
extern FBCALL int fb_hDecode(const unsigned char *in_buffer, int in_size, unsigned char *out_buffer, int *out_size);
extern void fb_hPostKey(int key);
extern BLITTER *fb_hGetBlitter(int device_depth, int is_rgb);
extern PUTTER *fb_hGetPutter(int mode, int *alpha);
extern unsigned int fb_hMakeColor(unsigned int index, int r, int g, int b);
extern unsigned int fb_hFixColor(unsigned int color);
extern void fb_hRestorePalette(void);
extern void fb_hPrepareTarget(void *target);
extern void fb_hTranslateCoord(float fx, float fy, int *x, int *y);
extern void fb_hFixRelative(int coord_type, float *x1, float *y1, float *x2, float *y2);
extern void fb_hFixCoordsOrder(int *x1, int *y1, int *x2, int *y2);
extern void fb_hGfxBox(int x1, int y1, int x2, int y2, unsigned int color, int full, unsigned int style);
extern void fb_hScreenInfo(int *width, int *height, int *depth, int *refresh);
extern void *fb_hMemCpyMMX(void *dest, const void *src, size_t size);
extern void *fb_hMemSetMMX(void *dest, int value, size_t size);
extern void fb_hResetCharCells(int do_alloc);
extern void fb_hClearCharCells(int x1, int y1, int x2, int y2, int page, FB_WCHAR ch, unsigned fg, unsigned bg);
extern void fb_hSoftCursorInit(void);
extern void fb_hSoftCursorExit(void);
extern void fb_hSoftCursorPut(int x, int y);
extern void fb_hSoftCursorUnput(int x, int y);
extern void fb_hSoftCursorPaletteChanged(void);


/* Public API */
extern FBCALL int fb_GfxScreen(int mode, int depth, int num_pages, int flags, int refresh_rate);
extern FBCALL int fb_GfxScreenRes(int width, int height, int depth, int num_pages, int flags, int refresh_rate);
extern FBCALL void fb_GfxScreenInfo(int *width, int *height, int *depth, int *bpp, int *pitch, int *refresh_rate, FBSTRING *driver);
extern FBCALL void *fb_GfxImageCreate(int width, int height, unsigned int color);
extern FBCALL void fb_GfxImageDestroy(void *image);
extern FBCALL void fb_GfxPalette(int index, int r, int g, int b);
extern FBCALL void fb_GfxPaletteUsing(int *data);
extern FBCALL void fb_GfxPaletteGet(int index, int *r, int *g, int *b);
extern FBCALL void fb_GfxPaletteGetUsing(int *data);
extern FBCALL void fb_GfxPset(void *target, float x, float y, unsigned int color, int coord_type, int ispreset);
extern FBCALL int fb_GfxPoint(void *target, float x, float y);
extern FBCALL float fb_GfxPMap(float coord, int func);
extern FBCALL float fb_GfxCursor(int func);
extern FBCALL void fb_GfxView(int x1, int y1, int x2, int y2, unsigned int fill_color, unsigned int border_color, int screen);
extern FBCALL void fb_GfxWindow(float x1, float y1, float x2, float y2, int screen);
extern FBCALL void fb_GfxLine(void *target, float x1, float y1, float x2, float y2, unsigned int color, int type, unsigned int style, int coord_type);
extern FBCALL void fb_GfxEllipse(void *target, float x, float y, float radius, unsigned int color, float aspect, float start, float end, int fill, int coord_type);
extern FBCALL int fb_GfxGet(void *target, float x1, float y1, float x2, float y2, unsigned char *dest, int coord_type, FBARRAY *array);
extern FBCALL int fb_GfxPut(void *target, float x, float y, unsigned char *src, int x1, int y1, int x2, int y2, int coord_type, int mode, int alpha, BLENDER *blender, void *param);
extern FBCALL int fb_GfxWaitVSync(void);
extern FBCALL void fb_GfxPaint(void *target, float fx, float fy, unsigned int color, unsigned int border_color, FBSTRING *pattern, int mode, int coord_type);
extern FBCALL void fb_GfxDraw(void *target, FBSTRING *command);
extern FBCALL int fb_GfxDrawString(void *target, float fx, float fy, int coord_type, FBSTRING *string, unsigned int color, void *font, int mode, BLENDER *blender, void *param);
extern FBCALL void fb_GfxFlip(int from_page, int to_page);
extern FBCALL void fb_GfxSetPage(int work_page, int visible_page);
extern FBCALL void fb_GfxLock(void);
extern FBCALL void fb_GfxUnlock(int start_line, int end_line);
extern FBCALL void *fb_GfxScreenPtr(void);
extern FBCALL void fb_GfxSetWindowTitle(FBSTRING *title);
extern FBCALL int fb_GfxGetJoystick(int id, int *buttons, float *a1, float *a2, float *a3, float *a4, float *a5, float *a6, float *a7, float *a8);

/* Runtime library hooks */
int fb_GfxGetkey(void);
FBSTRING *fb_GfxInkey(void);
int fb_GfxKeyHit(void);
int fb_GfxColor(int fg_color, int bg_color);
void fb_GfxClear(int mode);
int fb_GfxWidth(int w, int h);
int fb_GfxLocateRaw(int y, int x, int cursor);
int fb_GfxLocate(int y, int x, int cursor);
int fb_GfxGetX(void);
int fb_GfxGetY(void);
void fb_GfxGetXY(int *col, int *row);
void fb_GfxGetSize(int *cols, int *rows);
void fb_GfxPrintBuffer(const char *buffer, int mask);
void fb_GfxPrintBufferWstr(const FB_WCHAR *buffer, int mask);
void fb_GfxPrintBufferEx(const void *buffer, size_t len, int mask);
void fb_GfxPrintBufferWstrEx(const FB_WCHAR *buffer, size_t len, int mask);
char *fb_GfxReadStr(char *buffer, int maxlen);
int fb_GfxMultikey(int scancode);
int fb_GfxGetMouse(int *x, int *y, int *z, int *buttons);
int fb_GfxSetMouse(int x, int y, int cursor);
int fb_GfxOut(unsigned short port, unsigned char value);
int fb_GfxIn(unsigned short port);
int fb_GfxLineInput( FBSTRING *text, void *dst, int dst_len, int fillrem, int addquestion, int addnewline );
int fb_GfxLineInputWstr( const FB_WCHAR *text, FB_WCHAR *dst, int max_chars, int addquestion, int addnewline );
unsigned int fb_GfxReadXY( int col, int row, int colorflag );
void fb_GfxSleep( int msecs );
int fb_GfxIsRedir( int is_input );

typedef void (*FBGFX_IMAGE_CONVERT)(const unsigned char *, unsigned char *, int);

void fb_image_convert_8to8(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_8to16(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_8to32(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_24to16(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_24to32(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_32to16(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_32to32(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_24bgrto16(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_24bgrto32(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_32bgrto16(const unsigned char *src, unsigned char *dest, int w);
void fb_image_convert_32bgrto32(const unsigned char *src, unsigned char *dest, int w);

FBCALL void fb_GfxImageConvertRow( const unsigned char *src, int src_bpp, unsigned char *dest, int dst_bpp, int width, int isrgb );

/** Returns TRUE if application is in graphics mode.
 *
 * This implementation is a hack until I found a better way to detect this.
 */
#define FB_GFX_ACTIVE() \
    (fb_hooks.printbuffproc!=NULL)

/** Returns the code page as integral value.
 *
 * This function returns the code page as integral value. When the code page
 * cannot be expressed as an integral value (like UTF-8 or UCS-4), it returns
 * -1 and the character set ID should be used instead.
 */
#define FB_GFX_GET_CODEPAGE() \
    437

/** Returns the character set as a string.
 */
#define FB_GFX_GET_CHARSET() \
    "CP437"

#ifdef __cplusplus
}
#endif

#endif
