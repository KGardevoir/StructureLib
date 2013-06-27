#include "splaytree.h"

/*Adapted from ftp://ftp.cs.cmu.edu/usr/ftp/usr/sleator/splaying*/
splaytree header = {.left = NULL, .right = NULL, .data = NULL};
static splaytree*
new_splaynode(Object* data, BOOLEAN deep_copy) {
	splaytree init = {
		.right = NULL,
		.left = NULL,
		.data = deep_copy?CALL(data,copy,(data, LINKED_MALLOC(data->method->size)), data):data
	};
	return memcpy(LINKED_MALLOC(sizeof(splaytree)), &init, sizeof(splaytree));
}

static splaytree*
splay(splaytree* root, Object* data, const Comparable_vtable* data_method) {
	splaytree *l, *r, *t, *y;
	l = r = &header;
	t = root;
	header.left = header.right = NULL;
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
			//t->parent = lBinary Search Tree;
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

splaytree*
splay_insert(splaytree* root, Object* data, const Comparable_vtable* method, BOOLEAN copy){
	splaytree* n;
	long c;
	if(root == NULL) return new_splaynode(data, copy);

	root = splay(root, data, method);
	if((c = method->compare(data, (Object*)root->data)) == 0) return root;//disallow duplicate elements (for now)
	n = new_splaynode(data, copy);
	if(c < 0){
		n->left = root->left;
		n->right = root;
		root->left = NULL;
	} else {
		n->right = root->right;
		n->left = root;
		root->right = NULL;
	}
	return n;
}

splaytree*
splay_remove(splaytree* root, Object *data, const Comparable_vtable *data_method, Object **rtn, BOOLEAN destroy_data){
	splaytree* x;
	if(root == NULL) return root;
	root = splay(root, data, data_method);
	if(data_method->compare(data, (Object*)root->data) == 0){// match found
		if(rtn) *rtn = root->data;
		if(destroy_data){
			CALL_VOID(root->data,destroy,(root->data));
			root->data = NULL;
		}
		if(root->left == NULL){
			x = root->right;
		} else {
			x = splay(root->left, data, data_method);
			x->right = root->right;
		}
		LINKED_FREE(root);
		return x;
	}
	return root;
}

splaytree*
splay_find(splaytree* root, Object* data, const Comparable_vtable* data_method){
	if(root == NULL) return root;
	root = splay(root, data, data_method);
	if(data_method->compare(data, (Object*)root->data) != 0) return root;
	return root;
}

