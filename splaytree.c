#include "linked_structures.h"
/*Adapted from ftp://ftp.cs.cmu.edu/usr/ftp/usr/sleator/splaying*/
splaytree header = {.left = NULL, .right = NULL, .data = NULL, .key = NULL};
static splaytree*
new_splaynode(void* key, void* data, BOOLEAN deep_copy, list_tspec* type) {
	splaytree init = {
		.right = NULL,
		.left = NULL,
		.key = key,
		.data = deep_copy?type->deep_copy(data):data
	};
	splaytree *n = type->adalloc(sizeof(splaytree));//TODO check if we run out of memory
	memcpy(n, &init, sizeof(*n));
	return n;
}
static splaytree*
splay(splaytree* root, void* key, list_tspec* type) {
	splaytree *l, *r, *t, *y;
	l = r = &header;
	t = root;
	header.left = header.right = NULL;
	for(;;){
		if(type->compar(key, t->data) < 0){
			if(t->left == NULL) break;
			if(type->compar(key, t->left->data) < 0){ //rotate right
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
		} else if(type->compar(key, t->data) > 0){
			if(t->right == NULL) break;
			if(type->compar(key, t->right->data) > 0){ //rotate left
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
splay_insert(splaytree* root, void* key, void* data, BOOLEAN copy, list_tspec* type){
	splaytree* n;
	long c;
	if(root == NULL){
		return new_splaynode(key, data, copy, type);
	}
	root = splay(root, key, type);
	if((c = type->compar(key, root->key)) == 0){//disallow duplicate elements (for now)
		return root;
	}
	n = new_splaynode(key, data, copy, type);
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
splay_remove(splaytree* root, void** rtn, void* key, BOOLEAN free_data, list_tspec* type){
	splaytree* x;
	if(root == NULL)
		return root;
	root = splay(root, key, type);
	if(type->compar(key, root->data) == 0){// match found
		if(free_data){
			type->adfree(root->data);
			root->data = NULL;
		}
		if(rtn){
			*rtn = root->data;
		}
		if(root->left == NULL){
			x = root->right;
		} else {
			x = splay(root->left, key, type);
			x->right = root->right;
		}
		type->adfree(root);
		return x;
	}
	return root;
}

splaytree*
splay_findmin(splaytree* root, void** rtn, list_tspec* type){
	splaytree* x = bstree_findmin(root);
	if(x == NULL){
		*rtn = NULL;
		return root;
	}
	if(rtn) *rtn = x->data;
	root = splay(root, x->data, type);
	return root;
}

splaytree*
splay_findmax(splaytree* root, void** rtn, list_tspec* type){
	splaytree* x = bstree_findmax(root);
	if(root == NULL){
		*rtn = NULL;
		return root;
	}
	if(rtn) *rtn = x->data;
	root = splay(root, x->data, type);
	return root;
}

splaytree*
splay_find(splaytree* root, void** rtn, void* key, list_tspec* type){
	if(root == NULL){
		if(rtn) *rtn = NULL;
		return root;
	}
	root = splay(root, key, type);
	if(type->compar(root->data, key) != 0){
		if(rtn) *rtn = NULL;
		return root;
	}
	if(rtn) *rtn = root->data;
	return root;
}

