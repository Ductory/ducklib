/**
 * more details, see https://ductory.github.io/2024/06/26/%E6%B5%85%E8%B0%88B+%E6%A0%91/
 */

#include <stdlib.h>
#include "bptree.h"

/* memory */

/**
 * allocate a new node
 * @param  t 
 * @return   new node
 */
static bpnode_t *new_node(const bptree_t *t)
{
	bpnode_t *n = malloc(sizeof(bpnode_t));
	n->keys = malloc(t->degree * sizeof(key_t));
	n->vals = malloc((t->degree + 1) * sizeof(val_t));
	return n;
}

/**
 * create a new b+ tree
 * @param  vtbl   vtable, allocated by caller
 * @param  degree 
 * @return        new tree
 */
bptree_t *bpt_new(const BPlusTreeVtbl *vtbl, size_t degree)
{
	// degree must >= 3
	if (degree < 3)
		return NULL;
	bptree_t *tree = malloc(sizeof(bptree_t));
	tree->vtbl = vtbl;
	tree->degree = degree;
	tree->root = NULL;
	return tree;
}

static void free_node(bpnode_t *n)
{
	free(n->keys);
	free(n->vals);
	free(n);
}

/* here `fn` is a function used to clean up value */

static void free_leaf(bpnode_t *n, void (*fn)(void*))
{
	for (size_t i = 0; i < n->num; ++i)
		fn(n->vals[i]);
	free_node(n);
}

static void free_internal(bpnode_t *n, void (*fn)(void*))
{
	if (n->isleaf) {
		free_leaf(n, fn);
		return;
	}
	for (size_t i = 0; i < n->num; ++i)
		free_internal(n->children[i], fn);
}

/**
 * destroy b+ tree
 * @param t 
 */
void bpt_free(bptree_t *t)
{
	assert_tree(t, );
	free_internal(t->root, t->vtbl->free);
	free(t);
}


/* search */

static size_t search_key(const_key_t key, const_key_t *keys, size_t count, int (*comp)(const void *, const void *))
{
	size_t i = 0, j = count - 1;
	while (i < j + 1) {
		size_t m = (i + j) >> 1;
		int res = comp(key, keys[m]);
		if (res == 0)
			return m;
		res < 0 ? j = m - 1 : (i = m + 1);
	}
	return INVALID_INDEX;
}

static size_t search_child(const_key_t key, const_key_t *keys, size_t count, int (*comp)(const void *, const void *))
{
	size_t i = 0, j = count - 1;
	while (i < j + 1) {
		size_t m = (i + j) >> 1;
		comp(key, keys[m]) < 0 ? j = m - 1 : (i = m + 1);
	}
	return i;
}

/**
 * find the leaf containing the key
 * @param  t   
 * @param  n   
 * @param  key 
 * @return     leaf containing the key
 */
static bpnode_t *search_leaf(const bptree_t *t, const bpnode_t *n, const_key_t key)
{
	if (n->isleaf)
		return (bpnode_t*)n;
	size_t i = search_child(key, n->keys, n->num, t->vtbl->compare);
	return search_leaf(t, n->children[i], key);
}

/**
 * find the value by its key
 * @param  t   
 * @param  key 
 * @return     value of the key
 */
val_t bpt_search(const bptree_t *t, const_key_t key)
{
	assert_tree(t, NULL);
	bpnode_t *leaf = search_leaf(t, t->root, key);
	size_t i = search_key(key, leaf->keys, leaf->num, t->vtbl->compare);
	return i != INVALID_INDEX ? leaf->vals[i] : NULL;
}

/**
 * return a enumerator (ie. the first leaf)
 * @param  t 
 * @return   enumerator
 */
bpnode_t *bpt_enum(const bptree_t *t)
{
	assert_tree(t, NULL);
	bpnode_t *n = t->root;
	while (!n->isleaf)
		n = n->children[0];
	return n;
}


/* insertion */

static void insert_internal(bptree_t *t, bpnode_t *ln, bpnode_t *rn, const_key_t key);

