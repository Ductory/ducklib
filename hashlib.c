#include <stdlib.h>
#include "hashlib.h"

#define HASHTABLE_INIT_SIZE 32

/* create a new hashtable */
HashTable *newHashTable(void)
{
	HashTable *ht = malloc(sizeof(HashTable));
	ht->table = calloc(HASHTABLE_INIT_SIZE, sizeof(HashNode*));
	ht->size = HASHTABLE_INIT_SIZE;
	ht->count = 0;
	return ht;
}

void freeHashTable(HashTable *ht, void (*freenode)(HashNode *node))
{
	for (int i = 0; i < ht->size; ++i) {
		HashNode *n = ht->table[i];
		while (n) {
			if (freenode)
				freenode(n);
			HashNode *t = n;
			n = n->next;
			free(t);
		}
	}
	free(ht->table);
	free(ht);
}

/* query the table, return data if node has existed */
void *hash_get(const HashTable *ht, const void *k, const void *def, HASH (*hash)(const void *k))
{
	HASH h = hash(k);
	HashNode *n = ht->table[h & (ht->size - 1)];
	while (n) {
		if (n->hash == h) //weak match. if need a strict match, you should implement it by yourself
			return n->data;
		n = n->next;
	}
	return (void*)def; //do not exists, return default value
}

/* if the node has existed, it cannot be visited until the new node has been removed */
void hash_put(HashTable *ht, const void *k, const void *v, HASH (*hash)(const void *k))
{
	HASH h = hash(k);
	int i = h & (ht->size - 1), j;
	//alloc a new node
	HashNode *n = malloc(sizeof(HashNode));
	n->hash = h; n->data = (void*)v; n->next = ht->table[i];
	ht->table[i] = n;
	++ht->count;
	//expand the table
	if (ht->count > ht->size * 3 >> 2) {
		//double size the table
		HashNode **t = calloc(ht->size << 1, sizeof(HashNode*));
		HashNode *nn;
		//update the table
		for (i = 0; i < ht->size; ++i) {
			n = ht->table[i];
			while (n) {
				j = (n->hash & ht->size) + i;
				nn = n->next;
				n->next = t[j];
				t[j] = n;
				n = nn;
			}
		}
		ht->size <<= 1;
		free(ht->table);
		ht->table = t;
	}
}

/* query the table, remove node if node has existed. the data should be freed by caller */
void *hash_remove(HashTable *ht, const void *k, HASH (*hash)(const void *k))
{
	HASH h = hash(k);
	int i = h & (ht->size - 1);
	HashNode *n = ht->table[i], *p = NULL;
	while (n) {
		if (n->hash != h) {
			p = n;
			n = n->next;
			continue;
		}
		if (!p)
			ht->table[i] = n->next;
		else
			p->next = n->next;
		void *d = n->data;
		free(n);
		--ht->count;
		return d;
	}
	return NULL;
}

void *hash_update(HashTable *ht, const void *k, void *v, HASH (*hash)(const void *k))
{
	HASH h = hash(k);
	HashNode *n = ht->table[h & (ht->size - 1)];
	while (n) {
		if (n->hash == h) {
			void *tmp = n->data;
			n->data = v;
			return tmp;
		}
		n = n->next;
	}
	hash_put(ht, k, v, hash);
	return NULL; //do not exists, return default value
}

void hash_traverse(HashTable *ht, void(*traverse)(HashNode *n, void *ex), void *ex)
{
	for (int i = 0; i < ht->size; ++i) {
		HashNode *n = ht->table[i];
		while (n) {
			traverse(n, ex);
			n = n->next;
		}
	}
}


/* hash functions */

HASH stimes31(const void *k)
{
	HASH h = 0;
	for (int i = 0; ((char*)k)[i]; ++i)
		h = h * 31 + ((unsigned char*)k)[i];
	return h;
}

HASH sshift(const void *k)
{
	HASH h = 1;
	for (int i = 0; ((char*)k)[i]; ++i)
		h = h + (h << 5) + (h >> 27) + ((unsigned char*)k)[i];
	return h;
}

HASH iself(const void *k)
{
	return PTOI(k);
}
