#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "buffer.h"

/* allocation */
#define alloc(obj,n)  malloc(sizeof(*(obj)) * (n))
#define allocz(obj,n) calloc((n), sizeof(*(obj)))
#define allocr(obj,n) realloc((obj), sizeof(*(obj)) * (n))

buf_t *new_buf(size_t n)
{
	size_t sz = 256;
	while (sz < n)
		sz <<= 1;
	buf_t *buf = alloc(buf, 1);
	buf->size = sz;
	buf->cnt  = 0;
	buf->buf  = malloc(sz);
	return buf;
}

void free_buf(buf_t *buf)
{
	free(buf->buf);
	free(buf);
}

void buf_cls(buf_t *buf)
{
	buf->cnt = 0;
}

void buf_ccat(buf_t *buf, int ch)
{
	if (buf->cnt == buf->size)
		buf->buf = realloc(buf->buf, buf->size <<= 1);
	buf->buf[buf->cnt++] = ch;
}

void buf_wccat(buf_t *buf, int ch)
{
	if (buf->cnt >= buf->size - 1)
		buf->buf = realloc(buf->buf, buf->size <<= 1);
	*(wchar_t*)(buf->buf + buf->cnt) = ch;
	buf->cnt += 2;
}

/**
 * \0 will not be added
 */
void buf_cat(buf_t *buf, const char *s)
{
	size_t len = strlen(s);
	while (buf->cnt + len > buf->size)
		buf->buf = allocr(buf->buf, buf->size <<= 1);
	memcpy(buf->buf + buf->cnt, s, len);
	buf->cnt += len;
}

void buf_wcat(buf_t *buf, const wchar_t *s)
{
	size_t len = wcslen(s) * 2;
	while (buf->cnt + len > buf->size)
		buf->buf = allocr(buf->buf, buf->size <<= 1);
	memcpy(buf->buf + buf->cnt, s, len);
	buf->cnt += len;
}

/* max buffer length: 2048 */
void buf_catf(buf_t *buf, const char *fmt, ...)
{
	char b[2048];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(b, fmt, ap);
	buf_cat(buf, b);
}

void buf_wcatf(buf_t *buf, const wchar_t *fmt, ...)
{
	wchar_t wb[2048];
	va_list ap;
	va_start(ap, fmt);
	vswprintf(wb, 2048, fmt, ap);
	buf_wcat(buf, wb);
}