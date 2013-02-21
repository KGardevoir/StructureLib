#include "linked_structures.h"
#include "allocator.h"
#define DEFAULT_SIZE 256
#include "city.h"
#include <math.h>

static htable_cluster*
new_htable_cluster(void* key, void* data, uint32_t hash){
	htable_cluster init = {
		.hash = hash,
		.key = key,
		.data = data
	};
	typeof(init) *dat = (typeof(init)*)MALLOC(sizeof(init));
	memcpy(dat, &init, sizeof(init));
	return data;
}

typedef struct htable_recompute_d {
	htable *tbl;
	list_tspec *type;
} htable_recompute_d;

static long
memcomp(const void *a, const void* b, list_tspec* type){ return (a>b?1:(a<b?-1:0)); }
static void
htable_cluster_destroy(htable_cluster* tbl, list_tspec* type){
	if(type && type->parent && type->parent[0] && type->parent[0]->destroy)
		type->parent[0]->destroy(tbl->data, type->parent[0]);
	FREE(tbl);
}
static htable_cluster*
htable_cluster_copy(htable_cluster* tbl, list_tspec* type){
	return new_htable_cluster(tbl->key,
		(type && type->parent && type->parent[0] && type->parent[0]->deep_copy)?
			type->parent[0]->deep_copy(tbl->data, type->parent[0]):
			tbl->data,
		tbl->hash);
}
static long
htable_cluster_compar(const htable_cluster* a, const htable_cluster *b, list_tspec* type){
	return (type && type->parent && type->parent[0] && type->parent[0]->compar)?
		type->parent[0]->compar(a,b,type->parent[0]):
		memcomp(a,b,type);
}
static long
htable_cluster_key_compar(const void* key, htable_cluster* tbl, list_tspec* type){
	return (type && type->parent && type->parent[0] && type->parent[0]->key_compar)?
		type->parent[0]->key_compar(key,tbl,type->parent[0]):
		memcomp(key,tbl, type);
}

static list_tspec hash_type = {
	.destroy = (lDestroy)htable_cluster_destroy,
	.deep_copy = (lDeepCopy)htable_cluster_copy,
	.compar = (lCompare)htable_cluster_compar,
	.key_compar = (lKeyCompare)htable_cluster_key_compar,
	.parent = NULL
};
static list_tspec*
new_derived_type(list_tspec* ddtype){
	list_tspec *new = MALLOC(sizeof(list_tspec)+sizeof(list_tspec*));
	memcpy(new, &hash_type, sizeof(list_tspec));
	new->parent = (void*)new+sizeof(list_tspec);
	new->parent[0] = ddtype;
	return new;
}

static BOOLEAN
htable_recompute_f(htable_cluster *data, htable_recompute_d* aux){
	//TODO remove old data first, assuming new entry
	uint64_t idx = data->hash%aux->tbl->size;
	if(aux->tbl->array[idx] != NULL) aux->tbl->collision++;
	aux->tbl->array[idx] = splay_insert(aux->tbl->array[idx], data, FALSE, aux->type);
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
htable_resize(htable* tbl, double scalar, size_t isize, list_tspec* type){
	isize = (tbl?tbl->size:(isize==0?(isize = DEFAULT_SIZE):isize));
	htable init = {
		.size = next_prime(isize*scalar),
		.filled = 0,
		.collision = 0
	};
	htable *ntbl = MALLOC(sizeof(htable)+sizeof(splaytree*)*init.size);
	memcpy(ntbl, &init, sizeof(htable));
	memset(ntbl->array, 0, ntbl->size*sizeof(void*));
	htable_recompute_d dat = {
		.tbl = ntbl,
		.type = type
	};
	htable_map(tbl, DEPTH_FIRST_PRE, FALSE, &dat, (lMapFunc)htable_recompute_f);
	return ntbl;
}

htable*
htable_insert(htable* table, void* key, size_t key_size, void *data, BOOLEAN copy, size_t isize, list_tspec *type){
	type = new_derived_type(type);
	isize = (table?table->size:(isize==0?(isize = DEFAULT_SIZE):isize));
	htable_cluster* new;
	size_t at = (new = new_htable_cluster(key, data, CityHash64((const char*)key, key_size)))->hash % isize;
	if(!table || table->filled > isize/2){//make table bigger
		htable *tbl = htable_resize(table, 2.0, isize, type);
		htable_clear(table, FALSE, type);
		table = tbl;
	}
	if(table->array[at] != NULL) table->collision++;
	table->array[at] = splay_insert(table->array[at], data, copy, type);
	table->filled++;
	free(type);
	return table;
}

htable*
htable_remove(htable* table, void* key, size_t key_size, void **rtn, BOOLEAN destroy_data, list_tspec* type){
	if(!table) return table;
	type = new_derived_type(type);
	htable_cluster **rtn2 = NULL;
	size_t at = CityHash64((const char*)key, key_size) % table->size;
	table->array[at] = splay_remove(table->array[at], key, (void**)rtn2, FALSE, type);
	if(rtn && *rtn2) *rtn = (*rtn2)->data;
	if(*rtn2){
		if(destroy_data) type->destroy((*rtn2)->data, type);
		if(table->array[at] != NULL) table->collision--;//means there was a collision
		table->filled--;
		if(table->filled < table->size/4){//make table smaller
			htable *tbl = htable_resize(table, 0.5, 0, type);
			htable_clear(table, FALSE, type);
			table = tbl;
			//rehash table
		}
		htable_cluster_destroy(*rtn2, NULL);
	}
	free(type);
	return table;
}

void*
htable_element(htable* table, void* key, size_t key_size, list_tspec* type){
	if(!table) return NULL;
	size_t at = CityHash64((const char*)key, key_size) % table->size;
	table->array[at] = splay_find(table->array[at], key, type);
	return ((htable_cluster*)table->array[at]->data)->data;
}

BOOLEAN
htable_map(htable *table, TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!table) return TRUE;
	size_t i = 0;
	for(; i < table->size; i++){
		if(!bstree_map(table->array[i], strat, FALSE, aux, func)) return FALSE;
	}
	return TRUE;
}

void
htable_clear(htable *tbl, BOOLEAN destroy_data, list_tspec* type){
	if(!tbl) return;
	size_t i = 0;
	for(; i < tbl->size; i++){
		bstree_clear(tbl->array[i], destroy_data, type);
	}
}
