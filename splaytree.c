#include "linked_structures.h"
#include "allocator.h"
#include "tlsf/tlsf.h"

/*Adapted from ftp://ftp.cs.cmu.edu/usr/ftp/usr/sleator/splaying*/
splaytree header = {.left = NULL, .right = NULL, .data = NULL};
static splaytree*
new_splaynode(aComparable* data, BOOLEAN deep_copy) {
	splaytree init = {
		.right = NULL,
		.left = NULL,
		.data = deep_copy?(aComparable*)data->method->parent.copy((anObject*)data):data
	};
	splaytree *n = (splaytree*)MALLOC(sizeof(splaytree));//TODO check if we run out of memory
	memcpy(n, &init, sizeof(*n));
	return n;
}

static splaytree*
splay(splaytree* root, aComparable* data) {
	splaytree *l, *r, *t, *y;
	l = r = &header;
	t = root;
	header.left = header.right = NULL;
	for(;;){
		if(data->method->compare(data, (anObject*)t->data) < 0){
			if(t->left == NULL) break;
			if(data->method->compare(data, (anObject*)t->left->data) < 0){ //rotate right
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
		} else if(data->method->compare(data, (anObject*)t->data) > 0){
			if(t->right == NULL) break;
			if(data->method->compare(data, (anObject*)t->right->data) > 0){ //rotate left
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
splay_insert(splaytree* root, aComparable* data, BOOLEAN copy){
	splaytree* n;
	long c;
	if(root == NULL) return new_splaynode(data, copy);

	root = splay(root, data);
	if((c = data->method->compare(data, (anObject*)root->data)) == 0) return root;//disallow duplicate elements (for now)
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
splay_remove(splaytree* root, aComparable* data, aComparable **rtn, BOOLEAN destroy_data){
	splaytree* x;
	if(root == NULL) return root;
	root = splay(root, data);
	if(data->method->compare(data, (anObject*)root->data) == 0){// match found
		if(rtn) *rtn = root->data;
		if(destroy_data){
			root->data->method->parent.destroy((anObject*)root->data);
			root->data = NULL;
		}
		if(root->left == NULL){
			x = root->right;
		} else {
			x = splay(root->left, data);
			x->right = root->right;
		}
		FREE(root);
		return x;
	}
	return root;
}

splaytree*
splay_find(splaytree* root, aComparable* data){
	if(root == NULL) return root;
	root = splay(root, data);
	if(data->method->compare(data, (anObject*)root->data) != 0) return root;
	return root;
}

