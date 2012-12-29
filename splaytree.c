#include "linked_structures.h"
#include "tlsf/tlsf.h"
/*Adapted from ftp://ftp.cs.cmu.edu/usr/ftp/usr/sleator/splaying*/
splaytree header = {.left = NULL, .right = NULL, .data = NULL, .key = NULL};
static splaytree*
new_splaynode(void* key, void* data, BOOLEAN deep_copy, list_tspec* type) {
	deep_copy = deep_copy && type && type->deep_copy;
	splaytree init = {
		.right = NULL,
		.left = NULL,
		.key = key,
		.data = deep_copy?type->deep_copy(data):data
	};
	splaytree *n = tlsf_malloc(sizeof(splaytree));//TODO check if we run out of memory
	memcpy(n, &init, sizeof(*n));
	return n;
}

static long
memcomp(void *a, void* b){ return (a>b?1:(a<b?-1:0)); }

static splaytree*
splay(splaytree* root, void* key, list_tspec* type) {
	splaytree *l, *r, *t, *y;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	l = r = &header;
	t = root;
	header.left = header.right = NULL;
	for(;;){
		if(compar(key, t->data) < 0){
			if(t->left == NULL) break;
			if(compar(key, t->left->data) < 0){ //rotate right
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
		} else if(compar(key, t->data) > 0){
			if(t->right == NULL) break;
			if(compar(key, t->right->data) > 0){ //rotate left
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
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	if(root == NULL){
		return new_splaynode(key, data, copy, type);
	}
	root = splay(root, key, type);
	if((c = compar(key, root->key)) == 0){//disallow duplicate elements (for now)
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
splay_remove(splaytree* root, void** rtn, void* key, BOOLEAN destroy_data, list_tspec* type){
	splaytree* x;
	if(root == NULL) return root;
	destroy_data = destroy_data && type && type->destroy;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	root = splay(root, key, type);
	if(compar(key, root->data) == 0){// match found
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
			x = splay(root->left, key, type);
			x->right = root->right;
		}
		tlsf_free(root);
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
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	root = splay(root, key, type);
	if(compar(root->data, key) != 0){
		if(rtn) *rtn = NULL;
		return root;
	}
	if(rtn) *rtn = root->data;
	return root;
}