/**
 * split internal
 * @param t 
 * @param n internal
 * @details
 * a bit different from splitting leaf.
 * for nodes k1 ... lk mk rk ... kn
 * we split to
 * .. mk ..
 *   /  \
 *  lk  rk
 * which is same as b tree.
 */
static void split_internal(bptree_t *t, bpnode_t *n)
{
	bpnode_t *nn = new_node(t);
	nn->isleaf = 0;
	nn->parent = n->parent;
	n->num >>= 1;
	nn->num = t->degree - n->num - 1;
	elemmove(nn->keys, n->keys + n->num + 1, nn->num);
	elemmove(nn->children, n->children + n->num + 1, nn->num + 1);
	for (size_t i = 0; i < nn->num + 1; ++i)
		nn->children[i]->parent = nn;

	insert_internal(t, n, nn, n->keys[n->num]);
}

/**
 * insert a record into internal
 * @param  t   
 * @param  ln  left node
 * @param  rn  right node
 * @param  key 
 */
static void insert_internal(bptree_t *t, bpnode_t *ln, bpnode_t *rn, const_key_t key)
{
	// root is full
	bpnode_t *parent = rn->parent;
	if (parent == NULL) {
		parent = new_node(t);
		parent->isleaf = 0;
		parent->num = 1;
		parent->keys[0] = key;
		parent->children[0] = ln;
		parent->children[1] = rn;
		parent->parent = NULL;
		t->root = ln->parent = rn->parent = parent;
		return;
	}
	// insert record
	size_t i = search_child(key, parent->keys, parent->num, t->vtbl->compare);
	size_t j = i + 1;
	elemmove(parent->keys + i + 1, parent->keys + i, parent->num - i);
	elemmove(parent->children + j + 1, parent->children + j, parent->num - i);
	++parent->num;
	parent->keys[i] = key;
	parent->children[j] = rn;
	// full, split node
	if (parent->num == t->degree)
		split_internal(t, parent);
}

/**
 * split a leaf node into two
 * @param t 
 * @param n leaf
 * for nodes k1 ... lk rk ... kn
 * we split to
 * .. rk ..
 *   /  \
 *  lk  rk
 */
void split_leaf(bptree_t *t, bpnode_t *n)
{
	bpnode_t *nn = new_node(t);
	nn->isleaf = 1;
	nn->next = n->next;
	n->next = nn;
	nn->parent = n->parent;

	n->num >>= 1;
	nn->num = t->degree - n->num;
	elemmove(nn->keys, n->keys + n->num, nn->num);
	elemmove(nn->vals, n->vals + n->num, nn->num);
	// update parent node
	insert_internal(t, n, nn, nn->keys[0]);
}

/**
 * insert a record into leaf
 * @param t   
 * @param n   leaf
 * @param key 
 * @param val 
 */
static void insert_leaf(bptree_t *t, bpnode_t *n, const_key_t key, val_t val)
{
	size_t i = search_child(key, n->keys, n->num, t->vtbl->compare);
	elemmove(n->keys + i + 1, n->keys + i, n->num - i);
	elemmove(n->vals + i + 1, n->vals + i, n->num - i);
	++n->num;
	n->keys[i] = key;
	n->vals[i] = val;
	// full
	if (n->num == t->degree)
		split_leaf(t, n);
}

/**
 * insert a key-value pair
 * @param  t   
 * @param  key 
 * @param  val 
 * @return     1 if success
 */
int bpt_insert(bptree_t *t, const_key_t key, val_t val)
{
	if (!t) return 0;
	// empty tree.
	// we allocate a new leaf as the root
	if (t->root == NULL) {
		bpnode_t *leaf = new_node(t);
		leaf->isleaf = 1;
		leaf->num = 1;
		leaf->keys[0] = key;
		leaf->vals[0] = val;
		leaf->next = NULL;
		leaf->parent = NULL;
		t->root = leaf;
		return 1;
	}

	bpnode_t *leaf = search_leaf(t, t->root, key);
	// we don't allow repeat keys
	if (search_key(key, leaf->keys, leaf->num, t->vtbl->compare) != INVALID_INDEX)
		return 0;

	insert_leaf(t, leaf, key, val);
	return 1;
}

