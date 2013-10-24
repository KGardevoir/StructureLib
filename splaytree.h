#ifndef _LINKED_STRUCTURES_SPLAY_TREE_H_
#define _LINKED_STRUCTURES_SPLAY_TREE_H_
#include "linked_base.h"
#include "btree.h"

typedef struct splaytree {
	size_t size;
	const Comparable_vtable *data_method;
	btree *root;
} splaytree;

//Splay Trees
splaytree* splaytree_new(const Comparable_vtable *data_method) __attribute__((warn_unused_result));
void splaytree_destroy(splaytree*);
BOOLEAN splay_insert(splaytree* root, Object* data, BOOLEAN copy);
Object* splay_remove(splaytree* root, Object* key, const Comparable_vtable *key_method, BOOLEAN destroy_data);
Object* splay_find(splaytree* root, Object* key, const Comparable_vtable *key_method);

void splay_clear(splaytree *root, BOOLEAN destroy);

#endif
