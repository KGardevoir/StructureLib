#ifndef _LINKED_STRUCTURES_SPLAY_TREE_H_
#define _LINKED_STRUCTURES_SPLAY_TREE_H_
#include "linked_base.h"
#include "btree.h"

typedef btree splaytree;
//Splay Trees
splaytree* splay_insert(splaytree* root, Object* data, const Comparable_vtable *data_method, BOOLEAN copy) __attribute__((warn_unused_result));
splaytree* splay_remove(splaytree* root, Object* data, const Comparable_vtable *data_method, Object **rtn, BOOLEAN destroy_data) __attribute__((warn_unused_result));
splaytree* splay_find(splaytree* root, Object* key, const Comparable_vtable *key_method) __attribute__((warn_unused_result));

#endif
