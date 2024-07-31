#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dict.h"


#define DICT_INIT_BITS 5
#define DICT_INIT_SIZE ((size_t)1 << DICT_INIT_BITS)

#define DICT_SIZE(dict) ((size_t)1 << (dict)->size_bits)
#define DICT_MASK(dict) (DICT_SIZE(dict) - 1)
#define NEXT_INDEX(dict,i) (((i) * 5 + 1) & DICT_MASK(dict))
#define GET_ENTRY(dict,i) ((dict)->indices[i])
#define IS_ENTRY(ent,h,k) ((ent)->hash == h && strcmp((ent)->key, k) == 0)

#define IDX_EMPTY ((size_t)-1)
#define IDX_DUMMY ((size_t)-2)
#define IDX_MAX   IDX_DUMMY

static value_t expire;

dict_t *dict_new(void)
{
	dict_t *dict = malloc(sizeof(dict_t));
	size_t size = DICT_SIZE(dict);
	dict->size_bits = DICT_INIT_BITS;
	dict->entries   = malloc(size * sizeof(entry_t*));
	dict->indices   = malloc(size * sizeof(size_t));
	dict->count     = 0;
	memset(dict->indices, -1, size * sizeof(size_t));
	return dict;
}

static void free_key(key_t key)
{
	free((void*)key);
}

static void free_val(val_t val)
{
	expire = *val;
	free(val);
}

static void free_entry(entry_t *entry)
{
	free_key(entry->key);
	free_val(entry->val);
	free(entry);
}

void dict_delete(dict_t *dict, void (*free_fn)(void*))
{
	for (size_t e = 0; e < dict->count; ++e) {
		entry_t *entry = dict->entries[e];
		if (!entry)
			continue;
		free_entry(entry);
		if (free_fn && expire.type == T_PTR)
			free_fn(expire.p);
	}
	free(dict->entries);
	free(dict->indices);
	free(dict);
}

static hash_t hash_key(key_t key)
{
	hash_t h = 1;
	const uint8_t *k = (const uint8_t*)key;
	while (*k)
		h += (h << 5) + (h >> 27) + *k++;
	return h;
}

static size_t query_index(dict_t *dict, key_t key)
{
	hash_t hash = hash_key(key);
	size_t i = hash & DICT_MASK(dict), e;
	while ((e = GET_ENTRY(dict, i)) != IDX_EMPTY) {
		if (e != IDX_DUMMY && IS_ENTRY(dict->entries[e], hash, key))
			return i;
		i = NEXT_INDEX(dict, i);
	}
	return IDX_EMPTY;
}

val_t dict_query(dict_t *dict, key_t key)
{
	size_t e = GET_ENTRY(dict, query_index(dict, key));
	if (e == IDX_EMPTY)
		return NULL;
	return dict->entries[e]->val;
}

static void expand_dict(dict_t *dict)
{
	size_t size = (size_t)1 << ++dict->size_bits;
	dict->entries = realloc(dict->entries, size * sizeof(entry_t*));
	dict->indices = realloc(dict->indices, size * sizeof(size_t));
	memset(dict->indices, -1, sizeof(size_t) * size);

	size_t cnt = 0;
	for (size_t e = 0; e < dict->count; ++e) {
		if (!dict->entries[e])
			continue;
		hash_t hash = dict->entries[e]->hash;
		size_t i = hash & (size - 1);
		while (GET_ENTRY(dict, i) < IDX_MAX)
			i = NEXT_INDEX(dict, i);
		dict->indices[i] = i;
		if (e != cnt)
			dict->entries[cnt++] = dict->entries[i];
	}
	dict->count = cnt;
}

val_t dict_add(dict_t *dict, key_t key, val_t val)
{
	hash_t hash = hash_key(key);
	size_t i = hash & DICT_MASK(dict), e;
	while ((e = GET_ENTRY(dict, i)) < IDX_MAX) {
		entry_t *entry = dict->entries[e];
		if (IS_ENTRY(entry, hash, key)) { // substitute
			free_val(entry->val);
			entry->val = val;
			return &expire;
		}
		i = NEXT_INDEX(dict, i);
	}
	entry_t *entry = malloc(sizeof(entry_t));
	entry->hash = hash;
	entry->key  = strdup(key);
	entry->val  = malloc(sizeof(value_t));
	memcpy(entry->val, val, sizeof(value_t));

	dict->indices[i] = dict->count;
	dict->entries[dict->count++] = entry;

	if (dict->count > DICT_SIZE(dict) * 2 / 3)
		expand_dict(dict);
}

val_t dict_remove(dict_t *dict, key_t key)
{
	size_t i = query_index(dict, key);
	size_t e = GET_ENTRY(dict, i);
	if (e == IDX_EMPTY)
		return NULL;
	entry_t *entry = dict->entries[e];
	free_entry(entry);

	dict->indices[i] = IDX_DUMMY;
	dict->entries[e] = NULL;

	return &expire;
}