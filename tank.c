/**
 * 幻影坦克 - Dangfer
 * 
 * note:
 * 算法细节请见博客 https://ductory.github.io/2023/10/25/%E6%B5%85%E8%B0%88%E4%B8%80%E4%BA%9B%E6%9C%89%E8%B6%A3%E7%9A%84%E5%9B%BE%E5%83%8F%E5%A4%84%E7%90%86%E7%AE%97%E6%B3%95/
 * S: 表图 (Surface)
 * H: 里图 (Hidden)
 */
#include "tank.h"

/**
 * 灰度幻坦
 * @param  img_s  
 * @param  img_h  
 * @param  fact_s 表图因子
 * @param  fact_h 里图因子
 * @return        
 */
image_t *tank_gray_sh(image_t *img_s, image_t *img_h, double fact_s, double fact_h)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t *img_m = gdip_create(w, h);
	argb_t (*arr_s)[w], (*arr_h)[w], (*arr_m)[w];
	data_t data_s, data_h, data_m;
	gdip_lock(img_s, 0, 0, w, h, &data_s, &arr_s);
	gdip_lock(img_h, 0, 0, w, h, &data_h, &arr_h);
	gdip_lock(img_m, 0, 0, w, h, &data_m, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t ps = arr_s[i][j], ph = arr_h[i][j];
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
	gdip_unlock(img_s, &data_s);
	gdip_unlock(img_h, &data_h);
	gdip_unlock(img_m, &data_m);
	return img_m;
}

/**
 * 无表灰度
 * @param  img_h 
 * @return       
 */
image_t *tank_gray_h(image_t *img_h)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t *img_m = gdip_create(w, h);
	argb_t (*arr_h)[w], (*arr_m)[w];
	data_t data_h, data_m;
	gdip_lock(img_h, 0, 0, w, h, &data_h, &arr_h);
	gdip_lock(img_m, 0, 0, w, h, &data_m, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t p = arr_h[i][j];
			int a = GRAYFY(p.r, p.g, p.b);
			arr_m[i][j] = ARGB(a, 255, 255, 255);
		}
	gdip_unlock(img_h, &data_h);
	gdip_unlock(img_m, &data_m);
	return img_m;
}

/**
 * 全彩幻坦
 * @param  img_s 
 * @param  img_h 
 * @param  algo  0:RGB算法 1:Luv算法(recommend)
 * @return       
 */
image_t *tank_rgb_sh(image_t *img_s, image_t *img_h, int algo)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t *img_m = gdip_create(w, h);
	argb_t (*arr_s)[w], (*arr_h)[w], (*arr_m)[w];
	data_t data_s, data_h, data_m;
	gdip_lock(img_s, 0, 0, w, h, &data_s, &arr_s);
	gdip_lock(img_h, 0, 0, w, h, &data_h, &arr_h);
	gdip_lock(img_m, 0, 0, w, h, &data_m, &arr_m);
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
	gdip_unlock(img_s, &data_s);
	gdip_unlock(img_h, &data_h);
	gdip_unlock(img_m, &data_m);
	return img_m;
}

/**
 * 无表全彩
 * @param  img_h 
 * @return       
 */
image_t *tank_rgb_h(image_t *img_h)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t *img_m = gdip_create(w, h);
	argb_t (*arr_h)[w], (*arr_m)[w];
	data_t data_h, data_m;
	gdip_lock(img_h, 0, 0, w, h, &data_h, &arr_h);
	gdip_lock(img_m, 0, 0, w, h, &data_m, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t p = arr_h[i][j];
			int a = gdip_getmax(p);
			int r = 0, g = 0, b = 0;
			if (a)
				r = p.r * 255 / a, g = p.g * 255 / a, b = p.b * 255 / a;
			arr_m[i][j] = ARGB(a, r, g, b);
		}
	gdip_unlock(img_h, &data_h);
	gdip_unlock(img_m, &data_m);
	return img_m;
}

/**
 * 精确解
 * @param  img_s 
 * @param  img_h 
 * @return       
 */
