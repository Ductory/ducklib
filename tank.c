#include "gdiplib.h"

/* untested */
image_t tank_gray_sh(image_t img_s, image_t img_h, double fact_s, double fact_h)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t img_m = gdip_create(w, h);
	argb_t (*arr_s)[w], (*arr_h)[w], (*arr_m)[w];
	data_t data_s = gdip_lock(img_s, 0, 0, w - 1, h - 1, &arr_s);
	data_t data_h = gdip_lock(img_h, 0, 0, w - 1, h - 1, &arr_h);
	data_t data_m = gdip_lock(img_m, 0, 0, w - 1, h - 1, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t ps =arr_s[i][j], ph = arr_h[i][j];
			double rs = ps.r, gs = ps.g, bs = ps.b;
			double rh = ph.r, gh = ph.g, bh = ph.b;
			BYTE sg = GRAYFY(rs, gs, bs);
			BYTE hg = GRAYFY(rh, gh, bh);
			BYTE a = 255 - sg + hg;
			a = a > 255 ? 255 : a;
			arr_m[i][j] = a ? ARGB(a, rh / a, gh / a, bh / a)
							: ARGB(0, 0, 0, 0);
		}
	gdip_unlock(img_s, data_s);
	gdip_unlock(img_h, data_h);
	gdip_unlock(img_m, data_m);
	return img_m;
}

image_t tank_gray_h(image_t img_h)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t img_m = gdip_create(w, h);
	argb_t (*arr_h)[w], (*arr_m)[w];
	data_t data_h = gdip_lock(img_h, 0, 0, w - 1, h - 1, &arr_h);
	data_t data_m = gdip_lock(img_m, 0, 0, w - 1, h - 1, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t p = arr_h[i][j];
			double r = p.r, g = p.g, b = p.b;
			BYTE a = GRAYFY(r, g, b);
			arr_m[i][j] = ARGB(a, 255, 255, 255);
		}
	gdip_unlock(img_h, data_h);
	gdip_unlock(img_m, data_m);
	return img_m;
}

image_t tank_rgb_h(image_t img_h)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t img_m = gdip_create(w, h);
	argb_t (*arr_h)[w], (*arr_m)[w];
	data_t data_h = gdip_lock(img_h, 0, 0, w - 1, h - 1, &arr_h);
	data_t data_m = gdip_lock(img_m, 0, 0, w - 1, h - 1, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t p = arr_h[i][j];
			BYTE a = gdip_max(p);
			arr_m[i][j] = a ? ARGB(a, p.a / a, p.g / a, p.b / a)
							: ARGB(0, 0, 0, 0);
		}
	gdip_unlock(img_h, data_h);
	gdip_unlock(img_m, data_m);
	return img_m;
}