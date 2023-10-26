#include "tank.h"

int main(void)
{
	gdip_init();
	image_t img_s = gdip_loadfile(L"a.png"); // 表图
	image_t img_h = gdip_loadfile(L"b.png"); // 里图
	image_t img_m;
	// img_m = tank_gray_sh(img_s, img_h, 1.2, 0.7);
	// img_m = tank_gray_h(img_h);
	// img_m = tan_rgb_sh(img_s, img_h, 1);
	// img_m = tan_rgb_h(img_h);
	gdip_save(img_m, L"gen.png", IF_PNG, 0, 0);
	gdip_dispose(img_s);
	gdip_dispose(img_h);
	gdip_dispose(img_m);
	gdip_uninit();
	return 0;
}