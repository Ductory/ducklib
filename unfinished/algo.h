#ifndef _ALGO_H_
#define _ALGO_H_

typedef int (*compare_t)(const void*, const void*);

void insertion_sort(void *base, size_t n, size_t size, compare_t comp);
void merge_sort(void *base, size_t n, size_t size, compare_t comp);
size_t euler_sieve(unsigned int *prime, size_t n);

#endif