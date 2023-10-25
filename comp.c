#include <stdint.h>

typedef uint32_t u32;
typedef int32_t s32;
typedef int16_t s16;
typedef uint16_t u16;
typedef uint8_t u8;

#define IS_IN_RANGE(p) (p < src + src_size)
#define CHECK_IN_RANGE(p) if (!IS_IN_RANGE(p)) break


s32 RLComp(u8 *src, u8 *dest, s32 src_size)
{
	u8 *p = src, *dest_org = dest;
	*(u32*)dest = (u32)src_size << 8 | 0x30;
	dest += 4;
	while (p < src + src_size) {
		u8 b = *p++, f = 1; // f: 如果写重复数据则为1
		s32 cnt = 1, dup = 1; // cnt: 读取的长度, dup: 重复的长度
		while (IS_IN_RANGE(p)) {
			++cnt;
			if (*p++ == b) {
				++dup;
				if (!f && dup == 3) {
					p -= 3, cnt -= 3;
					break;
				}
			} else { // neq
				dup = 1;
				if (f) {
					if (dup >= 3)
						break;
					f = 0;
				}
			}
			if (dup == 130)
				break;
			if (!f && cnt == 128) { // try twice at most
				CHECK_IN_RANGE(p);
				if (*p == b) {
					if (dup == 2) {
						p -= 2, cnt -= 2;
						break;
					}
					CHECK_IN_RANGE(p);
					if (p[1] == b)
						--p, --cnt;
				}
				break;
			}
		}
		if (f) {
			if (dup >= 3) {
				*dest++ = 0x80 | (dup - 3);
				*dest++ = b;
			} else { // only if reach the end
				f = 0;
			}
		}
		if (!f) {
			*dest++ = cnt - 1;
			p -= cnt;
			for (s32 i = 0; i < cnt; ++i)
				*dest++ = *p++;
		}
	}
	return dest - dest_org;
}

s32 RLUnComp(u8 *src, u8 *dest)
{
	s32 size = *(u32*)src >> 8, size_org = size;
	src += 4;
	while (size > 0) {
		u8 b = *src++;
		s32 len = b & 0x7F;
		if (!(b & 0x80)) { // 非重复的连续字节
			size -= len += 1;
			while (len--)
				*dest++ = *src++;
		} else { // 重复的连续字节
			size -= len += 3;
			b = *src++;
			while (len--)
				*dest++ = b;
		}
	}
	return size_org;
}


s32 LZ77Comp(u8 *src, u8 *dest, s32 src_size, u8 lazy)
{
	u8 *p = src, *dest_org = dest;
	*(u32*)dest = (u32)src_size << 8 | 0x10;
	dest += 4;
	while (IS_IN_RANGE(p)) {
		u8 f = 0, *pf = dest++;
		for (s32 i = 7; IS_IN_RANGE(p) && i >= 0; --i) {
			if (p == src) { // 第一个字节之前不可能出现过
				*dest++ = *p++;
				continue;
			}
			/* match */
			s32 len = 0, of;
			for (u8 *t = p - src <= 0x1000 ? src : p - 0x1000; t < p; ++t) {
				while (IS_IN_RANGE(t) && *t != *p)
					++t;
				if (t >= p) // cannot find
					break;
				u8 *p_org = p, *t_org = t, j;
				for (j = 0; j < 18 && IS_IN_RANGE(p) && *t == *p; ++j, ++t, ++p);
				if (j > len) {
					len = j;
					of = p_org - t_org;
				}
				p = p_org, t = lazy ? t : t_org;
			}
			/* encode */
			if (len < 3) { // raw
				*dest++ = *p++;
			} else { // offset + length
				f |= 1 << i;
				--of;
				*(u16*)dest = (len - 3) << 4 | (of & 0xF00) >> 8 | (of & 0xFF) << 8;
				dest += 2;
				p += len;
			}
		}
		*pf = f; // save flag
	}
	return dest - dest_org;
}

s32 LZ77UnComp(u8 *src, u8 *dest)
{
	s32 size = *(u32*)src >> 8, size_org = size;
	src += 4;
	while (1) {
		u8 f = *src++; // flag
		for (s32 i = 7; i >= 0; --i) {
			u8 b = *src++;
			if (f >> i & 1) { // offset + length
				s32 len = (b >> 4) + 2;
				size -= len;
				s32 of = (*src++ | (b & 0xF) << 8) + 1;
				while (len--) {
					*dest = dest[-of];
					++dest;
				}
			} else { // raw
				*dest++ = b;
			}
			if (--size <= 0)
				return size_org;
		}
	}
}

typedef struct {u32 f; s16 l, r;} node;

static void heapify(node *a, s16 *x, s32 l, s32 r)
{
	s32 p = l, c = p * 2 + 1;
	while (c <= r) {
		if (c + 1 <= r && a[x[c]].f > a[x[c + 1]].f)
			++c;
		if (a[x[p]].f <= a[x[c]].f)
			return;
		s16 t = x[p];
		x[p] = x[c];
		x[c] = t;
		p = c, c = p * 2 + 1;
	}
}

