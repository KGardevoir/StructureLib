#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

/* Algorithms adapted from Ch.12 of Introduction To Algorithms by Thomas H. Cormen, Charles E. Leiserson, Ronald L.
 * Rivest, Clifford Stein */
static long
memcomp(void *a, void* b, list_tspec* type){ return (a>b?1:(a<b?-1:0)); }

static bstree*
new_bsnode(void* data, BOOLEAN deep_copy, list_tspec*type){
	deep_copy = deep_copy && type && type->deep_copy;
	bstree init = {
		.left = NULL,
		.right = NULL,
		.data = deep_copy?type->deep_copy(data, type):data
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
	} else if(compar(data, p->data, type) < 0) {
		p->left = new;
	} else if(compar(data, p->data, type) > 0) {
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
		type->destroy(data, type);
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
	while(root != NULL && (c = compar(data, root->data, type)) != 0){
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
	while(root != NULL && (c = key_compar(key, root->data, type)) != 0){
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
	while(root != NULL && (c = compar(data, root->data, type)) != 0){
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
	while(root != NULL && (c = compar(data, root->data, type)) != 0){
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
	if(aux->destroy_data) aux->type->destroy(root->data, NULL);
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
	dlist_map(freer, FALSE, &aux, (lMapFunc)bstree_clear_map);
	dlist_clear(freer, FALSE, NULL);
}

#if 1
struct node_and_depth {
	bstree *node;
	size_t depth;
};
static struct node_and_depth*
new_node_and_depth(size_t depth, bstree *node){
	struct node_and_depth init = {
		.node = node,
		.depth = depth
	};
	struct node_and_depth *new = malloc(sizeof(typeof(init)));
	memcpy(new, &init, sizeof(typeof(init)));
	return new;
}

static inline BOOLEAN
bstree_map_pre_in_internal(bstree *root, BOOLEAN pass_data, BOOLEAN more_info, BOOLEAN IN_ORDER, void* aux, lMapFunc func){
	dlist *stk = NULL;
	size_t depth = 0, position = 0, size = 0;
	if(more_info) bstree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	bstree *cur = root;
	while(stk || cur){
		if(cur){
			depth++;
			stk = dlist_append(stk, new_node_and_depth(depth, cur), FALSE, NULL);
			if(!IN_ORDER){
				if(more_info){
					lMapFuncAux ax = {
						.isAux = TRUE,
						.depth = depth,
						.position = position,
						.size = size,
						.aux = aux
					};
					if(!func(pass_data?cur->data:cur, &ax)) return FALSE;
					position++;
				} else {
					if(!func(pass_data?cur->data:cur, aux)) return FALSE;
				}
			}
			cur = cur->left;
		} else {
			struct node_and_depth *node = NULL;
			stk = dlist_pop(stk, (void**)&node, FALSE, NULL);
			cur = node->node;
			depth = node->depth;
			free(node);
			//by changing this from a stack to a queue, this can be transformed to a pre-order
			//traversal
			if(IN_ORDER){
				if(more_info){
					lMapFuncAux ax = {
						.isAux = TRUE,
						.depth = depth,
						.position = position,
						.size = size,
						.aux = aux
					};
					if(!func(pass_data?cur->data:cur, &ax)) return FALSE;
					position++;
				} else {
					if(!func(pass_data?cur->data:cur, aux)) return FALSE;
				}
			}
			cur = cur->right;
		}
	}
	return TRUE;
}
static inline BOOLEAN
bstree_map_post_internal(bstree *root, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!root) return TRUE;
	size_t depth = 0, position = 0, size = 0;
	if(more_info){
		bstree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	}
	dlist *stk = dlist_push(NULL, new_node_and_depth(depth, root), FALSE, NULL);
	bstree *prev = NULL;
	while(stk){
		struct node_and_depth* node = stk->data;
		bstree *cur = node->node;
		depth = node->depth;
		if(!prev || prev->left == cur || prev->right == cur){
			if(cur->left || cur->right){
				stk = dlist_push(stk, new_node_and_depth(depth+1, cur->left?cur->left:cur->right), FALSE, NULL);
			}
		} else if(cur->left == prev){
			if(cur->right)
				stk = dlist_push(stk, new_node_and_depth(depth+1, cur->right), FALSE, NULL);
		} else {
			if(more_info){
				lMapFuncAux ax = {
					.isAux = TRUE,
					.depth = depth,
					.position = position,
					.size = size,
					.aux = aux
				};
				if(!func(pass_data?cur->data:cur, &ax)) return FALSE;
				position++;
			} else {
				if(!func(pass_data?cur->data:cur, aux)) return FALSE;
			}
			stk = dlist_dequeue(stk, (void**)&node, FALSE, NULL);
			free(node);
		}
		prev = cur;
	}
	return TRUE;
}
static inline BOOLEAN
bstree_map_breadth_internal(bstree *root, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	dlist* q = dlist_append(NULL, new_node_and_depth(0, root), FALSE, NULL);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) bstree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	bstree *prev = root;
	while(q){
		bstree *node;
		q = dlist_dequeue(q, (void**)&node, FALSE, NULL);
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = depth,
				.position = position,
				.size = size,
				.aux = aux
			};
			if(!func(pass_data?node->data:node, &ax)) return FALSE;
			position++;
		} else {
			if(!func(pass_data?node->data:node, aux)) return FALSE;
		}
		if(prev && (prev->left == node || prev->right == node)){//new level
			depth++;
			prev = NULL;
		}
		if(!prev)
			prev = node->left?node->left:node->right;
		if(node->left)
			q = dlist_append(q, node->left, FALSE, NULL);
		if(node->right)
			q = dlist_append(q, node->right, FALSE, NULL);
	}
	return TRUE;
}

static inline BOOLEAN
bstree_map_internal(bstree *root, const TRAVERSAL_STRATEGY strat, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!root) return TRUE;
	if(strat == DEPTH_FIRST_PRE || strat == DEPTH_FIRST_IN){
		return bstree_map_pre_in_internal(root, pass_data, more_info, strat == DEPTH_FIRST_IN, aux, func);
	} else if( strat == DEPTH_FIRST_POST){
		return bstree_map_post_internal(root, pass_data, more_info, aux, func);
	} else { //Breadth First
		return bstree_map_breadth_internal(root, pass_data, more_info, aux, func);
	}
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

BOOLEAN
bstree_map(bstree* root, const TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc func){
	return bstree_map_internal(root, strat, TRUE, more_info, aux, func);
}

#define MAX(a,b) ({ typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ typeof(a) _a = (a), _b = (b); _a < _b ? _a : _b; })

void
bstree_info(bstree *root, size_t *min, size_t *max, size_t *avg, size_t *num_leaves, size_t *size, dlist** rleaves, dlist** rnodes){
	//dlist* leaves = bstree_leaves(root);
	dlist *leaves = NULL, *nodes = NULL;
	size_t tmin = 0, tmax = 0, tavg = root?1:0, tleaves = root?1:0, tsize = root?1:0;
	BOOLEAN init = FALSE;
	dlist *stk = NULL;
	size_t depth = 0;
	bstree *cur = root;
	while(stk || cur){
		if(cur){
			stk = dlist_append(stk, cur, FALSE, NULL);
			if(rnodes) nodes = dlist_append(nodes, cur, FALSE, NULL);
			cur = cur->left;
			depth++;
			tsize++;
		} else {
			stk = dlist_dequeue(stk, (void**)&cur, FALSE, NULL);
			depth--;
			if(cur->left == NULL && cur->right == NULL){
				tleaves++;
				if(rleaves)
					leaves = dlist_append(leaves, cur, FALSE, NULL);
			}
			if(init){
				tmin = MIN(tmin, depth);
				tmax = MAX(tmax, depth);
			} else {
				tmin = depth;
				tmax = depth;
				init = TRUE;
			}
			tavg = tavg + depth;
			cur = cur->right;
		}
	}
	//printf("Leaves: %lu\n", dlist_length(leaves));
	if(min) *min = tmin;
	if(max) *max = tmax;
	if(avg) *avg = (size_t)(tavg/tsize);
	if(num_leaves) *num_leaves = tleaves;
	if(size) *size = tsize;
	if(rleaves) *rleaves = leaves;
	if(rnodes)  *rnodes = nodes;
}


#if 0
static BOOLEAN
bstree_dump_map_f(bstree* node, size_t level, struct bstree_dump_map_d *aux){
	printf("%*s%-4p: %lu <%p, %p>\n", (int)level, "", node->data, level, node->left, node->right);
	return TRUE;
}

void
print_bstree_structure(bstree *root, list_tspec *type){
	bstree_map_internal(root, DEPTH_FIRST_PRE, &dat, (TMapFunc)bstree_dump_map_f);
	bstree_map_internal(root, DEPTH_FIRST_IN, &dat, (TMapFunc)bstree_dump_map_f);
	bstree_map_internal(root, DEPTH_FIRST_POST, &dat, (TMapFunc)bstree_dump_map_f);
}
#endif
