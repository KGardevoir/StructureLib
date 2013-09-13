#include "btree.h"

/* Algorithms adapted from Ch.12 of Introduction To Algorithms by Thomas H. Cormen, Charles E. Leiserson, Ronald L.
 * Rivest, Clifford Stein */

static inline btree*
new_bsnode(Object* data, BOOLEAN deep_copy){
	btree init = {
		.left = NULL,
		.right = NULL,
		.data = deep_copy?CALL(data,copy,(data, LINKED_MALLOC(data->method->size)),data):data
	};
	return memcpy(LINKED_MALLOC(sizeof(btree)), &init, sizeof(init));
}

btree*
btree_insert(btree *root, Object* data, const Comparable_vtable* data_method, BOOLEAN copy){
	btree *p = btree_parent(root, data, data_method);
	btree *lnew = new_bsnode(data, copy);
	if(p == NULL){//tree was NULL
		root = lnew;
	} else if(data_method->compare(data, p->data) < 0) {
		p->left = lnew;
	} else if(data_method->compare(data, p->data) > 0) {
		p->right = lnew;
	} else {//are equal, ignore for now (no duplicates)
		LINKED_FREE(lnew);
	}
	return root;
}

static btree*
btree_transplant(btree *root, btree *u, btree *v, btree *up, btree *vp){
	if(up == NULL){
		return v;
	} else if(u == up->left){
		up->left = v;
	} else {
		up->right = v;
	}
	return root;
}

btree*
btree_remove(btree *root, Object* data, const Comparable_vtable* data_method, Object** rtn, BOOLEAN destroy_data){
	btree *node = btree_find(root, data, data_method);
	if(node == NULL) return NULL;//nothing to return, no such element
	btree *nodep = btree_parent(root, data, data_method);
	if(node->left == NULL){
		root = btree_transplant(root, node, node->right, nodep, node);
	} else if(node->right == NULL){
		root = btree_transplant(root, node, node->left, nodep, node);
	} else {
		btree* min = btree_findmin(node->right);
		btree* par = btree_parent(root, min->data, data_method);
		if(par != node){
			root = btree_transplant(root, min, min->right, par, min);
			min->right = node->right;
		}
		root = btree_transplant(root, node, min, nodep, par);
		min->left = node->left;
	}
	data = node->data;
	LINKED_FREE(node);
	if(destroy_data){
		CALL_VOID(data, destroy,(data));
		data = NULL;
	}
	if(rtn) *rtn = data;
	return root;
}

