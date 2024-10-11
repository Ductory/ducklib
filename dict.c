#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dict.h"

#include <stdio.h>

#define DICT_INIT_BITS 5
#define DICT_INIT_SIZE ((size_t)1 << DICT_INIT_BITS)

#define DICT_SIZE(dict) ((size_t)1 << (dict)->size_bits)
#define DICT_MASK(dict) (DICT_SIZE(dict) - 1)
#define NEXT_INDEX(dict,i) (((i) * 5 + 1) & DICT_MASK(dict))
#define GET_ENTRY(dict,i) ((dict)->indices[i])
#define IS_ENTRY(dict,ent,h,k) ((ent)->hash == h && (dict)->vtbl->equal((ent)->key, k))

#define IDX_EMPTY ((size_t)-1)
#define IDX_DUMMY ((size_t)-2)
#define IDX_MAX   IDX_DUMMY
#define IDX_INVALID ((size_t)-1)

static const IDictVtbl _DictVtbl = {
	.del = dict_del,
	.add = dict_add,
	.remove = dict_remove,
	.query = dict_query
};

/**
 * create a new dictionary
 * @param  vtbl costom virtual table
 * @return      dictionary
 */
dict_t *dict_new(const ODictVtbl *vtbl)
{
	dict_t *dict = malloc(sizeof(dict_t));
	dict->vtbl = malloc(sizeof(DictVtbl));
	memcpy((void*)dict->vtbl, &(DictVtbl){_DictVtbl, *vtbl}, sizeof(DictVtbl));

	dict->size_bits = DICT_INIT_BITS;
	size_t size = DICT_SIZE(dict);
	dict->entries   = malloc(size * sizeof(entry_t*));
	dict->indices   = malloc(size * sizeof(size_t));
	dict->count     = 0;
	memset(dict->indices, -1, size * sizeof(size_t));

	return dict;
}

static void free_entry(dict_t *dict, entry_t *entry)
{
	dict->vtbl->free_key(entry->key);
	dict->vtbl->free_val(entry->val);
	free(entry);
}

/**
 * delete a dictionary
 * @param dict 
 */
void dict_del(dict_t *dict)
{
	for (size_t e = 0; e < dict->count; ++e) {
		entry_t *entry = dict->entries[e];
		if (!entry)
			continue;
		free_entry(dict, entry);
	}
	free(dict->entries);
	free(dict->indices);
	free((void*)dict->vtbl);
	free(dict);
}

static size_t query_index(dict_t *dict, void *key)
{
	hash_t hash = dict->vtbl->hash(key);
	size_t i = hash & DICT_MASK(dict), e;
	while ((e = dict->indices[i]) != IDX_EMPTY) {
		if (e != IDX_DUMMY && IS_ENTRY(dict, dict->entries[e], hash, key))
			return i;
		i = NEXT_INDEX(dict, i);
	}
	return IDX_INVALID;
}

/**
 * query in dictionary by key
 * @param  dict 
 * @param  key  
 * @return      val
 */
void *dict_query(dict_t *dict, void *key)
{
	size_t i = query_index(dict, key);
	if (i == IDX_INVALID)
		return NULL;
	return dict->entries[dict->indices[i]]->val;
}

static void expand_dict(dict_t *dict)
{
	++dict->size_bits;
	size_t size = DICT_SIZE(dict), mask = size - 1;
	dict->entries = realloc(dict->entries, size * sizeof(entry_t*));
	dict->indices = realloc(dict->indices, size * sizeof(size_t));
	memset(dict->indices, -1, size * sizeof(size_t));

	size_t cnt = 0;
	for (size_t e = 0; e < dict->count; ++e) {
		if (!dict->entries[e])
			continue;
		hash_t hash = dict->entries[e]->hash;
		size_t i = hash & mask;
		while (dict->indices[i] < IDX_MAX)
			i = NEXT_INDEX(dict, i);
		dict->indices[i] = cnt;
		dict->entries[cnt] = dict->entries[e];
		++cnt;
	}
	dict->count = cnt;
}

/**
 * add an entry to dictionary
 * @param dict 
 * @param key  
 * @param val  
 */
void dict_add(dict_t *dict, void *key, void *val)
{
	hash_t hash = dict->vtbl->hash(key);
	size_t i = hash & DICT_MASK(dict), e;
	while ((e = dict->indices[i]) < IDX_MAX) {
		entry_t *entry = dict->entries[e];
		if (IS_ENTRY(dict, entry, hash, key)) { // substitute
			dict->vtbl->free_val(entry->val);
			entry->val = dict->vtbl->copy_val(val);
			return;
		}
		i = NEXT_INDEX(dict, i);
	}

	entry_t *entry = malloc(sizeof(entry_t));
	entry->hash = hash;
	entry->key  = dict->vtbl->copy_key(key);
	entry->val  = dict->vtbl->copy_val(val);

	dict->indices[i] = dict->count;
	dict->entries[dict->count] = entry;
	++dict->count;

	if (dict->count > DICT_SIZE(dict) * 2 / 3)
		expand_dict(dict);
}

/**
 * remove an entry from dictionary
 * @param dict 
 * @param key  
 */
void dict_remove(dict_t *dict, void *key)
{
	size_t i = query_index(dict, key);
	if (i == IDX_INVALID)
		return;
	size_t e = dict->indices[i];
	entry_t *entry = dict->entries[e];
	printf("i:%zu, e:%zu, ent:%p\n", i, e, entry);
	free_entry(dict, entry);
	dict->indices[i] = IDX_DUMMY;
	dict->entries[e] = NULL;
}


//////////
// hash //
//////////

hash_t hash_shift32(void *key)
{
	hash_t h = 1;
	const uint8_t *k = (const uint8_t*)key;
	while (*k)
		h += (h << 5) + (h >> 27) + *k++;
	return h;
}
