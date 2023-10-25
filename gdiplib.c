#define INITGUID
#include "gdiplib.h"

DEFINE_GUID(EncoderQuality, 0x1d5be4b5, 0xfa4a, 0x452d, 0x9c, 0xdd, 0x5d,0xb3,0x51,0x05,0xe7,0xeb);

static ULONG_PTR Token;
static ImageCodecInfo *Encoders;
static UINT EncodersNum;

static CLSID *get_encoder_clsid(WCHAR *mimetype);

void gdip_init(void)
{
	GdiplusStartupInput gsi = {.GdiplusVersion = 1, .DebugEventCallback = NULL,
		.SuppressBackgroundThread = FALSE, .SuppressExternalCodecs = FALSE};
	GdiplusStartup(&Token, &gsi, NULL);
	/* get all avaliable encoders */
	UINT size;
	GetImageEncodersSize(&EncodersNum, &size);
	if (!size) // no avaliable encoders
		return;
	Encoders = malloc(size);
	GetImageEncoders(EncodersNum, size, Encoders);
}

void gdip_uninit(void)
{
	GdiplusShutdown(Token);
	free(Encoders);
}

image_t gdip_loadfile(WCHAR *filename)
{
	image_t image;
	GdipLoadImageFromFile(filename, &image);
	return image;
}

void gdip_save(image_t image, WCHAR *filename, ImageFormat format,
	UINT jpgquality, double resolution)
{
	if (resolution)
		GdipBitmapSetResolution(image, resolution, resolution);
	CLSID *clsid;
	switch (format) {
		case IF_BMP:
			if ((clsid = get_encoder_clsid(L"image/bmp")))
				GdipSaveImageToFile(image, filename, clsid, NULL);
			break;
		case IF_JPG:
			if ((clsid = get_encoder_clsid(L"image/jpeg"))) {
				jpgquality = jpgquality > 100 ? 100 : jpgquality;
				EncoderParameters params = {1, {{EncoderQuality, 1,
					EncoderParameterValueTypeLong, &jpgquality}}};
				GdipSaveImageToFile(image, filename, clsid, &params);
			}
			break;
		case IF_GIF:
			if ((clsid = get_encoder_clsid(L"image/gif")))
				GdipSaveImageToFile(image, filename, clsid, NULL);
			break;
		case IF_TIFF:
			if ((clsid = get_encoder_clsid(L"image/tiff")))
				GdipSaveImageToFile(image, filename, clsid, NULL);
			break;
		case IF_PNG:
			if ((clsid = get_encoder_clsid(L"image/png")))
				GdipSaveImageToFile(image, filename, clsid, NULL);
			break;
	}
}

image_t gdip_create(int width, int height)
{
	image_t img;
	int stride = 4 * width;
	BYTE *scan0 = calloc(1, stride * height);
	GdipCreateBitmapFromScan0(width, height, stride, PixelFormat32bppARGB, scan0,
		&img);
	return img;
}

image_t gdip_clone(image_t img)
{
	image_t new;
	GdipCloneImage(img, &new);
	return new;
}

void gdip_dispose(image_t image)
{
	GdipDisposeImage(image);
}

UINT gdip_getwidth(image_t image)
{
	UINT width;
	GdipGetImageWidth(image, &width);
	return width;
}

UINT gdip_getheight(image_t image)
{
	UINT height;
	GdipGetImageHeight(image, &height);
	return height;
}

data_t gdip_lock(image_t image, LONG x1, LONG y1, LONG x2, LONG y2, void *arr)
{
	GpRect rc = {x1, y1, x2, y2};
	data_t data = malloc(sizeof(BitmapData));
	GdipBitmapLockBits(image, &rc, ImageLockModeRead | ImageLockModeWrite,
		PixelFormat32bppARGB, data);
	*(void**)arr = data->Scan0;
	return data;
}

void gdip_unlock(image_t image, data_t data)
{
	GdipBitmapUnlockBits(image, data);
	free(data);
}

BYTE gdip_max(argb_t p)
{
	return p.r > p.g ? p.r > p.b ? p.r : p.b
					 : p.g > p.b ? p.g : p.b;
}

static CLSID *get_encoder_clsid(WCHAR *mimetype)
{
	for (UINT i = 0; i < EncodersNum; ++i)
		if (!wcscmp(mimetype, Encoders[i].MimeType))
			return &Encoders[i].Clsid;
	return NULL;
}