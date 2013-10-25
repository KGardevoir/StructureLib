#include "splaytree.h"

/*Adapted from ftp://ftp.cs.cmu.edu/usr/ftp/usr/sleator/splaying*/
splaytree*
splaytree_new(const Comparable_vtable *data_method){
	return (splaytree*)memcpy(
		LINKED_MALLOC(sizeof(splaytree)),
		&(splaytree){
			.size = 0,
			.root = NULL,
			.data_method = data_method
		},
		sizeof(splaytree));
}

void
splaytree_destroy(splaytree *root){
	LINKED_FREE(root);
}


static btree*
new_splaynode(Object* data, BOOLEAN deep_copy) {
	return memcpy(LINKED_MALLOC(sizeof(btree)), &(btree){
		.right = NULL,
		.left = NULL,
		.data = deep_copy?CALL(data,copy,data, LINKED_MALLOC(data->method->size)):data
	}, sizeof(btree));
}

static btree*
splay(btree* root, Object* data, __typeof__(((splaytree*)NULL)->data_method) data_method){
	btree *l, *r, *t, *y;
	btree header = {.left = NULL, .right = NULL, .data = NULL};
	if(!root) return root;
	l = r = &header;
	t = root;
	for(;;){
		if(data_method->compare(data, (Object*)t->data) < 0){
			if(t->left == NULL) break;
			if(data_method->compare(data, (Object*)t->left->data) < 0){ //rotate right
				y = t->left;
				t->left = y->right;
				y->right = t;
				//t->parent = y;
				t = y;
				if(t->left == NULL) break;
			}
			r->left = t; //link right
			r = t;
			t = t->left;
		} else if(data_method->compare(data, (Object*)t->data) > 0){
			if(t->right == NULL) break;
			if(data_method->compare(data, (Object*)t->right->data) > 0){ //rotate left
				y = t->right;
				t->right = y->left;
				//if(t->right != NULL) t->right->parent = t;
				y->left = t;
				//t->parent = y;
				t = y;
				if(t->right == NULL) break;
			}
			l->right = t; //link left
			//t->parent = Binary Search Tree;
			l = t;
			t = t->right;
		} else {
			break;
		}
	}
	l->right = t->left;
	//if(l->right != NULL) l->right->parent = l;
	r->left = t->right;
	//if(r->left != NULL) r->left->parent = r;
	t->left = header.right;
	//if(t->left != NULL) t->left->parent = t;
	t->right = header.left;
	//if(t->right != NULL) t->right->parent = t;
	//t->parent = NULL;
	return t;
}

BOOLEAN
splay_insert(splaytree* root, Object* data, BOOLEAN copy){
	btree* n;
	long c;
	if(!root) return FALSE;

	if(!root->root){
		root->root = new_splaynode(data, copy);
		root->size++;
		return TRUE;
	} else {
		root->root = splay(root->root, data, root->data_method);
	}
	if((c = root->data_method->compare(data, (Object*)root->root->data)) == 0) return FALSE;//disallow duplicate elements (for now)
	n = new_splaynode(data, copy);
	if(c < 0){
		n->left = root->root->left;
		n->right = root->root;
		root->root->left = NULL;
	} else {
		n->right = root->root->right;
		n->left = root->root;
		root->root->right = NULL;
	}
	root->size++;
	root->root = n;
	return TRUE;
}

Object*
splay_remove(splaytree* root, Object *key, const Comparable_vtable *key_method, BOOLEAN destroy_data){
	if(!root || !root->root) return NULL;
	root->root = splay(root->root, key, key_method);
	Object *rtn = NULL;
	if(key_method->compare(key, (Object*)root->root->data) == 0){// match found
		btree *del = root->root;
		rtn = del->data;
		if(destroy_data){
			CALL_VOID(rtn,destroy);
		}
		if(!del->left){
			root->root = del->right;
		} else {
			btree* x = del->right;
			root->root = splay(del->left, key, key_method);
			root->root->right = x;
		}
		LINKED_FREE(del);
		root->size--;
	}
	return rtn;
}

Object*
splay_find(splaytree* root, Object* key, const Comparable_vtable *key_method){
	if(!root || !root->root) return NULL;
	root->root = splay(root->root, key, key_method);
	if(key_method->compare(key, (Object*)root->root->data) == 0) return root->root->data;
	return NULL;
}

void
splay_clear(splaytree *root, BOOLEAN destroy){
	btree_clear(root->root, destroy);
	root->root = NULL;
	root->size = 0;
}
