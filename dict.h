#ifndef _DICT_H
#define _DICT_H

#include <stdint.h>

typedef size_t hash_t;

typedef struct entry_t {
	hash_t hash;
	void *key;
	void *val;
} entry_t;

typedef struct dict_t dict_t;

typedef struct IDictVtbl {
	void (*del)(dict_t*);
	void *(*query)(dict_t*, void*);
	void (*add)(dict_t*, void*, void*);
	void (*remove)(dict_t*, void*);
} IDictVtbl;

typedef struct ODictVtbl {
	hash_t (*hash)(void*);
	int (*equal)(void*, void*);
	void (*free_key)(void*);
	void (*free_val)(void*);
	void *(*copy_key)(void*);
	void *(*copy_val)(void*);
} ODictVtbl;

typedef struct DictVtbl {
	IDictVtbl;
	ODictVtbl;
} DictVtbl;

struct dict_t {
	const DictVtbl *vtbl;
	entry_t **entries;
	size_t *indices;
	size_t count;
	uint8_t size_bits;
};

dict_t *dict_new(const ODictVtbl *vtbl);
void dict_del(dict_t *dict);
void *dict_query(dict_t *dict, void *key);
void dict_add(dict_t *dict, void *key, void* val);
void dict_remove(dict_t *dict, void *key);

/* hash */
hash_t hash_shift32(void *key);

#endif
