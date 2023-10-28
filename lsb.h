#ifndef _LSB_H_
#define _LSB_H_

#include "gdiplib.h"

image_t lsb(image_t img, BYTE *bytes, size_t size);
BYTE *ilsb(image_t img, size_t *size);

#endif