#ifndef _RANDOM_H
#define _RANDOM_H

#include <stdint.h>

#define declare_random_engine(UIntType,Engine) \
typedef struct Engine Engine; \
Engine *new_ ## Engine(UIntType seed); \
void delete_ ## Engine(Engine *e); \
void Engine ## _srand(Engine *e, UIntType s); \
UIntType Engine ## _rand(Engine *e);

#define MINSTD_DEFAULT_SEED 1u
declare_random_engine(uint_fast32_t, minstd_rand0);
declare_random_engine(uint_fast32_t, minstd_rand);
#define MT19937_DEFAULT_SEED 5489u
declare_random_engine(uint_fast32_t, mt19937);
declare_random_engine(uint_fast64_t, mt19937_64);

#endif