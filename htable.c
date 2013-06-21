#include "htable.h"

#include "city.h"
#include <math.h>
static const char* htable_node_hashables(const htable_node* tbl, size_t *size);
static void htable_node_destroy(const htable_node* tbl);
static htable_node* htable_node_copy(const htable_node* tbl, void* buf);
static long htable_node_compare(const htable_node *self, const htable_node *oth);


static htable_node_vtable hash_type = {
	.parent = {
		.hashable = (char*(*)(const Object*, size_t *size))htable_node_hashables,
		.destroy = (void(*)(const Object*))htable_node_destroy,
		.copy = (Object*(*)(const Object*, void*))htable_node_copy,
		.equals = NULL,//TODO this shouldn't be null
		.size = sizeof(htable_node)
	},
	.compare = {
		.compare = (long(*)(const void*, const void*))htable_node_compare
	}
};
static htable_node*
new_htable_node(Object* key, const Comparable_vtable *key_method, void* data, uint32_t hash, void* buf){
	htable_node init = {
		.method = &hash_type,
		.hash = hash,
		.key = key,
		.key_method = key_method,
		.data = data
	};
	return memcpy(buf, &init, sizeof(init));
}

static void
htable_node_destroy(const htable_node* self){
	//nothing to destroy...
}
const char*
htable_node_hashables(const htable_node* self, size_t *len){
	*len = self->method->parent.size;
	return (const char*)self;
}
static htable_node*
htable_node_copy(const htable_node* self, void *buf){
	return new_htable_node(self->key, self->key_method, self->data, self->hash, buf);
}
static long
htable_node_compare(const htable_node* self, const htable_node *oth){
	//return memcomp(self,oth);
	return self->key_method->compare(self->key, oth->key);
}

static BOOLEAN
htable_recompute_f(htable_node *data, htable* tbl){
	//TODO remove old data first, assuming new entry
	uint64_t idx = data->hash%tbl->size;
	if(tbl->array[idx] != NULL) tbl->collision++;
	tbl->array[idx] = splay_insert(tbl->array[idx], (Object*)data, &data->method->compare, FALSE);
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
	isize = (tbl?tbl->size:(isize==0?(isize = DEFAULT_HTABLE_SIZE):isize));
	htable init = {
		.size = next_prime(isize*scalar),
		.filled = 0,
		.collision = 0
	};
	htable *ntbl = (htable*)LINKED_MALLOC(sizeof(htable)+sizeof(splaytree*)*init.size);
	memcpy(ntbl, &init, sizeof(htable));
	memset(ntbl->array, 0, ntbl->size*sizeof(void*));
	htable_map(tbl, DEPTH_FIRST_PRE, FALSE, ntbl, (lMapFunc)htable_recompute_f);
	return ntbl;
}

htable*
htable_insert(htable* table, Object* key, const Comparable_vtable* key_method, void *data, BOOLEAN copy, size_t isize){
	isize = (table?table->size:(isize==0?(isize = DEFAULT_HTABLE_SIZE):isize));
	htable_node* lnew;
	size_t len;
	const char* hashes = key->method->hashable(key, &len);
	size_t at = (lnew = new_htable_node(key, key_method, data, CityHash64(hashes, len), LINKED_MALLOC(sizeof(htable_node))))->hash % isize;
	if(!table || table->filled > isize/2){//make table bigger
		htable *tbl = htable_resize(table, 2.0, isize);
		htable_clear(table);
		table = tbl;
	}
	if(table->array[at] != NULL) table->collision++;
	table->array[at] = splay_insert(table->array[at], (Object*)lnew, &lnew->method->compare, copy);
	table->filled++;
	return table;
}

htable*
htable_remove(htable* table, Object* key, const Comparable_vtable *key_method, Object** key_rtn, void **rtn){
	if(!table) return table;
	htable_node **rtn2 = NULL;
	size_t len;
	const char* hashes = key->method->hashable(key, &len);
	size_t at = CityHash64(hashes, len) % table->size;
	table->array[at] = splay_remove(table->array[at], key, key_method, (Object**)rtn2, FALSE);
	if(rtn && *rtn2) *rtn = (*rtn2)->data;
	if(key_rtn && *rtn2) *rtn = (*rtn2)->key;
	if(*rtn2){
		if(table->array[at] != NULL) table->collision--;//means there was a collision
		table->filled--;
		if(table->filled < table->size/4){//make table smaller
			htable *tbl = htable_resize(table, 0.5, 0);
			htable_clear(table);
			table = tbl;
			//rehash table
		}
		LINKED_FREE(*rtn2);
	}
	return table;
}

void*
htable_element(htable* table, Object* key, const Comparable_vtable *key_method){
	if(!table) return NULL;
	size_t len;
	const char* hashes = key->method->hashable(key, &len);
	size_t at = CityHash64(hashes, len) % table->size;
	table->array[at] = splay_find(table->array[at], key, key_method);
	return ((htable_node*)table->array[at]->data)->data;
}

BOOLEAN
htable_map(htable *table, const TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!table) return TRUE;
	size_t i = 0;
	for(; i < table->size; i++){
		if(!btree_map(table->array[i], strat, FALSE, aux, func)) return FALSE;
	}
	return TRUE;
}

void
htable_clear(htable *tbl){
	if(!tbl) return;
	size_t i = 0;
	for(; i < tbl->size; i++){
		btree_clear(tbl->array[i], FALSE);
	}
}