/**
 * update a record
 * @param  t   
 * @param  key 
 * @param  val 
 * @return     original value
 */
val_t bpt_update(bptree_t *t, const_key_t key, val_t val)
{
	assert_tree(t, NULL);
	bpnode_t *leaf = search_leaf(t, t->root, key);
	size_t i = search_key(key, leaf->keys, leaf->num, t->vtbl->compare);
	if (i == INVALID_INDEX)
		return NULL;
	val_t tmp = leaf->vals[i];
	leaf->vals[i] = val;
	return tmp;
}

/**
 * bulk load
 * @param t     
 * @param keys  
 * @param vals  
 * @param count 
 */
void bpt_load(bptree_t *t, key_t *keys, val_t *vals, size_t count)
{
	if (!t) return;
	// allocate root
	bpnode_t *n = new_node(t);
	n->isleaf = 1;
	n->num = 0;
	n->next = NULL;
	n->parent = NULL;
	t->root = n;

	for (size_t i = 0, j = 0; i < count; ++i, ++j) {
		++n->num;
		n->keys[j] = keys[i];
		n->vals[j] = vals[i];
		// full
		if (n->num == t->degree) {
			split_leaf(t, n);
			n = n->next;
			j = n->num - 1;
		}
	}
}


/* deletion */

/**
 * delete a record in internal
 * @param t    
 * @param n    internal
 * @param idx  
 */
static void delete_internal(bptree_t *t, bpnode_t *n, size_t idx)
{
	// delete record
	// index is always > 0, because it's right sibling's index
	elemmove(n->keys + idx - 1, n->keys + idx, n->num - idx);
	elemmove(n->children + idx, n->children + idx + 1, n->num - idx);
	--n->num;

	bpnode_t *parent = n->parent;
	// root
	if (!parent) {
		if (!n->num) {
			t->root = n->children[0];
			n->children[0]->parent = NULL;
			free_node(n);
		}
		return;
	}

	if (n->num >= (t->degree - 1) >> 1)
		return;

	size_t lsibling = INVALID_INDEX, rsibling = INVALID_INDEX, curr;
	curr = search_child(n->keys[0], parent->keys, parent->num, t->vtbl->compare);
	if (curr)
		lsibling = curr - 1;
	if (curr < parent->num)
		rsibling = curr + 1;

	bpnode_t *sibling = NULL;
	// steal
	if (lsibling != INVALID_INDEX && parent->children[lsibling]->num > (t->degree - 1) >> 1) {
		sibling = parent->children[lsibling];
		elemmove(n->keys + 1, n->keys, n->num);
		elemmove(n->children + 1, n->children, n->num + 1);
		++n->num;
		size_t i = sibling->num--;
		n->keys[0] = parent->keys[lsibling];
		n->children[0] = sibling->children[i];
		n->children[0]->parent = n;
		parent->keys[lsibling] = sibling->keys[i - 1];
	} else if (rsibling != INVALID_INDEX && parent->children[rsibling]->num > (t->degree - 1) >> 1) {
		sibling = parent->children[rsibling];
		sibling->children[0]->parent = n;
		size_t i = n->num++;
		n->keys[i] = parent->keys[curr];
		parent->keys[curr] = sibling->keys[0];
		n->children[i + 1] = sibling->children[0];
		elemmove(sibling->keys, sibling->keys + 1, sibling->num - 1);
		elemmove(sibling->children, sibling->children + 1, sibling->num);
		--sibling->num;
	}
	if (sibling)
		return;
	// merge
	if (lsibling != INVALID_INDEX) {
		sibling = parent->children[lsibling];
		sibling->keys[sibling->num] = parent->keys[lsibling];
		elemmove(sibling->keys + sibling->num + 1, n->keys, n->num);
		elemmove(sibling->children + sibling->num + 1, n->children, n->num + 1);
		sibling->num += n->num + 1;
		for (size_t i = 0; i <= n->num; ++i)
			n->children[i]->parent = sibling;
		delete_internal(t, sibling->parent, curr);
		free_node(n);
	} else if (rsibling != INVALID_INDEX) {
		sibling = parent->children[rsibling];
		n->keys[n->num] = parent->keys[curr];
		elemmove(n->keys + n->num + 1, sibling->keys, sibling->num);
		elemmove(n->children + n->num + 1, sibling->children, sibling->num + 1);
		n->num += sibling->num + 1;
		for (size_t i = 0; i <= sibling->num; ++i)
			sibling->children[i]->parent = n;
		delete_internal(t, sibling->parent, rsibling);
		free_node(sibling);
	}
}

