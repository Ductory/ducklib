#include "gdiplib.h"

#define LSB_SHL(x,i,j) ((x) << (((i) << 2) + (j)))
#define SET_LSB(b,d,i,j) ((b) = ((b) & ~1 | !!((d) & LSB_SHL(1, i, j))))
#define GET_LSB(b,i,j) LSB_SHL((b) & 1, i, j)

/**
 * lsb encode
 * @param bytes hidden data
 * @param size size of hidden data
 * @return generated image
 */
image_t *lsb(image_t *img, BYTE *bytes, size_t size)
{
	image_t *new = gdip_clone(img);
	UINT w = gdip_getwidth(new), h = gdip_getheight(new);
	argb_t *arr;
	data_t data;
	gdip_lock(new, 0, 0, w, h, &data, &arr);
	for (int i = 0; i < 8; ++i) {
		argb_t p = arr[i];
		arr[i] = ARGB(SET_LSB(p.a, size, i, 0), SET_LSB(p.r, size, i, 1),
					  SET_LSB(p.g, size, i, 2), SET_LSB(p.b, size, i, 3));
	}
	for (size_t i = 0; i < size * 2; ++i) {
		argb_t p = arr[i + 8];
		int l = i & 1;
		BYTE b = bytes[i >> 1];
		arr[i + 8] = ARGB(SET_LSB(p.a, b, l, 0), SET_LSB(p.r, b, l, 1),
						  SET_LSB(p.g, b, l, 2), SET_LSB(p.b, b, l, 3));
	}
	gdip_unlock(new, &data);
	return new;
}

/**
 * lsb decode
 * @param size pointer to store hidden size
 * @return hidden data
 */
BYTE *ilsb(image_t *img, size_t *size)
{
	UINT w = gdip_getwidth(img), h = gdip_getheight(img);
	argb_t *arr;
	data_t data;
	gdip_lock(img, 0, 0, w, h, &data, &arr);
	size_t sz = 0;
	for (int i = 0; i < 8; ++i) {
		argb_t p = arr[i];
		sz |= GET_LSB(p.a, i, 0) | GET_LSB(p.r, i, 1) |
			  GET_LSB(p.g, i, 2) | GET_LSB(p.b, i, 3);
	}
	BYTE *buf = malloc(sz), b = 0;
	for (size_t i = 0; i < sz * 2; ++i) {
		argb_t p = arr[i + 8];
		int l = i & 1;
		b |= GET_LSB(p.a, l, 0) | GET_LSB(p.r, l, 1) |
			 GET_LSB(p.g, l, 2) | GET_LSB(p.b, l, 3);
		if (l)
			buf[i >> 1] = b, b = 0;
	}
	if (size)
		*size = sz;
	gdip_unlock(img, &data);
	return buf;
}
