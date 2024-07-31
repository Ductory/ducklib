#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef int (*compare_t)(const void*, const void*);

void
insertion_sort(void *base, size_t n, size_t size, compare_t comp)
{
	typedef uint8_t elem_t[size];
	elem_t *p = base, *k = alloca(sizeof(*k));
	for (size_t i = 1, j; i < n; ++i) {
		memcpy(k, &p[i], size);
		for (j = i - 1; j + 1 && comp(&p[j], k) > 0; --j)
			memcpy(&p[j + 1], &p[j], size);
		memcpy(&p[j + 1], k, size);
	}
}

static size_t _size;
static compare_t _comp;

static void
merge(void *base, size_t l, size_t m, size_t r)
{
	typedef unsigned char elem_t[_size];
	size_t n1 = m - l + 1, n2 = r - m;
	elem_t *p = base;
	elem_t *al = malloc(_size * n1);
	elem_t *ar = malloc(_size * n2);
	memcpy(al, &p[l],     _size * n1);
	memcpy(ar, &p[m + 1], _size * n2);
	size_t i = 0, j = 0, k;
	for (k = l; i < n1 && j < n2; ++k) {
		if (_comp(&al[i], &ar[j]) <= 0)
			memcpy(&p[k], &al[i++], _size);
		else
			memcpy(&p[k], &ar[j++], _size);
	}
	if (i < n1)
		memcpy(&p[k], &al[i], _size * (n1 - i));
	else
		memcpy(&p[k], &ar[j], _size * (n2 - j));
	free(al);
	free(ar);
}

static void
mergesort_impl(void *base, size_t l, size_t r)
{
	if (l < r) {
		size_t m = (l + r) >> 1;
		mergesort_impl(base, l, m);
		mergesort_impl(base, m + 1, r);
		merge(base, l, m, r);
	}
}
/**
 * 归并排序
 * @param base 待排序的数组
 * @param n    数组元素个数
 * @param size 元素大小
 * @param comp 用于比较的函数指针
 */
void
merge_sort(void *base, size_t n, size_t size, compare_t comp)
{
	_size = size;
	_comp = comp;
	mergesort_impl(base, 0, n - 1);
}

/**
 * 欧拉筛
 * @param prime 返回的素数数组(allocated by caller)
 * @param n 素数最大取值
 * @return 总共的素数个数
 */
size_t
euler_sieve(unsigned int *prime, size_t n)
{
	uint8_t *visit = calloc(n + 1, 1);
	size_t cnt = 0;
	for (size_t i = 2; i <= n; ++i) {
		if (!visit[i])
			prime[cnt++] = i;
		for (size_t j = 0; prime[j] * i <= n && j < cnt; ++j) {
			visit[prime[j] * i] = 1;
			if (i % prime[j] == 0)
				break;
		}
	}
	free(visit);
	return cnt;
}

static void swap(uint32_t *a, uint32_t *b)
{
	uint32_t tmp = *a;
	*a = *b;
	*b = tmp;
}

uint32_t gcd(uint32_t a, uint32_t b)
{
    int k = 0;
    while (!(a & 1) && !(b & 1))
        ++k, a >>= 1, b >>= 1;
    while (!(a & 1)) a >>= 1;
    while (!(b & 1)) b >>= 1;
    if (a < b) swap(&a, &b);
    while (a != b) {
        a -= b;
        if (a < b) swap(&a, &b);
    }
    return a << k;
}

uint32_t ext_euclid(uint32_t a, uint32_t b, int *x, int *y)
{
    if (!b) {
        *x = 1, *y = 0;
        return a;
    }
    uint32_t d = ext_euclid(b, a % b, x, y);
    int t = *x;
    *x = *y;
    *y = t - a / b * *y;
    return d;
}
