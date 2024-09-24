/**
 * gdiplib - Dangfer
 * A Gdiplus lib in C.
 * 
 * note:
 * In gdiplib, we always use ARGB32.
 */
#define INITGUID

#include "gdiplib.h"

#define PIXEL_BYTES 4

DEFINE_GUID(EncoderQuality, 0x1d5be4b5, 0xfa4a, 0x452d, 0x9c, 0xdd, 0x5d,0xb3,0x51,0x05,0xe7,0xeb);

static CLSID *get_encoder_clsid(WCHAR *mimetype);

static ULONG_PTR _token;
static ImageCodecInfo *_encoders;
static UINT _encoders_num;

/**
 * @gdip_init
 * Initialize gdiplib.
 */
void gdip_init(void)
{
	if (_token)
		return;
	GdiplusStartupInput gsi = {.GdiplusVersion = 1, .DebugEventCallback = NULL,
		.SuppressBackgroundThread = FALSE, .SuppressExternalCodecs = FALSE};
	GdiplusStartup(&_token, &gsi, NULL);

	/* get all avaliable encoders */
	UINT size;
	GetImageEncodersSize(&_encoders_num, &size);
	if (!size) // no avaliable encoders
		return;
	_encoders = malloc(size);
	GetImageEncoders(_encoders_num, size, _encoders);
}

/**
 * @gdip_uninit
 * Uninitialize gdiplib.
 */
void gdip_uninit(void)
{
	GdiplusShutdown(_token);
}

/**
 * @gdip_loadfile
 * Load image from file.
 * 
 * name: the file name of the image to be loaded
 * 
 * return: handler of the loaded image
 */
image_t *gdip_loadfile(const WCHAR *name)
{
	image_t *image;
	GdipLoadImageFromFile(name, &image);
	return image;
}

/**
 * @gdip_save
 * Save the image to file.
 * 
 * img:        handler of the image
 * name:       file name
 * format:     image file format
 * jpgquality: jpeg quality. avaliable only when format == IF_JPG
 * resolution: resolution of the image
 */
void gdip_save(image_t *img, WCHAR *name, ImageFormat format, UINT jpgquality, double resolution)
{
	if (resolution)
		GdipBitmapSetResolution(img, resolution, resolution);
	CLSID *clsid;
	switch (format) {
		case IF_BMP:
			if ((clsid = get_encoder_clsid(L"image/bmp")))
				GdipSaveImageToFile(img, name, clsid, NULL);
			break;
		case IF_JPG:
			if ((clsid = get_encoder_clsid(L"image/jpeg"))) {
				jpgquality = jpgquality > 100 ? 100 : jpgquality;
				EncoderParameters params = {1, {{EncoderQuality, 1, EncoderParameterValueTypeLong, &jpgquality}}};
				GdipSaveImageToFile(img, name, clsid, &params);
			}
			break;
		case IF_GIF:
			if ((clsid = get_encoder_clsid(L"image/gif")))
				GdipSaveImageToFile(img, name, clsid, NULL);
			break;
		case IF_TIFF:
			if ((clsid = get_encoder_clsid(L"image/tiff")))
				GdipSaveImageToFile(img, name, clsid, NULL);
			break;
		case IF_PNG:
			if ((clsid = get_encoder_clsid(L"image/png")))
				GdipSaveImageToFile(img, name, clsid, NULL);
			break;
	}
}

/**
 * @get_encoder_clsid
 * Get encoder's CLSID by mimetype.
 * 
 * mimetype: image type
 * 
 * return: encoder's CLSID
 */
static CLSID *get_encoder_clsid(WCHAR *mimetype)
{
	for (UINT i = 0; i < _encoders_num; ++i)
		if (!wcscmp(mimetype, _encoders[i].MimeType))
			return &_encoders[i].Clsid;
	return NULL;
}

/**
 * @gdip_create
 * Create a new empty image.
 * 
 * width:  the width of the image
 * height: the height of the image
 * 
 * return: handler of the new image
 */
image_t *gdip_create(int width, int height)
{
	image_t *img;
	int stride = PIXEL_BYTES * width;
	BYTE *scan0 = calloc(stride, height);
	GdipCreateBitmapFromScan0(width, height, stride, PixelFormat32bppARGB, scan0, &img);
	return img;
}

