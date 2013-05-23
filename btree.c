#include "btree.h"

/* Algorithms adapted from Ch.12 of Introduction To Algorithms by Thomas H. Cormen, Charles E. Leiserson, Ronald L.
 * Rivest, Clifford Stein */

static btree*
new_bsnode(Object* data, BOOLEAN deep_copy){
	btree init = {
		.left = NULL,
		.right = NULL,
		.data = deep_copy?data->method->copy(data, MALLOC(data->method->size)):data
	};
	btree *n = (btree*)MALLOC(sizeof(btree));
	memcpy(n, &init, sizeof(*n));
	return n;
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
		FREE(lnew);
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
	FREE(node);
	if(destroy_data){
		data->method->destroy(data);
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
		head = dlist_append(head, (Object*)root, FALSE);
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
	}
	return (root==NULL)?head:dlist_append(head, (Object*)root, FALSE);
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
struct btree_map_pre_in_internal_d {
	btree *node;
	size_t depth;
};
static struct btree_map_pre_in_internal_d*
btree_map_pre_in_internal_d_new(size_t depth, btree *node){
	struct btree_map_pre_in_internal_d init = {
		.node = node,
		.depth = depth
	};
	struct btree_map_pre_in_internal_d *new = MALLOC(sizeof(init));
	memcpy(new, &init, sizeof(init));
	return new;
}

static inline BOOLEAN
btree_map_pre_in_internal(btree *root, BOOLEAN pass_data, BOOLEAN more_info, BOOLEAN IN_ORDER, void* aux, lMapFunc func){
	dlist *stk = NULL;
	size_t depth = 0, position = 0, size = 0;
	if(more_info) btree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	btree *cur = root;
	while(stk || cur){
		if(cur){
			depth++;
			stk = dlist_append(stk, (Object*)btree_map_pre_in_internal_d_new(depth, cur), FALSE);
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
			struct btree_map_pre_in_internal_d *node = NULL;
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
btree_map_post_internal(btree *root, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!root) return TRUE;
	size_t depth = 0, position = 0, size = 0;
	if(more_info){
		btree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	}
	dlist *stk = dlist_push(NULL, (Object*)btree_map_pre_in_internal_d_new(depth, root), FALSE);
	btree *prev = NULL;
	while(stk){
		struct btree_map_pre_in_internal_d* node = (struct btree_map_pre_in_internal_d*)stk->data;
		btree *cur = node->node;
		depth = node->depth;
		if(!prev || prev->left == cur || prev->right == cur){
			if(cur->left || cur->right){
				stk = dlist_push(stk, (Object*)btree_map_pre_in_internal_d_new(depth+1, cur->left?cur->left:cur->right), FALSE);
			}
		} else if(cur->left == prev){
			if(cur->right)
				stk = dlist_push(stk, (Object*)btree_map_pre_in_internal_d_new(depth+1, cur->right), FALSE);
		} else {
			if(more_info){
				lMapFuncAux ax = {
					.isAux = TRUE,
					.depth = depth,
					.position = position,
					.size = size,
					.aux = aux
				};
				if(!func(pass_data?cur->data:(Object*)cur, &ax)) goto false_cleanup;
				position++;
			} else {
				if(!func(pass_data?cur->data:(Object*)cur, aux)) goto false_cleanup;
			}
			stk = dlist_dequeue(stk, (Object**)&node, FALSE);
			FREE(node);
		}
		prev = cur;
	}
	return TRUE;
false_cleanup:
	{
		dlist* run;
		DLIST_ITERATE(run, stk, FREE(run->data););
		return FALSE;
	}
}
static inline BOOLEAN
btree_map_breadth_internal(btree *root, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	dlist* q = dlist_append(NULL, (Object*)btree_map_pre_in_internal_d_new(0, root), FALSE);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) btree_info(root, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	btree *prev = root;
	while(q){
		btree *node;
		q = dlist_dequeue(q, (Object**)&node, FALSE);
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = depth,
				.position = position,
				.size = size,
				.aux = aux
			};
			if(!func(pass_data?node->data:(Object*)node, &ax)) goto false_cleanup;
			position++;
		} else {
			if(!func(pass_data?node->data:(Object*)node, aux)) goto false_cleanup;
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
false_cleanup:
	{
		dlist* run;
		DLIST_ITERATE(run, q, FREE(run->data););
		return FALSE;
	}
}

static inline BOOLEAN
btree_map_internal(btree *root, const TRAVERSAL_STRATEGY strat, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!root) return TRUE;
	if(strat == DEPTH_FIRST_PRE || strat == DEPTH_FIRST_IN){
		return btree_map_pre_in_internal(root, pass_data, more_info, strat == DEPTH_FIRST_IN, aux, func);
	} else if(strat == DEPTH_FIRST_POST){
		return btree_map_post_internal(root, pass_data, more_info, aux, func);
	} else { //Breadth First
		return btree_map_breadth_internal(root, pass_data, more_info, aux, func);
	}
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
btree_map(btree* root, const TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc func){
	return btree_map_internal(root, strat, TRUE, more_info, aux, func);
}

struct btree_clear_d {
	const BOOLEAN destroy_data;
};

static BOOLEAN
btree_clear_f1(btree* root, dlist **freer){
	*freer = dlist_append(*freer, (Object*)root, FALSE);
	return TRUE;
}
static BOOLEAN
btree_clear_f2(btree* root, struct btree_clear_d *aux){
	if(aux->destroy_data) root->data->method->destroy(root->data);
	FREE(root);
	return TRUE;
}

void
btree_clear(btree* root, BOOLEAN destroy_data){//iterate in postfix order
	dlist *freer = NULL;
	btree_map_post_internal(root, FALSE, FALSE, &freer, (lMapFunc)btree_clear_f1);
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
	size_t tmin = -1, tmax = 0, tavg = root?1:0, tleaves = root?1:0, tsize = root?1:0;
	dlist *stk = NULL;
	size_t depth = 0;
	btree *cur = root;
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
				//printf("%lu\n", depth);
				tavg = tavg + depth;
			}
			tmin = MIN(tmin, depth);
			tmax = MAX(tmax, depth);
			cur = cur->right;
		}
	}
	//printf("Leaves: %lu\n", dlist_length(leaves));
	if(min) *min = tmin;
	if(max) *max = tmax;
	if(avg) *avg = tsize==0?0:(size_t)(tavg/tsize);
	if(num_leaves) *num_leaves = tleaves;
	if(size) *size = tsize;
	if(rleaves) *rleaves = leaves;
	if(rnodes)  *rnodes = nodes;
}

