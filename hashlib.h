#define _V(x) ((void*)(x))
typedef unsigned int HASH;

typedef struct _HashNode
{
	HASH hash;
	void *data;
	struct _HashNode *next;
} HashNode;

typedef struct _HashTable
{
	HashNode **table;
	int count;
	int size;
} HashTable;

HashTable *newHashTable(void);
void *hash_get(const HashTable *ht, const void *k, void *const def, HASH (*hash)(const void *k));
void hash_put(HashTable *ht, const void *k, void *const v, HASH (*hash)(const void *k));
void *hash_remove(HashTable *ht, const void *k, HASH (*hash)(const void *k));

HASH stimes31(const void *k);