#include "linked_structures.h"
#include "allocator.h"
#include "tlsf/tlsf.h"

/*Adapted from ftp://ftp.cs.cmu.edu/usr/ftp/usr/sleator/splaying*/
splaytree header = {.left = NULL, .right = NULL, .data = NULL};
static splaytree*
new_splaynode(void* data, BOOLEAN deep_copy, list_tspec* type) {
	deep_copy = deep_copy && type && type->deep_copy;
	splaytree init = {
		.right = NULL,
		.left = NULL,
		.data = deep_copy?type->deep_copy(data):data
	};
	splaytree *n = MALLOC(sizeof(splaytree));//TODO check if we run out of memory
	memcpy(n, &init, sizeof(*n));
	return n;
}

static long
memcomp(void *a, void* b){ return (a>b?1:(a<b?-1:0)); }

static splaytree*
splay(splaytree* root, void* data, list_tspec* type) {
	splaytree *l, *r, *t, *y;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	l = r = &header;
	t = root;
	header.left = header.right = NULL;
	for(;;){
		if(compar(data, t->data) < 0){
			if(t->left == NULL) break;
			if(compar(data, t->left->data) < 0){ //rotate right
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
		} else if(compar(data, t->data) > 0){
			if(t->right == NULL) break;
			if(compar(data, t->right->data) > 0){ //rotate left
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
splay_insert(splaytree* root, void* data, BOOLEAN copy, list_tspec* type){
	splaytree* n;
	long c;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	if(root == NULL) return new_splaynode(data, copy, type);

	root = splay(root, data, type);
	if((c = compar(data, root->data)) == 0) return root;//disallow duplicate elements (for now)
	n = new_splaynode(data, copy, type);
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
splay_remove(splaytree* root, void* data, void** rtn, BOOLEAN destroy_data, list_tspec* type){
	splaytree* x;
	if(root == NULL) return root;
	destroy_data = destroy_data && type && type->destroy;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	root = splay(root, data, type);
	if(compar(data, root->data) == 0){// match found
		if(destroy_data){
			type->destroy(root->data);
			root->data = NULL;
		}
		if(rtn){
			*rtn = root->data;
		}
		if(root->left == NULL){
			x = root->right;
		} else {
			x = splay(root->left, data, type);
			x->right = root->right;
		}
		FREE(root);
		return x;
	}
	return root;
}

splaytree*
splay_find(splaytree* root, void* data, list_tspec* type){
	if(root == NULL) return root;

	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	root = splay(root, data, type);
	if(compar(root->data, data) != 0) return root;
	return root;
}