static BOOLEAN
btree_balance_f(btree *node, btree **prev){
	node->left = *prev;
	if(*prev) (*prev)->right = node;
	*prev = node;
	return TRUE;
}

struct aLong {
	void *method;
	long val;
};

static btree *
btree_balance_i(btree *mid, size_t len){
	if(len == 0 || mid == NULL) {
		if(mid == NULL) return mid;
		//printf("- %p, %ld, %p %p len: %ld\n", mid, ((struct aLong*)mid->data)->val, mid->left, mid->right, len);
		if(mid->left){
			btree *eh = mid->left;
			//printf("<- %p %ld, %p %p len: %ld\n", eh, eh?(long)eh->data:NULL, eh?eh->left:1, eh?eh->right:1, len);
			mid->left->left = NULL;
			mid->left->right = NULL;
		}
		if(mid->right){
			btree *eh = mid->right;
			//printf("-> %p %ld, %p %p len: %ld\n", eh, eh?(long)eh->data:NULL, eh?eh->left:1, eh?eh->right:1, len);
			mid->right->left = NULL;
			mid->right->right = NULL;
		}
		return mid;
	}
	//printf("- %p, %ld, %p %p len: %ld\n", mid, ((struct aLong*)mid->data)->val, mid->left, mid->right, len);
	size_t i;
	btree *left = NULL;
	if(mid->left){
		btree *n = mid->left;
		for(i = 0; i < len/2 && n->left != NULL; i++)
			n = n->left;
		mid->left->right = NULL;//clip end
		left = n;
	}
	btree *right = NULL;
	if(mid->right){
		btree* n = mid->right;
		for(i = 0; i < len/2 && n->right != NULL; i++)
			n = n->right;
		mid->right->left = NULL;//clip start
		right = n;
	}
	mid->left = btree_balance_i(left, len/2);
	mid->right = btree_balance_i(right, len/2);
	//printf("<> %p, %ld, %p %p len: %ld\n", mid, mid?(long)mid->data:NULL, mid?mid->left:1, mid?mid->right:1, len);
	return mid;
}

btree*
btree_balance(btree *root){
	btree *node = NULL;
	//turn tree into list
	btree_map_pre_in_internal(root, FALSE, FALSE, TRUE, &node, (lMapFunc)btree_balance_f);
	node->right = NULL;
	size_t len = 0;
	//left=prev
	//right=next
	for(; node->left != NULL; node = node->left, len++);
	size_t i = 0;
	for(; i < len/2; i++) node = node->right;
	return btree_balance_i(node, len/2);
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
