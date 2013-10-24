#ifndef _LINKED_STRUCTURES_BSTREE_H_
#define _LINKED_STRUCTURES_BSTREE_H_
#include "linked_base.h"
#include "dlist.h"

typedef struct btree {
	struct btree *right;
	Object* data;
	struct btree *left;
} btree;

//Binary Search Tree, a threaded tree would be nice, but determining leaf nodes is SIGNIFICANTLY more difficult
btree* btree_insert(btree* root, Object* data, const Comparable_vtable *data_method, BOOLEAN copy, BOOLEAN *success);
btree* btree_insert_with_depth(btree *root, Object *data, const Comparable_vtable* data_method, size_t *r_depth, BOOLEAN copy, BOOLEAN *success);
btree* btree_remove(btree* root, Object* key, const Comparable_vtable *key_method, Object** rtn, BOOLEAN destroy_data) __attribute__((warn_unused_result));

void    btree_clear(btree* root, BOOLEAN destroy_data);
btree*  btree_find(btree *root, Object* data, const Comparable_vtable *data_method);
btree*  btree_find_with_depth(btree *root, Object *data, const Comparable_vtable *data_method, size_t *r_depth);
btree*  btree_parent(btree *root, Object* data, const Comparable_vtable *data_method);
btree*  btree_parent_with_depth(btree *root, Object* data, const Comparable_vtable *data_method, size_t *r_depth);
dlist*  btree_path(btree *root, Object* data, const Comparable_vtable *data_method) __attribute__((warn_unused_result));

btree* btree_predessor(btree *root, btree *node, const Comparable_vtable *data_method);
btree* btree_successor(btree *root, btree *node, const Comparable_vtable *data_method);
btree* btree_findmin(btree *root);
btree* btree_findmax(btree *root);

void   btree_info(btree *root, size_t *min, size_t *max, size_t *avg, size_t *leaves, size_t *size, dlist **lleaves, dlist **lnodes);

BOOLEAN btree_map(btree *root, const TRAVERSAL_STRATEGY, const BOOLEAN more_info, void* aux, const lMapFunc func);
btree* btree_balance(btree *root);

static inline btree* btree_sibling(btree* root, btree *sibling){ btree *side = root->left; if(side == sibling) return root->right; return side; }

#endif