/**
 * delete a record in leaf
 * @param  t   
 * @param  n   leaf
 * @param  idx 
 * @return     value of the key
 */
static val_t delete_leaf(bptree_t *t, bpnode_t *n, size_t idx)
{
	// delete record
	val_t val = n->vals[idx];
	--n->num;
	elemmove(n->keys + idx, n->keys + idx + 1, n->num - idx);
	elemmove(n->vals + idx, n->vals + idx + 1, n->num - idx);

	bpnode_t *parent = n->parent;
	// root
	if (!parent) {
		if (!n->num) {
			t->root = NULL;
			free_node(n);
		}
		return val;
	}
	// at least half-full, just delete it
	if (n->num >= t->degree >> 1)
		return val;
	// otherwise,let it half-full

	// search sibling
	size_t lsibling = INVALID_INDEX, rsibling = INVALID_INDEX, curr;
	curr = search_child(n->keys[0], parent->keys, parent->num, t->vtbl->compare);
	if (curr)
		lsibling = curr - 1;
	if (curr < parent->num)
		rsibling = curr + 1;

	bpnode_t *sibling = NULL;
	// steal from sibling
	if (lsibling != INVALID_INDEX && parent->children[lsibling]->num > t->degree >> 1) {
		sibling = parent->children[lsibling];
		elemmove(n->keys + 1, n->keys, n->num);
		elemmove(n->vals + 1, n->vals, n->num);
		++n->num;
		size_t i = --sibling->num;
		n->keys[0] = sibling->keys[i];
		n->vals[0] = sibling->vals[i];
		parent->keys[lsibling] = n->keys[0];
	} else if (rsibling != INVALID_INDEX && parent->children[rsibling]->num > t->degree >> 1) {
		sibling = parent->children[rsibling];
		size_t i = n->num++;
		n->keys[i] = sibling->keys[0];
		n->vals[i] = sibling->vals[0];
		--sibling->num;
		elemmove(sibling->keys, sibling->keys + 1, sibling->num);
		elemmove(sibling->vals, sibling->vals + 1, sibling->num);
		parent->keys[curr] = sibling->keys[0];
	}
	if (sibling)
		return val;
	// merge with sibling
	// always merge to the left node
	if (lsibling != INVALID_INDEX) {
		sibling = parent->children[lsibling];
		elemmove(sibling->keys + sibling->num, n->keys, n->num);
		elemmove(sibling->vals + sibling->num, n->vals, n->num);
		sibling->num += n->num;
		delete_internal(t, parent, curr);
		sibling->next = n->next;
		free_node(n);
	} else if (rsibling != INVALID_INDEX) {
		sibling = parent->children[rsibling];
		elemmove(n->keys + n->num, sibling->keys, sibling->num);
		elemmove(n->vals + n->num, sibling->vals, sibling->num);
		n->num += sibling->num;
		delete_internal(t, sibling->parent, rsibling);
		n->next = sibling->next;
		free_node(sibling);
	}
	return val;
}

/**
 * delete a key-value pair
 * @param  t   
 * @param  key 
 * @return     value. should be cleaned by caller
 */
val_t bpt_delete(bptree_t *t, const_key_t key)
{
	assert_tree(t, NULL);
	bpnode_t *leaf = search_leaf(t, t->root, key);
	size_t i = search_key(key, leaf->keys, leaf->num, t->vtbl->compare);
	return i != INVALID_INDEX ? delete_leaf(t, leaf, i) : NULL;
}