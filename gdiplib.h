#ifndef _GDIPLIB_H
#define _GDIPLIB_H

#include <windows.h>
#include <wtypes.h>
#include <gdiplus/gdiplus.h>

typedef enum ImageFormat {
	IF_BMP, IF_JPG, IF_GIF, IF_TIFF, IF_PNG
} ImageFormat;

typedef GpImage image_t;
typedef BitmapData data_t;
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
data_t *gdip_lock(image_t *image, LONG x1, LONG y1, LONG x2, LONG y2, void *arr);
void gdip_unlock(image_t *image, data_t *data);
int gdip_getmax(argb_t p);

#endif