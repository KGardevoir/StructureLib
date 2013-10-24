#include "scapegoat.h"
#include "dlist.h"
#include <math.h>
#include "aLong.h"

#define NODE_PRINT(P) do {\
	printf("(%s=%p)", #P, P);\
	if(P) printf("->data->data=%ld", ((aLong*)(P)->data)->data);\
	printf(", (%s->left=%p)", #P, (P)->left);\
	if((P)->left) printf("->data->data=%ld", ((aLong*)((P)->left->data))->data);\
	printf(", (%s->right=%p)", #P, (P)->right);\
	if((P)->right) printf("->data->data=%ld", ((aLong*)(P)->right->data)->data);\
	printf("\n");\
} while(0)

scapegoat*
scapegoat_new(double a, const Comparable_vtable *data_method){
	if(a < 0.5) a = 0.5;
	if(a > 1.) a = 1.;
	scapegoat goat = {
		.size = 0,
		.max_size = 0,
		.a = a,
		.inv_log_a_inv = 1./(-log(a)),//== 1./log(1/a)
		.data_method = data_method,
		.root = NULL
	};
	return (scapegoat*)memcpy(LINKED_MALLOC(sizeof(goat)), &goat, sizeof(goat));
}

void
scapegoat_destroy(scapegoat* goat){
	LINKED_FREE(goat);
}

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
static inline BOOLEAN
isABalanced(scapegoat *goat, btree* run, btree* parent, size_t *size){
	size_t a = 0, b = 0;
	a = *size;
	//btree_info(run, NULL, NULL, NULL, NULL, &a, NULL, NULL);
	//determine which child run->data is (avoid re-evaluating the lhs)
	btree_info(btree_sibling(parent, run), NULL, NULL, NULL, NULL, &b, NULL, NULL);
	//wparent += w + 1;
	*size = a+1+b;//goes up by at least 1
	return (a <= (goat->a*(a+1+b))) && (b <= (goat->a*(a+1+b)));
}

BOOLEAN
scapegoat_insert(scapegoat* goat, Object* data, BOOLEAN copy){
	size_t depth = 0;
	BOOLEAN success;
	goat->root = btree_insert_with_depth(goat->root, data, goat->data_method, &depth, copy, &success);
	if(!success) return FALSE;
	goat->size++;
	goat->max_size = max(goat->max_size, goat->size);
	if(depth > log(goat->max_size)*goat->inv_log_a_inv){
		dlist *path = btree_path(goat->root, data, goat->data_method);
		dlist *run = path;
		size_t psize = 0;
		path = dlist_tail(path);
		DLIST_ITERATE_REVERSE(run, path){ if(!isABalanced(goat, (btree*)run->data, (btree*)run->prev->data, &psize)) break; }
		if(run == NULL) run = path;
		btree *parent = (btree*)run->prev->data;
		btree **side = &parent->right;
		if((btree*)run->data != *side)
			side = &parent->left;
		*side = btree_balance(*side);//update the side correctly
		dlist_clear(path, FALSE);
	}
	return TRUE;
}

Object *
scapegoat_remove(scapegoat* goat, Object* key, const Comparable_vtable *key_method, BOOLEAN destroy_data){
	btree *tr;
	Object *data = NULL;
	if((tr = btree_remove(goat->root, key, key_method, &data, destroy_data))) {
		goat->size--;
		goat->root = tr;
		if(goat->size < goat->a*goat->max_size){
			goat->root = btree_balance(goat->root);
			goat->max_size = goat->size;
		}
	}
	return data;
}

Object*
scapegoat_find(scapegoat* goat, Object* data, const Comparable_vtable *data_method){//returns the btree pointer to where Object is located
	btree *found = btree_find(goat->root, data, data_method);
	return (found&&data_method->compare(data, found->data)==0)?found->data:NULL;
}

void
scapegoat_clear(scapegoat* goat, BOOLEAN destroy){
	btree_clear(goat->root, destroy);
	goat->size = 0;
	goat->max_size = 0;
	goat->root = NULL;
}
