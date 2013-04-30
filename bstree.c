#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

/* Algorithms adapted from Ch.12 of Introduction To Algorithms by Thomas H. Cormen, Charles E. Leiserson, Ronald L.
 * Rivest, Clifford Stein */

static bstree*
new_bsnode(Object* data, BOOLEAN deep_copy){
	bstree init = {
		.left = NULL,
		.right = NULL,
		.data = deep_copy?data->method->copy(data, MALLOC(data->method->size)):data
	};
	bstree *n = (bstree*)MALLOC(sizeof(bstree));
	memcpy(n, &init, sizeof(*n));
	return n;
}

bstree*
bstree_insert(bstree *root, Object* data, const Comparable_vtable* data_method, BOOLEAN copy){
	bstree *p = bstree_parent(root, data, data_method);
	bstree *lnew = new_bsnode(data, copy);
	if(p == NULL){//tree was NULL
		root = lnew;
	} else if(data_method->compare(data, p->data) < 0) {
		p->left = lnew;
	} else if(data_method->compare(data, p->data) > 0) {
		p->right = lnew;
	} else {//are equal, ignore for now (no duplicates)
		FREE(lnew);
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
bstree_remove(bstree *root, Object* data, const Comparable_vtable* data_method, Object** rtn, BOOLEAN destroy_data){
	bstree *node = bstree_find(root, data, data_method);
	if(node == NULL) return NULL;//nothing to return, no such element
	bstree *nodep = bstree_parent(root, data, data_method);
	if(node->left == NULL){
		root = bstree_transplant(root, node, node->right, nodep, node);
	} else if(node->right == NULL){
		root = bstree_transplant(root, node, node->left, nodep, node);
	} else {
		bstree* min = bstree_findmin(node->right);
		bstree* par = bstree_parent(root, min->data, data_method);
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
		data->method->destroy(data);
		data = NULL;
	}
	if(rtn) *rtn = data;
	return root;
}

bstree*
bstree_find(bstree *root, Object *data, const Comparable_vtable *data_method){
	long c;
	if(root == NULL) return root;
	while(root != NULL && (c = data_method->compare(data, root->data)) != 0){
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return root;
}

bstree*
bstree_parent(bstree *root, Object* data, const Comparable_vtable *data_method){
	long c;
	bstree *parent = root;
	if(root == NULL) return root;
	while(root != NULL && (c = data_method->compare(data, root->data)) != 0){
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
bstree_path(bstree *root, Object* data, const Comparable_vtable *data_method){
	dlist *head = NULL;
	long c;
	if(root == NULL) return NULL;
	while(root != NULL && (c = data_method->compare(data, root->data)) != 0){
		head = dlist_append(head, (Object*)root, FALSE);
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return (root==NULL)?head:dlist_append(head, (Object*)root, FALSE);
}

bstree*
bstree_predessor(bstree* root, bstree* node, const Comparable_vtable *data_method){
	if(root == NULL || node == NULL) return NULL;
	if(node->left != NULL) return bstree_findmax(node->left);
	dlist* parent = bstree_path(root, node->data, data_method);//use bstree_path instead
	if(parent == NULL) return root;
	dlist* head = parent;
	parent = parent->prev;
	while((bstree*)parent->prev->data == node && (bstree*)parent->data != root){
		node = (bstree*)parent->data;
		parent = parent->prev;
	}
	dlist_clear(head, FALSE);
	return node;
}

bstree*
bstree_successor(bstree* root, bstree* node, const Comparable_vtable *data_method){
	if(root == NULL || node == NULL) return NULL;
	if(node->right != NULL) return bstree_findmin(node->right);
	dlist* parent = bstree_path(root, node->data, data_method);//use bstree_path instead
	if(parent == NULL) return root;
	dlist* head = parent;
	parent = parent->prev;
	while((bstree*)parent->prev->data == node && (bstree*)parent->data != root){
		node = (bstree*)parent->data;
		parent = parent->prev;
	}
	dlist_clear(head, FALSE);
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
};

static BOOLEAN
bstree_clear_map(bstree* root, struct free_cluster *aux){
	if(aux->destroy_data) root->data->method->destroy(root->data);
	FREE(root);
	return TRUE;
}

void
bstree_clear(bstree* root, BOOLEAN destroy_data){//iterate in postfix order
	dlist *stk = NULL, *freer = NULL;
	bstree *cur = root;
	while(stk != NULL || cur != NULL){
		if(cur){
			stk = dlist_push(stk, (Object*)cur, FALSE);
			cur = cur->left;
		} else {
			stk = dlist_pop(stk, (Object**)&cur, FALSE);
			freer = dlist_append(freer, (Object*)cur, FALSE);
			cur = cur->right;
		}
	}
	struct free_cluster aux = {.destroy_data = destroy_data};
	dlist_map(freer, FALSE, &aux, (lMapFunc)bstree_clear_map);
	dlist_clear(freer, FALSE);
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
	struct node_and_depth *new = MALLOC(sizeof(init));
	memcpy(new, &init, sizeof(init));
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
			stk = dlist_append(stk, (Object*)new_node_and_depth(depth, cur), FALSE);
			if(!IN_ORDER){
				if(more_info){
					lMapFuncAux ax = {
						.isAux = TRUE,
						.depth = depth,
						.position = position,
						.size = size,
						.aux = aux
					};
					if(!func(pass_data?cur->data:(Object*)cur, &ax)) return FALSE;
					position++;
				} else {
					if(!func(pass_data?cur->data:(Object*)cur, aux)) return FALSE;
				}
			}
			cur = cur->left;
		} else {
			struct node_and_depth *node = NULL;
			stk = dlist_pop(stk, (Object**)&node, FALSE);
			cur = node->node;
			depth = node->depth;
			FREE(node);
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
					if(!func(pass_data?cur->data:(Object*)cur, &ax)) return FALSE;
					position++;
				} else {
					if(!func(pass_data?cur->data:(Object*)cur, aux)) return FALSE;
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
	dlist *stk = dlist_push(NULL, (Object*)new_node_and_depth(depth, root), FALSE);
	bstree *prev = NULL;
	while(stk){
		struct node_and_depth* node = (struct node_and_depth*)stk->data;
		bstree *cur = node->node;
		depth = node->depth;
		if(!prev || prev->left == cur || prev->right == cur){
			if(cur->left || cur->right){
				stk = dlist_push(stk, (Object*)new_node_and_depth(depth+1, cur->left?cur->left:cur->right), FALSE);
			}
		} else if(cur->left == prev){
			if(cur->right)
				stk = dlist_push(stk, (Object*)new_node_and_depth(depth+1, cur->right), FALSE);
		} else {
			if(more_info){
				lMapFuncAux ax = {
					.isAux = TRUE,
					.depth = depth,
					.position = position,
					.size = size,
					.aux = aux
				};
				if(!func(pass_data?cur->data:(Object*)cur, &ax)) return FALSE;
				position++;
			} else {
				if(!func(pass_data?cur->data:(Object*)cur, aux)) return FALSE;
			}
			stk = dlist_dequeue(stk, (Object**)&node, FALSE);
			FREE(node);
		}
		prev = cur;
	}
	return TRUE;
}
static inline BOOLEAN
bstree_map_breadth_internal(bstree *root, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	dlist* q = dlist_append(NULL, (Object*)new_node_and_depth(0, root), FALSE);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) bstree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	bstree *prev = root;
	while(q){
		bstree *node;
		q = dlist_dequeue(q, (Object**)&node, FALSE);
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = depth,
				.position = position,
				.size = size,
				.aux = aux
			};
			if(!func(pass_data?node->data:(Object*)node, &ax)) return FALSE;
			position++;
		} else {
			if(!func(pass_data?node->data:(Object*)node, aux)) return FALSE;
		}
		if(prev && (prev->left == node || prev->right == node)){//new level
			depth++;
			prev = NULL;
		}
		if(!prev)
			prev = node->left?node->left:node->right;
		if(node->left)
			q = dlist_append(q, (Object*)node->left, FALSE);
		if(node->right)
			q = dlist_append(q, (Object*)node->right, FALSE);
	}
	return TRUE;
}

static inline BOOLEAN
bstree_map_internal(bstree *root, const TRAVERSAL_STRATEGY strat, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!root) return TRUE;
	if(strat == DEPTH_FIRST_PRE || strat == DEPTH_FIRST_IN){
		return bstree_map_pre_in_internal(root, pass_data, more_info, strat == DEPTH_FIRST_IN, aux, func);
	} else if(strat == DEPTH_FIRST_POST){
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
			stk = dlist_append(stk, (Object*)cur, FALSE);
			if(rnodes) nodes = dlist_append(nodes, (Object*)cur, FALSE);
			cur = cur->left;
			depth++;
			tsize++;
		} else {
			stk = dlist_dequeue(stk, (Object**)&cur, FALSE);
			depth--;
			if(cur->left == NULL && cur->right == NULL){
				tleaves++;
				if(rleaves)
					leaves = dlist_append(leaves, (Object*)cur, FALSE);
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
