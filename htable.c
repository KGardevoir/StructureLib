#include "linked_structures.h"
#include "allocator.h"
#define DEFAULT_SIZE 256
#include "city.h"
#include <math.h>
static void htable_cluster_destroy(const htable_cluster* tbl);
static size_t htable_cluster_size(const htable_cluster*);
static htable_cluster* htable_cluster_copy(const htable_cluster* tbl);
static long htable_cluster_compare(const htable_cluster *self, const htable_cluster *oth);

static Comparable hash_type = {
	.parent = {
		.copy = (anObject*(*)(const anObject*))htable_cluster_copy,
		.destroy = (void(*)(const anObject*))htable_cluster_destroy,
		.getSize = (size_t(*)(const anObject*))htable_cluster_size
	},
	.compare = (long(*)(const aComparable*, const anObject*))htable_cluster_compare
};
static htable_cluster*
new_htable_cluster(aComparable* key, anObject* data, uint32_t hash){
	htable_cluster init = {
		.method = &hash_type,
		.hash = hash,
		.key = key,
		.data = data
	};
	htable_cluster *dat = (htable_cluster*)MALLOC(sizeof(init));
	memcpy(dat, &init, sizeof(init));
	return dat;
}

static void
htable_cluster_destroy(const htable_cluster* self){
	self->data->method->destroy(self->data);
	self->key->method->parent.destroy((anObject*)self->key);
}
static htable_cluster*
htable_cluster_copy(const htable_cluster* self){
	return new_htable_cluster(self->key, self->data, self->hash);
}
static long
htable_cluster_compare(const htable_cluster* self, const htable_cluster *oth){
	//return memcomp(self,oth);
	return self->key->method->compare(self->key, (anObject*)oth->key);
}
static size_t
htable_cluster_size(const htable_cluster *self){
	return sizeof(htable_cluster);
}


static BOOLEAN
htable_recompute_f(htable_cluster *data, htable* tbl){
	//TODO remove old data first, assuming new entry
	uint64_t idx = data->hash%tbl->size;
	if(tbl->array[idx] != NULL) tbl->collision++;
	tbl->array[idx] = splay_insert(tbl->array[idx], (aComparable*)data, FALSE);
	return TRUE;
}

static size_t
next_prime(size_t num){
	if(num % 2 == 0) num++;
	while(TRUE){
		size_t max_test = floor(sqrt(num));
		size_t x = 3;
		for(; x < max_test && num % x != 0; x+=2);
		if(x == max_test && num % x != 0) return num;
		num+=2;
	}
}

static htable*
htable_resize(htable* tbl, double scalar, size_t isize){
	isize = (tbl?tbl->size:(isize==0?(isize = DEFAULT_SIZE):isize));
	htable init = {
		.size = next_prime(isize*scalar),
		.filled = 0,
		.collision = 0
	};
	htable *ntbl = (htable*)MALLOC(sizeof(htable)+sizeof(splaytree*)*init.size);
	memcpy(ntbl, &init, sizeof(htable));
	memset(ntbl->array, 0, ntbl->size*sizeof(void*));
	htable_map(tbl, DEPTH_FIRST_PRE, FALSE, ntbl, (lMapFunc)htable_recompute_f);
	return ntbl;
}

htable*
htable_insert(htable* table, aComparable* key, anObject *data, BOOLEAN copy, size_t isize){
	isize = (table?table->size:(isize==0?(isize = DEFAULT_SIZE):isize));
	htable_cluster* lnew;
	size_t at = (lnew = new_htable_cluster(key, data, CityHash64((const char*)key, key->method->parent.getSize((anObject*)key))))->hash % isize;
	if(!table || table->filled > isize/2){//make table bigger
		htable *tbl = htable_resize(table, 2.0, isize);
		htable_clear(table, FALSE);
		table = tbl;
	}
	if(table->array[at] != NULL) table->collision++;
	table->array[at] = splay_insert(table->array[at], (aComparable*)lnew, copy);
	table->filled++;
	return table;
}

htable*
htable_remove(htable* table, aComparable* key, anObject **rtn, BOOLEAN destroy_data){
	if(!table) return table;
	htable_cluster **rtn2 = NULL;
	size_t at = CityHash64((const char*)key, key->method->parent.getSize((anObject*)key)) % table->size;
	table->array[at] = splay_remove(table->array[at], key, (aComparable**)rtn2, FALSE);
	if(rtn && *rtn2) *rtn = (*rtn2)->data;
	if(*rtn2){
		if(destroy_data){
			(*rtn2)->data->method->destroy((anObject*)*rtn2);
		}
		if(table->array[at] != NULL) table->collision--;//means there was a collision
		table->filled--;
		if(table->filled < table->size/4){//make table smaller
			htable *tbl = htable_resize(table, 0.5, 0);
			htable_clear(table, FALSE);
			table = tbl;
			//rehash table
		}
		FREE(*rtn2);
	}
	return table;
}

void*
htable_element(htable* table, aComparable* key){
	if(!table) return NULL;
	size_t at = CityHash64((const char*)key, key->method->parent.getSize((anObject*)key)) % table->size;
	table->array[at] = splay_find(table->array[at], key);
	return ((htable_cluster*)table->array[at]->data)->data;
}

BOOLEAN
htable_map(htable *table, const TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!table) return TRUE;
	size_t i = 0;
	for(; i < table->size; i++){
		if(!bstree_map(table->array[i], strat, FALSE, aux, func)) return FALSE;
	}
	return TRUE;
}

void
htable_clear(htable *tbl, BOOLEAN destroy_data){
	if(!tbl) return;
	size_t i = 0;
	for(; i < tbl->size; i++){
		bstree_clear(tbl->array[i], destroy_data);
	}
}