btree*
btree_find(btree *root, Object *data, const Comparable_vtable *data_method){
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

btree*
btree_parent(btree *root, Object* data, const Comparable_vtable *data_method){
	long c;
	btree *parent = root;
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

dlist* /* with type btree*/
btree_path(btree *root, Object* data, const Comparable_vtable *data_method){
	dlist *head = NULL;
	long c;
	if(root == NULL) return NULL;
	while(root != NULL && (c = data_method->compare(data, root->data)) != 0){
		head = dlist_pushback(head, (Object*)root, FALSE);
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return (root==NULL)?head:dlist_pushback(head, (Object*)root, FALSE);
}

btree*
btree_predessor(btree* root, btree* node, const Comparable_vtable *data_method){
	if(root == NULL || node == NULL) return NULL;
	if(node->left != NULL) return btree_findmax(node->left);
	dlist* parent = btree_path(root, node->data, data_method);//use btree_path instead
	if(parent == NULL) return root;
	dlist* head = parent;
	parent = parent->prev;
	while((btree*)parent->prev->data == node && (btree*)parent->data != root){
		node = (btree*)parent->data;
		parent = parent->prev;
	}
	dlist_clear(head, FALSE);
	return node;
}

btree*
btree_successor(btree* root, btree* node, const Comparable_vtable *data_method){
	if(root == NULL || node == NULL) return NULL;
	if(node->right != NULL) return btree_findmin(node->right);
	dlist* parent = btree_path(root, node->data, data_method);//use btree_path instead
	if(parent == NULL) return root;
	dlist* head = parent;
	parent = parent->prev;
	while((btree*)parent->prev->data == node && (btree*)parent->data != root){
		node = (btree*)parent->data;
		parent = parent->prev;
	}
	dlist_clear(head, FALSE);
	return node;
}

btree*
btree_findmin(btree* root){
	if(root == NULL) return root;
	while(root->left != NULL) root = root->left;
	return root;
}

btree*
btree_findmax(btree* root){
	if(root == NULL) return root;
	while(root->right != NULL) root = root->right;
	return root;
}

#if 1
struct btree_map_internal_d {
	btree *node;
	size_t depth;
};
static struct btree_map_internal_d*
btree_map_internal_d_new(size_t depth, btree *node){
	struct btree_map_internal_d init = {
		.node = node,
		.depth = depth
	};
	struct btree_map_internal_d *new = LINKED_MALLOC(sizeof(init));
	memcpy(new, &init, sizeof(init));
	return new;
}

static inline BOOLEAN
btree_map_in_internal(btree *root, const BOOLEAN more_info, const void *aux, const lMapFunc func){
	dlist *stk = NULL;
	size_t depth = 0, position = 0, size = 0;
	if(more_info) btree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	btree *cur = root;
	while(stk || cur){
		if(cur){
			depth++;
			stk = dlist_pushback(stk, (Object*)btree_map_internal_d_new(depth, cur), FALSE);
			cur = cur->left;
		} else {
			struct btree_map_internal_d *node = NULL;
			stk = dlist_popback(stk, (Object**)&node, FALSE);
			cur = node->node;
			depth = node->depth;
			LINKED_FREE(node);
			//by changing this from a stack to a queue, this can be transformed to a pre-order
			//traversal
			if(more_info){
				lMapFuncAux ax = {
					.isAux = TRUE,
					.depth = depth-1,
					.position = position,
					.size = size,
					.aux = aux
				};
				if(!func(cur->data, &ax, cur)) return FALSE;
				position++;
			} else {
				if(!func(cur->data, aux, cur)) return FALSE;
			}
			cur = cur->right;
		}
	}
	return TRUE;
}
static inline BOOLEAN
btree_map_pre_internal(btree *root, const BOOLEAN more_info, const void* aux, const lMapFunc func){
	dlist *stk = NULL;
	size_t depth = 0, position = 0, size = 0;
	if(more_info) btree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	btree *cur = root;
	while(stk || cur){
		if(cur){
			depth++;
			stk = dlist_pushback(stk, (Object*)btree_map_internal_d_new(depth, cur), FALSE);
			if(more_info){
				lMapFuncAux ax = {
					.isAux = TRUE,
					.depth = depth-1,
					.position = position,
					.size = size,
					.aux = aux
				};
				if(!func(cur->data, &ax, cur)) goto false_cleanup;
				position++;
			} else {
				if(!func(cur->data, aux, cur)) goto false_cleanup;
			}
			cur = cur->left;
		} else {
			struct btree_map_internal_d *node = NULL;
			stk = dlist_popback(stk, (Object**)&node, FALSE);
			cur = node->node;
			depth = node->depth;
			LINKED_FREE(node);
			//by changing this from a stack to a queue, this can be transformed to a pre-order
			//traversal
			cur = cur->right;
		}
	}
	return TRUE;
false_cleanup:
	{
		dlist* run;
		DLIST_ITERATE(run, stk, LINKED_FREE(run->data););
		dlist_clear(stk, FALSE);
		return FALSE;
	}
}
static inline BOOLEAN
btree_map_post_internal(btree *root, const BOOLEAN more_info, const void* aux, const lMapFunc func){
	if(!root) return TRUE;
	size_t depth = 0, position = 0, size = 0;
	if(more_info){
		btree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	}
	dlist *stk = dlist_pushfront(NULL, (Object*)btree_map_internal_d_new(depth, root), FALSE);
	btree *prev = NULL;
	while(stk){
		struct btree_map_internal_d* node = (struct btree_map_internal_d*)stk->data;
		btree *cur = node->node;
		depth = node->depth;
		if(!prev || prev->left == cur || prev->right == cur){
			if(cur->left || cur->right){
				stk = dlist_pushfront(stk, (Object*)btree_map_internal_d_new(depth+1, cur->left?cur->left:cur->right), FALSE);
			}
		} else if(cur->left == prev){
			if(cur->right)
				stk = dlist_pushfront(stk, (Object*)btree_map_internal_d_new(depth+1, cur->right), FALSE);
		} else {
			if(more_info){
				lMapFuncAux ax = {
					.isAux = TRUE,
					.depth = depth,
					.position = position,
					.size = size,
					.aux = aux
				};
				if(!func(cur->data, &ax, cur)) goto false_cleanup;
				position++;
			} else {
				if(!func(cur->data, aux, cur)) goto false_cleanup;
			}
			stk = dlist_popfront(stk, (Object**)&node, FALSE);
			LINKED_FREE(node);
		}
		prev = cur;
	}
	return TRUE;
false_cleanup:
	{
		dlist* run;
		DLIST_ITERATE(run, stk, LINKED_FREE(run->data););
		dlist_clear(stk, FALSE);
		return FALSE;
	}
}
static inline BOOLEAN
btree_map_breadth_internal(btree *root, const BOOLEAN more_info, const void* aux, const lMapFunc func){
	dlist* q = dlist_pushback(NULL, (Object*)btree_map_internal_d_new(0, root), FALSE);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) btree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	btree *prev = root;
	while(q){
		btree *node;
		q = dlist_popfront(q, (Object**)&node, FALSE);
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = depth,
				.position = position,
				.size = size,
				.aux = aux
			};
			if(!func(node->data, &ax, node)) goto false_cleanup;
			position++;
		} else {
			if(!func(node->data, aux, node)) goto false_cleanup;
		}
		if(prev && (prev->left == node || prev->right == node)){//new level
			depth++;
			prev = NULL;
		}
		if(!prev)
			prev = node->left?node->left:node->right;
		if(node->left)
			q = dlist_pushback(q, (Object*)node->left, FALSE);
		if(node->right)
			q = dlist_pushback(q, (Object*)node->right, FALSE);
	}
	return TRUE;
false_cleanup:
	{
		dlist* run;
		DLIST_ITERATE(run, q, LINKED_FREE(run->data););
		dlist_clear(q, FALSE);
		return FALSE;
	}
}