image_t *tank_color(image_t *img_s, image_t *img_h)
{
	const double fr = 0.299, fg = 0.587, fb = 0.114;
	const double W = 0.5;
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t *img_m = gdip_create(w, h);
	argb_t (*arr_s)[w], (*arr_h)[w], (*arr_m)[w];
	data_t data_s, data_h, data_m;
	gdip_lock(img_s, 0, 0, w, h, &data_s, &arr_s);
	gdip_lock(img_h, 0, 0, w, h, &data_h, &arr_h);
	gdip_lock(img_m, 0, 0, w, h, &data_m, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t ps =arr_s[i][j], ph = arr_h[i][j];
			double sr = ps.r, sg = ps.g, sb = ps.b;
			double hr = ph.r, hg = ph.g, hb = ph.b;
			double ar = 255 - sr + hr, ag = 255 - sg + hg, ab = 255 - sb + hb;
			double a;
			a = (fr * ar + fg * ag + fb * ab) / (fr + fg + fb);
			int r = 0, g = 0, b = 0;
			if (a) {
				r = ((hr - W * ar) / a + W) * 255;
				g = ((hg - W * ag) / a + W) * 255;
				b = ((hb - W * ab) / a + W) * 255;
				r = r < 0 ? 0 : r;
				g = g < 0 ? 0 : g;
				b = b < 0 ? 0 : b;
				r = r > 255 ? 255 : r;
				g = g > 255 ? 255 : g;
				b = b > 255 ? 255 : b;
			}
			a = a > 255 ? 255 : a;
			arr_m[i][j] = ARGB(a, r, g, b);
		}
	gdip_unlock(img_s, &data_s);
	gdip_unlock(img_h, &data_h);
	gdip_unlock(img_m, &data_m);
	return img_m;
}

/**
 * PS方法
 * @param  img_s 
 * @param  img_h 
 * @return       
 */
image_t *tank_ps(image_t *img_s, image_t *img_h)
{
	UINT w = gdip_getwidth(img_h), h = gdip_getheight(img_h);
	image_t *img_m = gdip_create(w, h);
	argb_t (*arr_s)[w], (*arr_h)[w], (*arr_m)[w];
	data_t data_s, data_h, data_m;
	gdip_lock(img_s, 0, 0, w, h, &data_s, &arr_s);
	gdip_lock(img_h, 0, 0, w, h, &data_h, &arr_h);
	gdip_lock(img_m, 0, 0, w, h, &data_m, &arr_m);
	for (UINT i = 0; i < h; ++i)
		for (UINT j = 0; j < w; ++j) {
			argb_t ps =arr_s[i][j], ph = arr_h[i][j];
			int sr = ps.r, sg = ps.g, sb = ps.b;
			int hr = ph.r, hg = ph.g, hb = ph.b;
	// 色阶调整
	// ignore

	// 表图反相
			int sr_r = 255 - sr, sg_r = 255 - sg, sb_r = 255 - sb;
	// 表图 + 线性减淡(添加)
	// 盖印 M1
			int m1r = sr_r + hr, m1g = sg_r + hg, m1b = sb_r + hb;
			m1r = m1r > 255 ? 255 : m1r;
			m1g = m1g > 255 ? 255 : m1g;
			m1b = m1b > 255 ? 255 : m1b;
	// 隐藏表图
	// M1 + 划分
	// 盖印 M2
			int m2r = !m1r ? hr ? 255 : 0 : 255 * hr / m1r;
			int m2g = !m1g ? hg ? 255 : 0 : 255 * hg / m1g;
			int m2b = !m1b ? hb ? 255 : 0 : 255 * hb / m1b;
			m2r = m2r > 255 ? 255 : m2r;
			m2g = m2g > 255 ? 255 : m2g;
			m2b = m2b > 255 ? 255 : m2b;
	// 只显示 M1
	// M1 + 正常
	// RGB通道
			int a = GRAYFY(m1r, m1g, m1b);
			a = a > 255 ? 255 : a;
	// 只显示 M2
	// 蒙版
			arr_m[i][j] = ARGB(a, m2r, m2g, m2b);
		}
	gdip_unlock(img_s, &data_s);
	gdip_unlock(img_h, &data_h);
	gdip_unlock(img_m, &data_m);
	return img_m;
}
