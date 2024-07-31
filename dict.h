#ifndef _DICT_H
#define _DICT_H

#include <stdint.h>

typedef size_t hash_t;
typedef const char *key_t;

enum valtype {
	T_CHAR,
	T_SHORT,
	T_INT,
	T_LONG,
	T_LONGLONG,
	T_UCHAR,
	T_USHORT,
	T_UINT,
	T_ULONG,
	T_ULONGLONG,
	T_PTR
};
typedef enum valtype valtype;

struct value_t {
	valtype type;
	union {
		char c;
		short s;
		int i;
		long l;
		long long ll;
		unsigned char uc;
		unsigned short us;
		unsigned int ui;
		unsigned long ul;
		unsigned long long ull;
		void *p;
	};
};
typedef struct value_t value_t, *val_t;

struct entry_t {
	hash_t hash;
	key_t key;
	val_t val;
};
typedef struct entry_t entry_t;

struct dict_t {
	entry_t **entries;
	size_t *indices;
	size_t count;
	uint8_t size_bits;
};
typedef struct dict_t dict_t;

dict_t *dict_new(void);
void dict_delete(dict_t *dict, void (*free_fn)(void*));
val_t dict_query(dict_t *dict, key_t key);
val_t dict_add(dict_t *dict, key_t key, val_t val);
val_t dict_remove(dict_t *dict, key_t key);

#endif