#include "gdiplib.h"

image_t *_tank_rgb_sh(image_t *img_s, image_t *img_h, UINT x, UINT y)
{
	UINT w = gdip_getwidth(img_s), h = gdip_getheight(img_s);
	UINT w2 = gdip_getwidth(img_h), h2 = gdip_getheight(img_h);
	image_t *img_m = gdip_create(w, h);
	argb_t (*arr_s)[w], (*arr_h)[w2], (*arr_m)[w];
	data_t data_s, data_h, data_m;
	gdip_lock(img_s, 0, 0, w,  h,  &data_s, &arr_s);
	gdip_lock(img_h, 0, 0, w2, h2, &data_h, &arr_h);
	gdip_lock(img_m, 0, 0, w,  h,  &data_m, &arr_m);
	for (UINT i = 0; i < y; ++i)
		for (UINT j = 0; j < w; ++j)
			arr_m[i][j] = arr_s[i][j];
	for (UINT i = y; i < y + h2; ++i) {
		for (UINT j = 0; j < x; ++j)
			arr_m[i][j] = arr_s[i][j];
		for (UINT j = x; j < x + w2; ++j) {
			// Luv 空间, 我们没有推导的情况.
			argb_t ps =arr_s[i][j], ph = arr_h[i - y][j - x];
			double sr = ps.r, sg = ps.g, sb = ps.b;
			double hr = ph.r, hg = ph.g, hb = ph.b;
			double ar = 1 - sr/256 + hr/256, ag = 1 - sg/256 + hg/256, ab = 1 - sb/256 + hb/256;
			double mr = hr/256 / ar, mg = hg/256 / ag, mb = hb/256 / ab;
			double fr = (mr - 1) * (mr - 1) + mr * mr,
				   fg = (mg - 1) * (mg - 1) + mg * mg,
				   fb = (mb - 1) * (mb - 1) + mb * mb;
			double A = ((mr-1)*((mr-1)*(mr-1)-(mb-1)*(mb-1))+mr*(mr*mr-mb*mb))/512
					 + (mr*(fr+fb)-((mr-1)*(mr-1)+(mb-1)*(mb-1)))/256;
			double B = (((mr-1)*(mr-1)*(mr-1)+mr*mr*mr)*ar-((mr-1)*(mb-1)*(mb-1)+mr*mb*mb)*ab)/-256
					 + (mr*fb-(mb-1)*(mb-1))*(ar-ab)/256 + ((mr-1)*(mr-1)+(mb-1)*(mb-1))/128
					 + 4*fr + 8*fg + (5+127./128)*fb;
			double C = (((mr-1)*(mr-1)*(mr-1)+mr*mr*mr)*ar*ar-((mr-1)*(mb-1)*(mb-1)+mr*mb*mb)*ab*ab)/512
					 + (mr*(fr*ar*ar-fb*ar*ab)-((mr-1)*(mr-1)*ar*ar-(mb-1)*(mb-1)*ar*ab))/256 - ((mr-1)*(mr-1)*ar+(mb-1)*(mb-1)*ab)/128
					 - 4*fr*ar - 8*fg*ag - (5+127./128)*fb*ab;
			double a = A ? (-B + sqrt(B*B-4*A*C)) / (2*A) * 256 : 10000;
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
		for (UINT j = x + w2; j < w; ++j)
			arr_m[i][j] = arr_s[i][j];
	}
	for (UINT i = y + h2; i < h; ++i)
		for (UINT j = 0; j < w; ++j)
			arr_m[i][j] = arr_s[i][j];
	gdip_unlock(img_s, &data_s);
	gdip_unlock(img_h, &data_h);
	gdip_unlock(img_m, &data_m);
	return img_m;
}

int main(void)
{
	gdip_init();
	image_t *img_s = gdip_loadfile(L"s.jpg"); // 表图
	image_t *img_h = gdip_loadfile(L"h.jpg"); // 里图
	image_t *img_m = _tank_rgb_sh(img_s, img_h, 0, 0);
	gdip_save(img_m, L"new.png", IF_PNG, 0, 0);
	gdip_dispose(img_s);
	gdip_dispose(img_h);
	gdip_dispose(img_m);
	gdip_uninit();
	return 0;
}