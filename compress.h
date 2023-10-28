#ifndef _COMPRESS_H_
#define _COMPRESS_H_

#include <stdint.h>

int32_t RLComp(uint8_t *src, uint8_t *dest, int32_t src_size);
int32_t RLUnComp(uint8_t *src, uint8_t *dest);
int32_t LZ77Comp(uint8_t *src, uint8_t *dest, int32_t src_size, uint8_t lazy);
int32_t LZ77UnComp(uint8_t *src, uint8_t *dest);
int32_t HuffComp(uint8_t *src, uint8_t *dest, int32_t src_size, uint8_t bmode);
int32_t HuffUnComp(uint8_t *src, uint32_t *dest);

#endif