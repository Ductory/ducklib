
#ifdef _WIN64
	#define ITOP(x) ((void*)(__int64)(x))
	#define PTOI(x) ((__int64)(x))
#else
	#define ITOP(x) ((void*)(__int32)(x))
	#define PTOI(x) ((__int32)(x))
#endif

typedef unsigned int HASH;

typedef struct _HashNode {
	HASH hash;
	void *data;
	struct _HashNode *next;
} HashNode;

typedef struct _HashTable {
	HashNode **table;
	int count;
	int size;
} HashTable;

HashTable *newHashTable(void);
void freeHashTable(HashTable *ht, void (*freenode)(HashNode *node));
void *hash_get(const HashTable *ht, const void *k, const void *def, HASH (*hash)(const void *k));
void hash_put(HashTable *ht, const void *k, const void *v, HASH (*hash)(const void *k));
void *hash_remove(HashTable *ht, const void *k, HASH (*hash)(const void *k));
void *hash_update(HashTable *ht, const void *k, void *v, HASH (*hash)(const void *k));
void hash_traverse(HashTable *ht, void(*traverse)(HashNode *n, void *ex), void *ex);

HASH stimes31(const void *k);
HASH sshift(const void *k);
HASH iself(const void *k);
