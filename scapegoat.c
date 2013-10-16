#include "scapegoat.h"
#include "dlist.h"
#include <math.h>

scapegoat*
scapegoat_new(double a){
	scapegoat goat = {
		.size = 0,
		.max_size = 0,
		.a = a,
		.inv_log_a_inv = 1./log(1./a)
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

void
scapegoat_insert(scapegoat* goat, Object* data, const Comparable_vtable *data_method, BOOLEAN copy){
	size_t depth = 0;
	goat->root = btree_insert_with_depth(goat->root, data, data_method, &depth, copy);
	goat->size++;
	goat->max_size = max(goat->max_size, goat->size);
	if(depth > log(goat->max_size)*goat->inv_log_a_inv){
		dlist *path = btree_path(goat->root, data, data_method);
		dlist *run = path;
		BOOLEAN isNotABalanced(btree* run, btree* parent){
			size_t w = 0, wparent = 0;
			btree_info(run, NULL, NULL, NULL, NULL, &w, NULL, NULL);
			//determine which child run->data is (avoid re-evaluating the lhs)
			btree* side = parent->left;
			if(parent->left == (btree*)run){
				side = parent->right;
			}
			btree_info(side, NULL, NULL, NULL, NULL, &wparent, NULL, NULL);
			wparent += w + 1;
			if(w > goat->a*wparent){
				return TRUE;
			}
			return FALSE;
		}
		DLIST_ITERATE_REVERSE(run, path, if(isNotABalanced((btree*)run->data, (btree*)run->prev->data)) goto loop_end; );
	loop_end: {
			btree *parent = (btree*)run->prev->data;
			btree **side;
			if((btree*)run->data == parent->left)
				side = &parent->left;
			else
				side = &parent->right;
			*side = btree_balance((btree*)run->data);//update the side correctly
			dlist_clear(path, FALSE);
		}
	}
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

btree *
scapegoat_find(scapegoat* goat, Object* data, const Comparable_vtable *data_method){//returns the btree pointer to where Object is located
	return btree_find(goat->root, data, data_method);
}

