#ifndef _BPTREE_H
#define _BPTREE_H

#include <string.h>

typedef void *key_t, *val_t;
typedef const void *const_key_t;

typedef struct bptree_t bptree_t;
typedef struct bpnode_t bpnode_t;

typedef struct BPlusTreeVtbl {
	int (*compare)(const void *a, const void *b);
	void (*free)(void *p);
} BPlusTreeVtbl;

struct bpnode_t {
	int isleaf;
	size_t num; // number of keys
	const_key_t *keys;
	union {
		bpnode_t **children; // internal only
		val_t *vals; // leaf only
	};
	bpnode_t *next;
	bpnode_t *parent;
};

struct bptree_t {
	const BPlusTreeVtbl *vtbl; // user function table
	size_t degree;
	bpnode_t *root;
};

#define INVALID_INDEX (-1)

#define elemmove(dst,src,n) memmove(dst, src, sizeof(*(dst)) * (n))
#define assert_tree(t,ret) do {if (!(t) || !(t)->root) return ret;} while (0)

#endif
