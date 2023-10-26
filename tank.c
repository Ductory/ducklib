#include "tank.h"

/**
 * 灰度幻影坦克
 * @param img_s 表图
 * @param img_h 里图
 * @param fact_s 表图因子
 * @param fact_h 里图因子
 * @return 混合图
 */
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
			double sr = ps.r * fact_s, sg = ps.g * fact_s, sb = ps.b * fact_s;
			double hr = ph.r * fact_h, hg = ph.g * fact_h, hb = ph.b * fact_h;
			double gs = GRAYFY(sr, sg, sb), gh = GRAYFY(hr, hg, hb);
			double a = 255 - gs + gh;
			a = a < 0 ? 0 : a;
			int g;
			if (a) {
				g = gh * 255 / a;
				g = g > 255 ? 255 : g;
			}
			a = a > 255 ? 255 : a;
			arr_m[i][j] = ARGB(a, g, g, g);
		}
	gdip_unlock(img_s, data_s);
	gdip_unlock(img_h, data_h);
	gdip_unlock(img_m, data_m);
	return img_m;
}

/**
 * 无表图的灰度幻影坦克
 * @param img_h 里图
 * @return 混合图
 */
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
			int a = GRAYFY(p.r, p.g, p.b);
			arr_m[i][j] = ARGB(a, 255, 255, 255);
		}
	gdip_unlock(img_h, data_h);
	gdip_unlock(img_m, data_m);
	return img_m;
}

/**
 * 全彩幻影坦克
 * @param img_s 表图
 * @param img_h 里图
 * @param algo 0:RGB算法 1:Lab算法(recommend)
 * @return 混合图
 */
image_t tank_rgb_sh(image_t img_s, image_t img_h, int algo)
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
			double sr = ps.r, sg = ps.g, sb = ps.b;
			double hr = ph.r, hg = ph.g, hb = ph.b;
			double a;
			if (algo) {
				double dr = hr - sr, dg = hg - sg, db = hb - sb;
				double A = (dr - db) / 128 + 17 + 127. / 128;
				double B = (dr * dr - db * db) / 512 + (hr + sr) * (dr - db) / 256 + 4 * dr + 8 * dg + (5 + 127. / 128) * db;
				a = 255 + B / A;
			} else {
				double ar = 255 - sr + hr, ag = 255 - sg + hg, ab = 255 - sb + hb;
				double xr = hr * hr + (hr - ar) * (hr - ar),
					   xg = hg * hg + (hg - ag) * (hg - ag),
					   xb = hb * hb + (hb - ab) * (hb - ab);
				a = (xr * ar + xg * ag + xb * ab) / (xr + xg + xb);
			}
			a = a < 0 ? 0 : a;
			int r = 0, g = 0, b = 0;
			if (a) {
				r = hr * 255 / a, g = hg * 255 / a, b = hb * 255 / a;
				r = r > 255 ? 255 : r;
				g = g > 255 ? 255 : g;
				b = b > 255 ? 255 : b;
			}
			a = a > 255 ? 255 : a;
			arr_m[i][j] = ARGB(a, r, g, b);
		}
	gdip_unlock(img_s, data_s);
	gdip_unlock(img_h, data_h);
	gdip_unlock(img_m, data_m);
	return img_m;
}

/**
 * 无表图的全彩幻影坦克
 * @param img_h 里图
 * @return 混合图
 */
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
			int a = gdip_max(p);
			int r = 0, g = 0, b = 0;
			if (a)
				r = p.r * 255 / a, g = p.g * 255 / a, b = p.b * 255 / a;
			arr_m[i][j] = ARGB(a, r, g, b);
		}
	gdip_unlock(img_h, data_h);
	gdip_unlock(img_m, data_m);
	return img_m;
}