static void map_tree(node *a, s16 i, u32 *map, u8 *bmap)
{
	static u32 v;
	static u8 b;
	if (a[i].l == -1) {
		s16 t = i;
		map[t] = v;
		bmap[t] = b;
		return;
	}
	v <<= 1, ++b;
	map_tree(a, a[i].l, map, bmap);
	v |= 1;
	map_tree(a, a[i].r, map, bmap);
	v >>= 1, --b;
}

/* bmode 指示一次性应当写多少bit */
s32 HuffComp(u8 *src, u8 *dest, s32 src_size, u8 bmode)
{
	u8 *p = src, *dest_org = dest;
	u8 bits = bmode ? 8 : 4;
	*(u32*)dest = (u32)src_size << 8 | 0x20 | bits;
	dest += 4;
	s32 n = 1 << bits;
	node a[511];
	s16 x[256];
	s16 cnt = 256;
	/* initialize */
	for (s32 i = 0; i < n; ++i)
		a[i].f = 0, a[i].l = -1, x[i] = i;
	/* read frequency */
	for (; IS_IN_RANGE(p); ++p)
		++a[*p].f;
	if (!bmode) { // 4-bit
		u32 b[16] = {};
		for (s32 i = 0; i < 256; ++i) {
			b[i >> 4] += a[i].f;
			b[i & 0xF] += a[i].f;
		}
		for (s32 i = 0; i < 16; ++i)
			a[i].f = b[i];
	}
	/* build tree */
	for (s32 i = n / 2 - 1; i >= 0; --i)
		heapify(a, x, i, n - 1);
	while (a[x[0]].f == 0) { // 丢弃未出现过的字符
		x[0] = x[--n];
		heapify(a, x, 0, n - 1);
	}
	s32 i, j;
	while (n && (i = x[0], x[0] = x[--n], heapify(a, x, 0, n - 1), n)) {
		j = x[0];
		a[cnt].f = a[i].f + a[j].f;
		a[cnt].l = j;
		a[cnt].r = i;
		x[0] = cnt++;
		heapify(a, x, 0, n - 1);
	}
	/* save tree */
	u8 *tree = dest;
	*dest++ = cnt - 256; // size
	s16 q[511], *qh = q, *qt = q;
	*qt++ = cnt - 1; // root
	for (s32 c = 0; qh < qt;) { // layer by layer
		i = *qh++;
		if (a[i].l == -1) {
			*dest++ = i;
			--c;
			continue;
		}
		if (c >= 0x80) {
			printf("table is too large\n");
			return -1;
		}
		*dest++ = c++ >> 1 | (a[i].l < 256) << 7 | (a[i].r < 256) << 6;
		*qt++ = a[i].l;
		*qt++ = a[i].r;
	}
	/* 将每个字符的编码写到数组里方便后续处理 */
	u32 map[256]; // 很容易证明任意编码的码长不会超过32位
	u8 bmap[256];
	map_tree(a, cnt - 1, map, bmap);
	/* encode */
	p = src;
	u32 *dest32 = (u32*)(tree + ((cnt - 255) << 1));
	u32 v = 0;
	i = 0x20;
	while (IS_IN_RANGE(p)) {
		u8 t = *p++;
		for (s32 j = 0; j < 8; j += bits, t >>= bits) {
			u8 b = t & ((1 << bits) - 1);
			if (i < bmap[b]) {
				*dest32++ = v | map[b] >> (bmap[b] - i);
				i += 0x20 - bmap[b];
				v = map[b] << i;
			} else {
				i -= bmap[b];
				v |= map[b] << i;
			}
		}
	}
	*dest32++ = v;
	return (u8*)dest32 - dest_org;
}

s32 HuffUnComp(u8 *src, u32 *dest)
{
	s32 size = *(u32*)src >> 8, size_org = size;
	u8 *tree = src + 4;
	u8 *root = tree + 1;
	u8 bits = *src & 0xF; //每次写入的bit数(4 or 8)
	u8 n = (bits & 7) + 4; // 写几次才能写完一个word
	u32 *p = (u32*)(tree + ((*tree + 1) << 1)); // data
	u8 *q = root;
	u8 cnt = 0;
	while (size > 0) {
		u32 v = *p++, t;
		for (s32 i = 0x20; --i >= 0; v <<= 1) {
			u8 f = v >> 0x1F;
			u8 b = *q << f;
			q = (u8*)((u32)q & ~1) + (((*q & 0x3F) + 1) << 1) + f; // query tree
			if (b & 0x80) { // is leaf
				t = t >> bits | *q << (0x20 - bits);
				q = root;
				if (++cnt == n) {
					*dest++ = t;
					size -= 4;
					cnt = 0;
					if (size <= 0)
						return size_org;
				}
			}
		}
	}
	return size_org;
}
