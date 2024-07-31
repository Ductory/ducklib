#include "random.h"
#include <stdlib.h>

#define linear_congruential_engine(UIntType,A,C,M,Engine) \
typedef struct Engine { \
	UIntType x; \
} Engine; \
\
Engine *new_ ## Engine(UIntType seed) { \
	Engine *e = malloc(sizeof(Engine)); \
	if (!e) return NULL; \
	e->x = seed; \
	return e; \
} \
\
void delete_ ## Engine(Engine *e) { free(e); } \
\
void Engine ## _srand(Engine *e, UIntType s) { e->x = s; } \
\
UIntType Engine ## _rand(Engine *e) { return e->x = (A * e->x + C) % M; }

linear_congruential_engine(uint_fast32_t, 16807UL, 0UL, 2147483647UL, minstd_rand0);

linear_congruential_engine(uint_fast32_t, 48271UL, 0UL, 2147483647UL, minstd_rand);


#define mersenne_twister_engine(UIntType,W,N,M,R,A,U,D,S,B,T,C,L,F,Engine) \
typedef struct Engine { \
	size_t i; \
	UIntType x[N]; \
} Engine; \
\
Engine *new_ ## Engine(UIntType seed) { \
	Engine *e = malloc(sizeof(Engine)); \
	if (!e) return NULL; \
	Engine ## _srand(e, seed); \
	return e; \
} \
\
void delete_ ## Engine(Engine *e) { free(e); } \
\
void Engine ## _srand(Engine *e, UIntType s) { \
	const size_t Max = ~0ULL >> (sizeof(0ULL) * CHAR_BIT - W); \
	e->x[0] = s & Max; \
	for (size_t i = 1; i < N; ++i) \
		e->x[i] = (F * (e->x[i - 1] ^ (e->x[i - 1] >> (W - 2))) + i) & Max; \
	e->i = 0; \
} \
\
UIntType Engine ## _rand(Engine *e) { \
	size_t j = (e->i + 1) % N; \
	UIntType mask = (1UL << R) - 1; \
	UIntType yp = (e->x[e->i] & ~mask) | (e->x[j] & mask); \
	e->x[e->i] = e->x[(e->i + M) % N] ^ (yp >> 1) ^ (A * (yp & 1)); \
	UIntType z = e->x[e->i] ^ ((e->x[e->i] >> U) & D); \
	e->i = j; \
	z ^= (z << S) & B; \
	z ^= (z << T) & C; \
	return z ^ (z >> L); \
}

mersenne_twister_engine(uint_fast32_t,
	32, 624, 397, 31,
	0x9908b0dfUL, 11,
	0xffffffffUL, 7,
	0x9d2c5680UL, 15,
	0xefc60000UL, 18, 1812433253UL, mt19937);

mersenne_twister_engine(uint_fast64_t,
	64, 312, 156, 31,
	0xb5026f5aa96619e9ULL, 29,
	0x5555555555555555ULL, 17,
	0x71d67fffeda60000ULL, 37,
	0xfff7eee000000000ULL, 43,
	6364136223846793005ULL, mt19937_64);