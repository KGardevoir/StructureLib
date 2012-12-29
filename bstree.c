#include "linked_structures.h"
#include "tlsf/tlsf.h"
/* Algorithms adapted from Ch.12 of Introduction To Algorithms by Thomas H. Cormen, Charles E. Leiserson, Ronald L.
 * Rivest, Clifford Stein */
static long
memcomp(void *a, void* b){ return (a>b?1:(a<b?-1:0)); }

static bstree*
new_bsnode(void* key, void* data, BOOLEAN deep_copy, list_tspec*type){
	deep_copy = deep_copy && type && type->deep_copy;
	bstree init = {
		.left = NULL,
		.right = NULL,
		.key = key,
		.data = deep_copy?type->deep_copy(data):data
	};
	bstree *n = tlsf_malloc(sizeof(bstree));
	memcpy(n, &init, sizeof(*n));
	return n;
}

bstree*
bstree_insert(bstree *root, void* key, void* data, BOOLEAN copy, list_tspec* type){
	bstree *p = bstree_parent(root, key, type);
	bstree *new = new_bsnode(key, data, copy, type);
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	if(p == NULL){//tree was NULL
		root = new;
	} else if(compar(key, p->data) < 0) {
		p->left = new;
	} else if(compar(key, p->data) > 0) {
		p->right = new;
	} else {//are equal, ignore for now (no duplicates)

	}
	return root;
}

static bstree*
bstree_transplant(bstree *root, bstree *u, bstree *v, bstree *up, bstree *vp){
	if(up == NULL){
		return v;
	} else if(u == up->left){
		up->left = v;
	} else {
		up->right = v;
	}
	return root;
}

bstree*
bstree_remove(bstree *root, void* key, void** rtn, BOOLEAN destroy_data, list_tspec* type){
	bstree *node = bstree_find(root, key, type);
	if(node == NULL) return NULL;//nothing to return, no such element
	destroy_data = destroy_data && type && type->destroy;
	bstree *nodep = bstree_parent(root, key, type);
	if(node->left == NULL){
		root = bstree_transplant(root, node, node->right, nodep, node);
	} else if(node->right == NULL){
		root = bstree_transplant(root, node, node->left, nodep, node);
	} else {
		bstree* min = bstree_findmin(node->right);
		bstree* par = bstree_parent(root, min->data, type);
		if(par != node){
			root = bstree_transplant(root, min, min->right, par, min);
			min->right = node->right;
		}
		root = bstree_transplant(root, node, min, nodep, par);
		min->left = node->left;
	}
	void* data = node->data;
	tlsf_free(node);
	if(destroy_data){
		type->destroy(data);
		data = NULL;
	}
	if(rtn) *rtn = data;
	return root;
}

bstree*
bstree_find(bstree *root, void *key, list_tspec* type){
	long c;
	if(root == NULL) return root;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	while(root != NULL && (c = compar(key, root->data)) != 0){
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return root;
}

bstree*
bstree_parent(bstree *root, void *data, list_tspec* type){
	long c;
	bstree *parent = root;
	if(root == NULL) return root;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	while(root != NULL && (c = compar(data, root->data)) != 0){
		parent = root;
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return parent;
}

dlist* /* with type bstree*/
bstree_path(bstree *root, void *data, list_tspec* type){
	dlist *head = NULL;
	long c;
	if(root == NULL) return NULL;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	while(root != NULL && (c = compar(data, root->data)) != 0){
		head = dlist_append(head, root, FALSE, type);
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return dlist_append(head, root, FALSE, type);
}

bstree*
bstree_predessor(bstree* root, bstree* node, list_tspec* type){
	if(root == NULL || node == NULL) return NULL;
	if(node->left != NULL) return bstree_findmax(node->left);
	dlist* parent = bstree_path(root, node->data, type);//use bstree_path instead
	if(parent == NULL) return root;
	dlist* head = parent;
	parent = parent->prev;
	while(parent->prev->data == node && parent->data != root){
		node = parent->data;
		parent = parent->prev;
	}
	dlist_clear(head, FALSE, type);
	return node;
}

bstree*
bstree_successor(bstree* root, bstree* node, list_tspec* type){
	if(root == NULL || node == NULL) return NULL;
	if(node->right != NULL) return bstree_findmin(node->right);
	dlist* parent = bstree_path(root, node->data, type);//use bstree_path instead
	if(parent == NULL) return root;
	dlist* head = parent;
	parent = parent->prev;
	while(parent->prev->data == node && parent->data != root){
		node = parent->data;
		parent = parent->prev;
	}
	dlist_clear(head, FALSE, type);
	return node;
}

bstree*
bstree_findmin(bstree* root){
	if(root == NULL) return root;
	while(root->left != NULL) root = root->left;
	return root;
}

bstree*
bstree_findmax(bstree* root){
	if(root == NULL) return root;
	while(root->right != NULL) root = root->right;
	return root;
}

struct free_cluster {
	const BOOLEAN destroy_data;
	const list_tspec const* type;
};

static BOOLEAN
bstree_clear_map(bstree* root, struct free_cluster *aux){
	if(aux->destroy_data) aux->type->destroy(root->data);
	tlsf_free(root);
	return TRUE;
}

void
bstree_clear(bstree* root, BOOLEAN destroy_data, list_tspec* type){//iterate in postfix order
	dlist *stk = NULL, *freer = NULL;
	bstree *cur = root;
	while(stk != NULL || cur != NULL){
		if(cur){
			stk = dlist_push(stk, cur, FALSE, type);
			cur = cur->left;
		} else {
			cur = stk->data;
			stk = dlist_pop(stk, NULL, FALSE, type);
			freer = dlist_append(freer, cur, FALSE, type);
			cur = cur->right;
		}
	}
	struct free_cluster aux = {.destroy_data = destroy_data, .type = type};
	dlist_map(freer, &aux, (lMapFunc)bstree_clear_map);
	dlist_clear(freer, FALSE, type);
}

BOOLEAN
bstree_map(bstree* root, void* aux, lMapFunc func, list_tspec* type){
	//perform a depth first search (in-order traversal)
	dlist *stk = NULL;
	bstree *cur = root;
	while(stk != NULL || cur != NULL){
		if(cur){
			stk = dlist_push(stk, cur, FALSE, type);
			cur = cur->left;
		} else {
			cur = stk->data;
			stk = dlist_pop(stk, NULL, FALSE, type);
			if(!func(cur->data, aux)) return FALSE;
			cur = cur->right;
		}
	}
	return TRUE;
}
