#ifndef _TANK_H_
#define _TANK_H_

#include "gdiplib.h"

image_t *tank_gray_sh(image_t *img_s, image_t *img_h, double fact_s, double fact_h);
image_t *tank_gray_h(image_t *img_h);
image_t *tank_rgb_sh(image_t *img_s, image_t *img_h, int algo);
image_t *tank_rgb_h(image_t *img_h);

#endif