/**
 * @gdip_clone
 * Create an image copy.
 * 
 * img: handler of the source image
 * 
 * return: handler of the image copy.
 */
image_t *gdip_clone(image_t *img)
{
	image_t *new_img;
	GdipCloneImage(img, &new_img);
	return new_img;
}

/**
 * @gdip_dispose
 * Dispose an image.
 * 
 * img: handler of the image to be disposed
 */
void gdip_dispose(image_t *img)
{
	GdipDisposeImage(img);
}

/**
 * @gdip_getwidth
 * Get the width of an image.
 * 
 * img: handler of the image
 * 
 * return: the width of the image
 */
UINT gdip_getwidth(image_t *img)
{
	UINT width;
	GdipGetImageWidth(img, &width);
	return width;
}

/**
 * @gdip_getheight
 * Get the height of an image.
 * 
 * img: handler of the image
 * 
 * return: the height of the image
 */
UINT gdip_getheight(image_t *img)
{
	UINT height;
	GdipGetImageHeight(img, &height);
	return height;
}

/**
 * lock the image to accelerate
 * @param  img
 * @param  x    left
 * @param  y    top
 * @param  w    width
 * @param  h    height
 * @param  data
 * @param  arr  (optional) the address of pointer to be set to Scan0
 * @return      non-zero if success, zero if fail
 */
int gdip_lock(image_t *img, LONG x, LONG y, LONG w, LONG h, data_t *data, void *arr)
{
	if (!img || !data)
		return 0;
	GdipBitmapLockBits(img, &(GpRect){x, y, w, h}, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, data);
	if (arr)
		*(void**)arr = data->Scan0;
	return 1;
}

/**
 * unlock the image
 * @param img  
 * @param data 
 */
void gdip_unlock(image_t *img, data_t *data)
{
	GdipBitmapUnlockBits(img, data);
}

bitmap_t *gdip_create_bitmap_from_arr(void *arr, int width, int height)
{
	bitmap_t *bmp;
	GdipCreateBitmapFromScan0(width, height, sizeof(argb_t) * width, PixelFormat32bppARGB, arr, &bmp);
	return bmp;
}

image_t *gdip_double_size(image_t *img)
{
	int w = gdip_getwidth(img), h = gdip_getheight(img);
	image_t *img_big = gdip_create(w * 2, h * 2);
	argb_t (*img_arr)[w];
	data_t data, data_big;
	gdip_lock(img, 0, 0, w, h, &data, &img_arr);
	argb_t (*img_big_arr)[w * 2];
	gdip_lock(img, 0, 0, w * 2, h * 2, &data_big, &img_big_arr);
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			argb_t pix = img_arr[j][i];
			img_big_arr[j * 2][i * 2] = pix;
			img_big_arr[j * 2 + 1][i * 2] = pix;
			img_big_arr[j * 2][i * 2 + 1] = pix;
			img_big_arr[j * 2 + 1][i * 2 + 1] = pix;
		}
	}
	gdip_unlock(img, &data);
	gdip_unlock(img_big, &data_big);
	return img_big;
}

void gdip_fill_rect(image_t *img, argb_t color, int x, int y, int w, int h)
{
	graphics_t *graphics;
	GdipGetImageGraphicsContext(img, &graphics);
	GpSolidFill *sf;
	GdipCreateSolidFill(*(ARGB*)&color, &sf);
	GdipFillRectangleI(graphics, sf, x, y, w, h);
	GdipDeleteBrush(sf);
	GdipDeleteGraphics(graphics);
}

palette_t *gdip_get_palette(image_t *img)
{
	int size;
	if (GdipGetImagePaletteSize(img, &size) != Ok || !size)
		return NULL;
	palette_t *cp = malloc(sizeof(palette_t) + sizeof(ARGB) * (size - 1));
	if (GdipGetImagePalette(img, cp, size) != Ok)
		free(cp), cp = NULL;
	return cp;
}

/**
 * calculate max color component in RGB
 * @param  p pixel
 * @return   max component
 */
int gdip_getmax(argb_t p)
{
	return p.r > p.g ? p.r > p.b ? p.r : p.b
					 : p.g > p.b ? p.g : p.b;
}
