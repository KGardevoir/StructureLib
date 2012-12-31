#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

/* Algorithms adapted from Ch.12 of Introduction To Algorithms by Thomas H. Cormen, Charles E. Leiserson, Ronald L.
 * Rivest, Clifford Stein */
static long
memcomp(void *a, void* b){ return (a>b?1:(a<b?-1:0)); }

static bstree*
new_bsnode(void* data, BOOLEAN deep_copy, list_tspec*type){
	deep_copy = deep_copy && type && type->deep_copy;
	bstree init = {
		.left = NULL,
		.right = NULL,
		.data = deep_copy?type->deep_copy(data):data
	};
	bstree *n = MALLOC(sizeof(bstree));
	memcpy(n, &init, sizeof(*n));
	return n;
}

bstree*
bstree_insert(bstree *root, void* data, BOOLEAN copy, list_tspec* type){
	bstree *p = bstree_parent(root, data, type);
	bstree *new = new_bsnode(data, copy, type);
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	if(p == NULL){//tree was NULL
		root = new;
	} else if(compar(data, p->data) < 0) {
		p->left = new;
	} else if(compar(data, p->data) > 0) {
		p->right = new;
	} else {//are equal, ignore for now (no duplicates)
		FREE(new);
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
bstree_remove(bstree *root, void* data, void** rtn, BOOLEAN destroy_data, list_tspec* type){
	bstree *node = bstree_find(root, data, type);
	if(node == NULL) return NULL;//nothing to return, no such element
	destroy_data = destroy_data && type && type->destroy;
	bstree *nodep = bstree_parent(root, data, type);
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
	data = node->data;
	FREE(node);
	if(destroy_data){
		type->destroy(data);
		data = NULL;
	}
	if(rtn) *rtn = data;
	return root;
}

bstree*
bstree_find(bstree *root, void *data, list_tspec* type){
	long c;
	if(root == NULL) return root;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	while(root != NULL && (c = compar(data, root->data)) != 0){
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return root;
}
bstree*
bstree_find_via_key(bstree *root, const void const *key, list_tspec* type){
	long c;
	if(root == NULL) return root;
	lKeyCompare key_compar = (type && type->key_compar)?type->key_compar:(lKeyCompare)memcomp;
	while(root != NULL && (c = key_compar(key, root->data)) != 0){
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
		head = dlist_append(head, root, FALSE, NULL);
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return (root==NULL)?head:dlist_append(head, root, FALSE, NULL);
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
	dlist_clear(head, FALSE, NULL);
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
	FREE(root);
	return TRUE;
}

void
bstree_clear(bstree* root, BOOLEAN destroy_data, list_tspec* type){//iterate in postfix order
	dlist *stk = NULL, *freer = NULL;
	bstree *cur = root;
	while(stk != NULL || cur != NULL){
		if(cur){
			stk = dlist_push(stk, cur, FALSE, NULL);
			cur = cur->left;
		} else {
			stk = dlist_pop(stk, (void**)&cur, FALSE, NULL);
			freer = dlist_append(freer, cur, FALSE, NULL);
			cur = cur->right;
		}
	}
	struct free_cluster aux = {.destroy_data = destroy_data, .type = type};
	dlist_map(freer, &aux, (lMapFunc)bstree_clear_map);
	dlist_clear(freer, FALSE, NULL);
}

typedef BOOLEAN (*TMapFunc)(bstree* node, void* aux);

#if 1
static BOOLEAN
bstree_map_internal(bstree *root, const TRAVERSAL_STRATEGY strat, void* aux, TMapFunc func){
	if(strat == DEPTH_FIRST_PRE || strat == DEPTH_FIRST_IN){
		dlist *stk = NULL;
		bstree *cur = root;
		while(stk || cur){
			if(cur){
				stk = dlist_push(stk, cur, FALSE, NULL);
				cur = cur->left;
			} else {
				if(strat == DEPTH_FIRST_IN || strat == DEPTH_FIRST_PRE){
					if(strat == DEPTH_FIRST_IN)
						stk = dlist_dequeue(stk, (void**)&cur, FALSE, NULL);
					else
						stk = dlist_pop(stk, (void**)&cur, FALSE, NULL);
					//by changing this from a stack to a queue, this can be transformed to a pre-order
					//traversal
					if(!func(cur, aux)) return FALSE;
				}
				cur = cur->right;
			}
		}
	} else if( strat == DEPTH_FIRST_POST){
		if(!root) return TRUE;
		dlist *stk = dlist_push(NULL, root, FALSE, NULL);
		bstree *prev = NULL;
		while(stk){
			bstree *cur = stk->data;
			if(!prev || prev->left == cur || prev->right == cur){
				if(cur->left)
					stk = dlist_push(stk, cur->left, FALSE, NULL);
				else if(cur->right)
					stk = dlist_push(stk, cur->right, FALSE, NULL);
			} else if(cur->left == prev){
				if(cur->right)
					stk = dlist_push(stk, cur->right, FALSE, NULL);
			} else {
				if(!func(cur, aux)) return FALSE;
				stk = dlist_dequeue(stk, NULL, FALSE, NULL);
			}
			prev = cur;
		}
	} else { //Breadth First
		dlist* q = dlist_append(NULL, root, FALSE, NULL);
		while(q){
			bstree *node;
			q = dlist_dequeue(q, (void**)&node, FALSE, NULL);
			if(!func(node, aux)) return FALSE;
			if(node->left){
				q = dlist_append(q, node->left, FALSE, NULL);
			}
			if(node->right){
				q = dlist_append(q, node->right, FALSE, NULL);
			}
		}
	}
	return TRUE;
}
#else
static BOOLEAN
bstree_map_internal(bstree *root, const TRAVERSAL_STRATEGY strat, void* aux, TMapFunc func){
	if(!root) return TRUE;
	if(!bstree_map_internal(root->left , strat, aux, func)) return FALSE;
	if(!bstree_map_internal(root->right, strat, aux, func)) return FALSE;
	if(!func(root, aux)) return FALSE;
	return TRUE;
}
#endif

struct bstree_map_d {
	void *aux;
	lMapFunc func;
};

static BOOLEAN
bstree_map_f(bstree* root, struct bstree_map_d* aux){
	return aux->func(root->data, aux->aux);
}

BOOLEAN
bstree_map(bstree* root, const TRAVERSAL_STRATEGY strat, void* aux, lMapFunc func){
	struct bstree_map_d dat = {
		.aux = aux,
		.func = func
	};
	return bstree_map_internal(root, strat, &dat, (TMapFunc)bstree_map_f);
}

static BOOLEAN
bstree_leaves_map_f(bstree *root, dlist **leaves){
	if(root->left == NULL && root->right == NULL)
		*leaves = dlist_append(*leaves, root, FALSE, NULL);
	return TRUE;
}

dlist*
bstree_leaves(bstree *root){
	dlist *head = NULL;
	bstree_map_internal(root, DEPTH_FIRST_IN, &head, (TMapFunc)bstree_leaves_map_f);
	return head;
}

struct height {
	bstree *root;
	list_tspec *type;
	size_t min, max, avg;
	BOOLEAN init;
};

#define MAX(a,b) ({ typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ typeof(a) _a = (a), _b = (b); _a < _b ? _a : _b; })

static BOOLEAN
bstree_height_map(bstree *leaf, struct height *h){
	dlist *path = bstree_path(h->root, leaf->data, h->type);
	size_t len = dlist_length(path)-1;
	if(h->init){
		h->min = MIN(h->min, len);
		h->max = MAX(h->max, len);
		h->avg += len;
	} else {
		h->min = len;
		h->max = len;
		h->avg = len;
		h->init = TRUE;
	}
	dlist_clear(path, FALSE, NULL);
	return TRUE;
}

void
bstree_height(bstree *root, size_t *min, size_t *max, size_t *avg, list_tspec* type){
	dlist* leaves = bstree_leaves(root);
	struct height h = {
		.root = root,
		.type = type,
		.init = FALSE
	};
	dlist_map(leaves, &h, (lMapFunc)bstree_height_map);
	printf("Leaves: %lu\n", dlist_length(leaves));
	*min = h.min;
	*max = h.max;
	*avg = (size_t)(h.avg/dlist_length(leaves));
	dlist_clear(leaves, FALSE, NULL);
}

static BOOLEAN
bstree_size_map_f(void* data, size_t* size){
	*size = *size + 1;
	return TRUE;
}

size_t
bstree_size(bstree *root){
	size_t size = 1;
	bstree_map(root, DEPTH_FIRST_IN, &size, (lMapFunc)bstree_size_map_f);
	return size;
}

struct bstree_dump_map_d {
	bstree *root;
	list_tspec *type;
};

#if 0
static BOOLEAN
bstree_dump_map_f(bstree* node, struct bstree_dump_map_d *aux){
	dlist *path = bstree_path(aux->root, node->data, aux->type);
	size_t len = dlist_length(path)-1;
	printf("%*s%-4p: %lu <%p, %p>\n", (int)len, "", node->data, len, node->left, node->right);
	dlist_clear(path, FALSE, NULL);
	return TRUE;
}

void
print_bstree_structure(bstree *root, list_tspec *type){
	struct bstree_dump_map_d dat = {
		.root = root,
		.type = type
	};
	bstree_map_internal(root, DEPTH_FIRST_IN, &dat, (TMapFunc)bstree_dump_map_f);
}
#endif
