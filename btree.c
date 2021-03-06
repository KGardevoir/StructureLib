#include "btree.h"
#include <math.h>

//#include "aLong.h"
#include <limits.h>
#define NODE_PRINT(P) do {\
	printf("(%s=%p)", #P, P);\
	if(P) printf("->data->data=%ld", ((aLong*)(P)->data)->data);\
	printf(", (%s->left=%p)", #P, (P)->left);\
	if((P)->left) printf("->data->data=%ld", ((aLong*)((P)->left->data))->data);\
	printf(", (%s->right=%p)", #P, (P)->right);\
	if((P)->right) printf("->data->data=%ld", ((aLong*)(P)->right->data)->data);\
	printf("\n");\
} while(0)
#define NODE_DUMP(P) if(P){ \
	printf("%s: <%p: %p %p> ", #P, P, (P)->left, (P)->right); \
}


/* Algorithms adapted from Ch.12 of Introduction To Algorithms by Thomas H. Cormen, Charles E. Leiserson, Ronald L.
 * Rivest, Clifford Stein */

static inline btree*
new_bsnode(Object* data, BOOLEAN deep_copy){
	btree init = {
		.left = NULL,
		.right = NULL,
		.data = deep_copy?CALL(data,copy,data,LINKED_MALLOC(data->method->size)):data
	};
	return memcpy(LINKED_MALLOC(sizeof(btree)), &init, sizeof(init));
}

btree*
btree_insert(btree *root, Object* data, const Comparable_vtable* data_method, BOOLEAN copy, BOOLEAN *success){
	btree *p = btree_parent(root, data, data_method);
	btree *lnew = new_bsnode(data, copy);
	BOOLEAN my_success = TRUE;
	if(!p) p = root;//btree_parent returns NULL if no parent
	if(!p){//tree was NULL
		p = root = lnew;
	} else if(data_method->compare(data, p->data) < 0) {
		p->left = lnew;
	} else if(data_method->compare(data, p->data) > 0) {
		p->right = lnew;
	} else {//are equal, ignore for now (no duplicates)
		my_success = FALSE;
		LINKED_FREE(lnew);
	}
	if(success) *success = my_success;
	//NODE_PRINT(p);
	return root;
}

btree*
btree_insert_with_depth(btree *root, Object *data, const Comparable_vtable* data_method, size_t *r_depth, BOOLEAN copy, BOOLEAN *success){
	size_t depth = 0;
	btree *p = btree_parent_with_depth(root, data, data_method, &depth);
	if(!p) p = root;//btree_parent returns NULL if no parent
	btree *lnew = new_bsnode(data, copy);
	BOOLEAN my_success = TRUE;
	if(p == NULL){//tree was NULL
		root = lnew;
	} else if(data_method->compare(data, p->data) < 0) {
		p->left = lnew;
	} else if(data_method->compare(data, p->data) > 0) {
		p->right = lnew;
	} else {//are equal, ignore for now (no duplicates)
		my_success = FALSE;
		LINKED_FREE(lnew);
	}
	if(success) *success = my_success;
	if(r_depth) *r_depth = depth+1;
	return root;
}


static btree*
btree_transplant(btree *root, btree *u, btree *v, btree *up, btree *vp){
	(void)vp;
	if(!up){
		return v;
	} else if(u == up->left){
		up->left = v;
	} else { //u == up->right
		up->right = v;
	}
	return root;
}

btree*
btree_remove(btree *root, Object* data, const Comparable_vtable* data_method, Object** rtn, BOOLEAN destroy_data, BOOLEAN *success){
	btree *nodep = btree_parent(root, data, data_method);//look for the parent first (or the would-be parent)
	btree *node;
	if(nodep){
		node = btree_find(nodep, data, data_method);//use the parent to locate the next node
	} else {
		node = btree_find(root, data, data_method);//the node we are looking for is the root (probably)
		if(!node) return NULL;
	}
	if(success) *success = TRUE;
	//NODE_DUMP(root); NODE_DUMP(node); NODE_DUMP(node->right); NODE_DUMP(node->left); NODE_DUMP(nodep); printf("\n");
	int whois = (node->left?1:0) | (node->right?2:0);
	switch(whois){
		default:
		case 1:
			root = btree_transplant(root, node, node->left, nodep, node);
			break;
		case 2:
			root = btree_transplant(root, node, node->right, nodep, node);
			break;
		case 3:{
			btree* par = btree_findmin_parent(node->right);
			btree* min = btree_findmin(par);
			if(par != node){
				root = btree_transplant(root, min, min->right, par, min);
				min->right = node->right;
			}
			root = btree_transplant(root, node, min, nodep, par);
			min->left = node->left;
		} break;
	}
	data = node->data;
	LINKED_FREE(node);
	if(destroy_data){
		CALL_VOID(data, destroy);
	}
	if(rtn) *rtn = data;
	//NODE_DUMP(root); printf("\n");
	return root;
}

btree*
btree_find_with_depth(btree *root, Object *data, const Comparable_vtable *data_method, size_t *r_depth){
	if(!root) return root;
	btree *node = btree_parent_with_depth(root, data, data_method, r_depth);
	if(!node) node = root;
	//NODE_DUMP(node); printf("\n");
	long c = data_method->compare(data, node->data);
	if(c != 0){
		if(c < 0){
			node = node->left;
		} else {
			node = node->right;
		}
	}
	if(r_depth) *r_depth = *r_depth+1;
	if(node && data_method->compare(data, node->data) != 0) return NULL;
	return node;
}

btree*
btree_find(btree *root, Object *data, const Comparable_vtable *data_method){
	return btree_find_with_depth(root, data, data_method, NULL);
}

btree*
btree_parent_with_depth(btree *root, Object* data, const Comparable_vtable *data_method, size_t *r_depth){
	long c;
	size_t depth = 0;
	if(!root) return root;
	btree *parent = NULL;
	while(root != NULL && (c = data_method->compare(data, root->data)) != 0){
		parent = root;
		if(c < 0){
			root = root->left;
		} else {
			root = root->right;
		}
		depth++;
	}
	if(parent && r_depth) *r_depth = depth-1;
	return parent;
}

btree*
btree_parent(btree *root, Object* data, const Comparable_vtable *data_method){
	return btree_parent_with_depth(root, data, data_method, NULL);
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

//predecessor, successor
btree*
btree_predecessor(btree* root, btree* node, const Comparable_vtable *data_method){
	if(!root || !node) return NULL;
	if(node->left) return btree_findmax(node->left);
	dlist* parents = btree_path(root, node->data, data_method);//use btree_path instead
	if(!parents) return NULL;
	dlist* run = parents;
	parents = dlist_tail(parents);//rotate so we are at the tail (parents->data == node)
	DLIST_ITERATE_REVERSE(run, parents){
		if(((btree*)run->data)->right == (btree*)run->next->data){//check if our last child was a right child
			node = (btree*)run->data;
			goto end;
		}
	}
	node = NULL;//there is no predecessor, node was a global minimum
end:
	dlist_clear(parents, FALSE);
	return node;
}

btree*
btree_successor(btree* root, btree* node, const Comparable_vtable *data_method){
	if(root == NULL || node == NULL) return NULL;
	if(node->right != NULL) return btree_findmin(node->right);
	dlist* parents = btree_path(root, node->data, data_method);//use btree_path instead
	if(parents == NULL) return NULL;
	dlist* run = parents;
	parents = dlist_tail(parents);//rotate so we are at the tail (parents->data == node)
	DLIST_ITERATE_REVERSE(run, parents){
		if(((btree*)run->data)->left == (btree*)run->next->data){//check if our last child was a left child
			node = (btree*)run->data;
			goto end;
		}
	}
	node = NULL;
end:
	dlist_clear(parents, FALSE);
	return node;
}

btree*
btree_findmin_parent(btree* root){
	if(!root) return root;
	btree *p = NULL;
	while(root->left){
		p = root;
		root = root->left;
	}
	return p;
}

btree*
btree_findmin(btree* root){
	if(root == NULL) return root;
	while(root->left) root = root->left;
	return root;
}

btree*
btree_findmax_parent(btree* root){
	if(!root) return root;
	btree *p = NULL;
	while(root->right){
		p = root;
		root = root->right;
	}
	return p;
}

btree*
btree_findmax(btree* root){
	if(!root) return root;
	while(root->right) root = root->right;
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

btree_iterator_in*
btree_iterator_in_new(btree *root, btree_iterator_in* mem){
	mem->i_curafter = mem->r_current = mem->i_root = root;
	mem->r_depth = -1;
	mem->i_stk = NULL;
	return mem;
}

btree *
btree_iterator_in_next(btree_iterator_in *self){
	if(!self->r_current){
		self->r_current = NULL;
		self->r_depth = 0;
		return NULL;
	}
	size_t depth = self->r_depth;
	btree *cur = self->i_curafter;
	while(self->i_stk || cur){
		if(cur){
			depth++;
			self->i_stk = dlist_pushback(self->i_stk, (Object*)btree_map_internal_d_new(depth, cur), FALSE);
			cur = cur->left;
		} else {
			struct btree_map_internal_d *node = NULL;
			self->i_stk = dlist_popback(self->i_stk, (Object**)&node, FALSE);
			self->r_current = node->node;
			self->r_depth = node->depth;
			self->i_curafter = node->node->right;
			LINKED_FREE(node);
			return self->r_current;
		}
	}
	self->r_current = NULL;
	self->r_depth = 0;
	return NULL;
}

void
btree_iterator_in_destroy(btree_iterator_in* self){
	dlist *run;
	DLIST_ITERATE(run, self->i_stk) LINKED_FREE(run->data);
	dlist_clear(self->i_stk, FALSE);
	self->i_stk = NULL;
	self->r_depth = 0;
	self->r_current = NULL;
	self->i_root = NULL;
}

btree_iterator_pre*
btree_iterator_pre_new(btree *root, btree_iterator_pre* mem){
	mem->r_current = mem->i_curafter = mem->i_root = root;
	mem->r_depth = -1;//I don't like this as it depends on the behavior of
	mem->i_stk = NULL;
	mem->p_add_children = TRUE;
	return mem;
}

btree *
btree_iterator_pre_next(btree_iterator_pre *self){
	if(!self->r_current){
		self->r_depth = 0;
		self->r_current = NULL;
		self->p_add_children = TRUE;
		return NULL;
	}
	size_t depth = self->r_depth;
	btree *cur = self->i_curafter;
	while(self->i_stk || cur){
		if(cur){
			depth++;
			self->i_stk = dlist_pushback(self->i_stk, (Object*)btree_map_internal_d_new(depth, cur), FALSE);
			self->r_current = cur;
			self->i_curafter = cur->left;
			self->r_depth = depth;
			self->p_add_children = TRUE;
			return self->r_current;
		} else {
			struct btree_map_internal_d *node = NULL;
			self->i_stk = dlist_popback(self->i_stk, (Object**)&node, FALSE);
			cur = node->node;
			depth = node->depth;
			LINKED_FREE(node);
			//by changing this from a stack to a queue, this can be transformed to a pre-order
			//traversal
			cur = cur->right;
		}
	}
	self->p_add_children = TRUE;
	self->r_current = NULL;
	self->r_depth = 0;
	return NULL;
}

void
btree_iterator_pre_destroy(btree_iterator_pre* self){
	dlist *run;
	DLIST_ITERATE(run, self->i_stk) LINKED_FREE(run->data);
	dlist_clear(self->i_stk, FALSE);
	self->i_stk = NULL;
	self->r_depth = 0;
	self->r_current = NULL;
	self->i_root = NULL;
}

btree_iterator_post*
btree_iterator_post_new(btree *root, btree_iterator_post* mem){
	mem->i_root = root;
	mem->r_current = NULL;
	mem->r_depth = 0;
	mem->i_stk = dlist_pushfront(NULL, (Object*)btree_map_internal_d_new(0, root), FALSE);
	mem->i_prev = NULL;
	return mem;
}

btree *
btree_iterator_post_next(btree_iterator_post *self){
	if(!(self->i_stk || self->r_current)){
		self->r_current = NULL;
		self->r_depth = 0;
		return NULL;
	}
	size_t depth = self->r_depth;
	btree *prev = self->i_prev;
	while(self->i_stk){
		struct btree_map_internal_d* node = (struct btree_map_internal_d*)self->i_stk->data;
		btree *cur = node->node;
		depth = node->depth;
		if(!prev || prev->left == cur || prev->right == cur){
			if(cur->left || cur->right){
				self->i_stk = dlist_pushfront(self->i_stk, (Object*)btree_map_internal_d_new(depth+1, cur->left?cur->left:cur->right), FALSE);
			}
		} else if(cur->left == prev){
			if(cur->right)
				self->i_stk = dlist_pushfront(self->i_stk, (Object*)btree_map_internal_d_new(depth+1, cur->right), FALSE);
		} else {
			self->i_stk = dlist_popfront(self->i_stk, (Object**)&node, FALSE);
			LINKED_FREE(node);
			//visit cur
			self->r_current = cur;
			self->r_depth = depth;
			self->i_prev = cur;
			return self->r_current;
		}
		prev = cur;
	}
	self->r_current = NULL;
	self->r_depth = 0;
	return NULL;
}

void
btree_iterator_post_destroy(btree_iterator_post* self){
	dlist *run;
	DLIST_ITERATE(run, self->i_stk) LINKED_FREE(run->data);
	dlist_clear(self->i_stk, FALSE);
	self->i_stk = NULL;
	self->r_depth = 0;
	self->r_current = NULL;
	self->i_root = NULL;
}


btree_iterator_breadth*
btree_iterator_breadth_new(btree *root, btree_iterator_breadth* mem){
	mem->i_root = root;
	mem->r_current = NULL;
	mem->r_depth = 0;
	mem->i_stk = dlist_pushfront(NULL, (Object*)btree_map_internal_d_new(0, root), FALSE);
	mem->i_prev = root;
	return mem;
}

btree *
btree_iterator_breadth_next(btree_iterator_breadth *self){
	if(!(self->i_stk || self->r_current)){
		self->r_current = NULL;
		return NULL;
	}
	size_t depth = self->r_depth;
	btree *node;
	self->i_stk = dlist_popfront(self->i_stk, (Object**)&node, FALSE);
	self->r_current = node;
	//return node
	if(self->i_prev && (self->i_prev->left == node || self->i_prev->right == node)){//new level
		depth++;//TODO figure out how to make this work
		self->i_prev = NULL;
	}
	if(!self->i_prev)
		self->i_prev = node->left?node->left:node->right;
	if(node->left)
		self->i_stk = dlist_pushback(self->i_stk, (Object*)node->left, FALSE);
	if(node->right)
		self->i_stk = dlist_pushback(self->i_stk, (Object*)node->right, FALSE);
	return self->r_current;
}

void
btree_iterator_breadth_destroy(btree_iterator_breadth* self){
	dlist *run;
	DLIST_ITERATE(run, self->i_stk) LINKED_FREE(run->data);
	dlist_clear(self->i_stk, FALSE);
	self->i_stk = NULL;
	self->r_depth = 0;
	self->r_current = NULL;
	self->i_root = NULL;
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

void
btree_clear(btree* root, BOOLEAN destroy_data){//iterate in postfix order
	dlist *freer = NULL;
	btree_iterator_pre* it = btree_iterator_pre_new(root, &(btree_iterator_pre){});
	btree *next;
	for(next = btree_iterator_pre_next(it); next; next = btree_iterator_pre_next(it)){
		freer = dlist_pushback(freer, (Object*)next, FALSE);
	}
	dlist *run;
	DLIST_ITERATE(run, freer){
		if(destroy_data) CALL_VOID(((btree*)run->data)->data, destroy);
		LINKED_FREE(run->data);
	}
	dlist_clear(freer, FALSE);
}

#define MAX(a,b) ({ __typeof__(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })

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

static btree *
btree_balance_inplace_i(btree *mid, size_t len){
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
	mid->left = btree_balance_inplace_i(left, i);
	mid->right = btree_balance_inplace_i(mid->right, len-i-1);
	return mid;
}

btree*
btree_balance(btree *root){
	if(!root) return root;
	//turn tree into list
	btree *prev = NULL;
	btree *head = NULL;
	btree_iterator_in *it = btree_iterator_in_new(root, &(btree_iterator_in){});
	btree *node;
	for(node = btree_iterator_in_next(it); node; node = btree_iterator_in_next(it)){
		node->left = prev;
		if(prev) prev->right = node;
		else head = node;
		prev = node;
	}
	prev->right = NULL;
	node = head;
	size_t len = 1;
	//left=prev
	//right=next
	for(; node->right != NULL; node = node->right, len++);
	return btree_balance_inplace_i(head, len);
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