static inline BOOLEAN
btree_map_internal(btree *root, const TRAVERSAL_STRATEGY strat, const BOOLEAN more_info, const void* aux, const lMapFunc func){
	if(!func) return FALSE;
	if(!root) return TRUE;
	switch(strat){
		case DEPTH_FIRST_PRE:
			return btree_map_pre_internal(root, more_info, aux, func);
		case DEPTH_FIRST_IN:
			return btree_map_in_internal(root, more_info, aux, func);
		case DEPTH_FIRST_POST:
			return btree_map_post_internal(root, more_info, aux, func);
		case BREADTH_FIRST:
			return btree_map_breadth_internal(root, more_info, aux, func);
	}
	return FALSE;
}
#else
static BOOLEAN
btree_map_internal(btree *root, const TRAVERSAL_STRATEGY strat, void* aux, TMapFunc func){
	if(!root) return TRUE;
	if(!btree_map_internal(root->left , strat, aux, func)) return FALSE;
	if(!btree_map_internal(root->right, strat, aux, func)) return FALSE;
	if(!func(root, aux)) return FALSE;
	return TRUE;
}
#endif

BOOLEAN
btree_map(btree* root, const TRAVERSAL_STRATEGY strat, const BOOLEAN more_info, const void* aux, const lMapFunc func){
	return btree_map_internal(root, strat, more_info, aux, func);
}

struct btree_clear_d {
	const BOOLEAN destroy_data;
};

static BOOLEAN
btree_clear_f1(Object *data, dlist **freer, btree* root){
	*freer = dlist_pushback(*freer, (Object*)root, FALSE);
	return TRUE;
}
static BOOLEAN
btree_clear_f2(btree* root, struct btree_clear_d *aux, dlist *node){
	if(aux->destroy_data) CALL_VOID(root->data, destroy, (root->data));
	LINKED_FREE(root);
	return TRUE;
}

void
btree_clear(btree* root, BOOLEAN destroy_data){//iterate in postfix order
	dlist *freer = NULL;
	btree_map_post_internal(root, FALSE, &freer, (lMapFunc)btree_clear_f1);
	struct btree_clear_d aux = {.destroy_data = destroy_data};
	dlist_map(freer, FALSE, &aux, (lMapFunc)btree_clear_f2);
	dlist_clear(freer, FALSE);
}

