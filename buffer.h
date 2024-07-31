#ifndef _BUFFER_H
#define _BUFFER_H

#include <wchar.h>

typedef struct buf_t {
	size_t size;
	size_t cnt;
	unsigned char *buf;
} buf_t;

buf_t *new_buf(size_t n);
void free_buf(buf_t *buf);
void buf_cls(buf_t *buf);
void buf_ccat(buf_t *buf, int ch);
void buf_wccat(buf_t *buf, int ch);
void buf_cat(buf_t *buf, const char *s);
void buf_wcat(buf_t *buf, const wchar_t *s);
void buf_catf(buf_t *buf, const char *fmt, ...);
void buf_wcatf(buf_t *buf, const wchar_t *fmt, ...);

#endif