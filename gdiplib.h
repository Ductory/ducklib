#ifndef _GDIPLIB_H
#define _GDIPLIB_H

#include <windows.h>
#include <wtypes.h>
#include <gdiplus/gdiplus.h>

typedef enum ImageFormat {
	IF_BMP, IF_JPG, IF_GIF, IF_TIFF, IF_PNG
} ImageFormat;

typedef GpImage image_t;
typedef GpBitmap bitmap_t;
typedef GpGraphics graphics_t;
typedef BitmapData data_t;
typedef ColorPalette palette_t;
typedef struct {BYTE b, g, r, a;} argb_t;

#define GRAYFY(r,g,b) (0.299 * (r) + 0.587 * (g) + 0.114 * (b))
#define ARGB(a,r,g,b) (argb_t){b, g, r, a}
#define CLAMP(a,b,c) ((a) < (b) ? (b) : (a) < (c) ? (a) : (c))

void gdip_init(void);
void gdip_uninit(void);
image_t *gdip_loadfile(const WCHAR *filename);
void gdip_save(image_t *img, WCHAR *name, ImageFormat format, UINT jpgquality, double resolution);
image_t *gdip_create(int width, int height);
image_t *gdip_clone(image_t *img);
void gdip_dispose(image_t *image);
UINT gdip_getwidth(image_t *image);
UINT gdip_getheight(image_t *image);
int gdip_lock(image_t *img, LONG x, LONG y, LONG w, LONG h, data_t *data, void *arr);
void gdip_unlock(image_t *image, data_t *data);
bitmap_t *gdip_create_bitmap_from_arr(void *arr, int width, int height);
image_t *gdip_double_size(image_t *img);
void gdip_fill_rect(image_t *img, argb_t color, int x, int y, int w, int h);
palette_t *gdip_get_palette(image_t *img);
int gdip_getmax(argb_t p);

#endif