#define MAX(a,b) ({ typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ typeof(a) _a = (a), _b = (b); _a < _b ? _a : _b; })

void
btree_info(btree *root, size_t *min, size_t *max, size_t *avg, size_t *num_leaves, size_t *size, dlist** rleaves, dlist** rnodes){
	//dlist* leaves = btree_leaves(root);
	dlist *leaves = NULL, *nodes = NULL;
	size_t tmin = -1, tmax = 0, tavg = 0, tleaves = 0, tsize = 0;
	dlist *stk = NULL;
	size_t depth = 0;
	btree *cur = root;
	while(stk || cur){
		if(cur){
			depth++;
			stk = dlist_pushback(stk, (Object*)btree_map_internal_d_new(depth, cur), FALSE);
			if(rnodes) nodes = dlist_pushback(nodes, (Object*)cur, FALSE);
			cur = cur->left;
			tmin = MIN(tmin, depth);
			tmax = MAX(tmax, depth);
			tsize++;
		} else {
			struct btree_map_internal_d* data = NULL;
			stk = dlist_popfront(stk, (Object**)&data, FALSE);
			depth = data->depth;
			cur = data->node;
			LINKED_FREE(data);
			if(cur->left == NULL && cur->right == NULL){
				tleaves++;
				if(rleaves)
					leaves = dlist_pushback(leaves, (Object*)cur, FALSE);
				//printf("%lu\n", depth);
				tavg = tavg + depth - 1;
			}
			cur = cur->right;
		}
	}
	//printf("Leaves: %lu\n", dlist_length(leaves));
	if(min) *min = tmin-1;
	if(max) *max = tmax-1;
	if(avg) *avg = tsize==0?0:(size_t)(tavg/tsize);
	if(num_leaves) *num_leaves = tleaves;
	if(size) *size = tsize;
	if(rleaves) *rleaves = leaves;
	if(rnodes)  *rnodes = nodes;
}

struct btree_balance_d {
	btree *prev;
	btree *head;
};

static BOOLEAN
btree_balance_f(Object *data, struct btree_balance_d *dat, btree *node){
	(void)data;
	node->left = dat->prev;
	if(dat->prev) dat->prev->right = node;
	else dat->head = node;
	dat->prev = node;
	return TRUE;
}

struct aLong {
	void *method;
	long val;
};

static btree *
btree_balance_i(btree *mid, size_t len){
	if(mid == NULL){
end:
		if(mid){
			//printf("mid: %p - %p %p\n", mid, mid->left, mid->right);
			mid->left = NULL;
			mid->right = NULL;
		}
		return mid;
	}
	btree *left = mid;
	size_t i = 0;
	for(; i < len/2; i++, mid = mid->right);
	if(mid == left) goto end;
	//printf("left:(%p <- %p -> %p) mid:(%p <- %p -> %p), %d - %d\n", left->left, left, left->right, mid->left, mid, mid->right, i, len-i-1);
	mid->left->right = NULL;
	//mid->left = NULL;
	mid->left = btree_balance_i(left, i);
	mid->right = btree_balance_i(mid->right, len-i-1);
	return mid;
}

btree*
btree_balance(btree *root){
	struct btree_balance_d nodes = {
		.prev = NULL,
		.head = NULL
	};
	//turn tree into list
	btree_map_in_internal(root, FALSE, &nodes, (lMapFunc)btree_balance_f);
	nodes.prev->right = NULL;
	btree* node = nodes.head;
	size_t len = 1;
	//left=prev
	//right=next
	for(; node->right != NULL; node = node->right, len++);
	return btree_balance_i(nodes.head, len);
}

#if 0
static BOOLEAN
btree_dump_map_f(btree* node, size_t level, struct btree_dump_map_d *aux){
	printf("%*s%-4p: %lu <%p, %p>\n", (int)level, "", node->data, level, node->left, node->right);
	return TRUE;
}

void
print_btree_structure(btree *root, list_tspec *type){
	btree_map_internal(root, DEPTH_FIRST_PRE, &dat, (TMapFunc)btree_dump_map_f);
	btree_map_internal(root, DEPTH_FIRST_IN, &dat, (TMapFunc)btree_dump_map_f);
	btree_map_internal(root, DEPTH_FIRST_POST, &dat, (TMapFunc)btree_dump_map_f);
}
#endif
