#include "htable.h"

#include "city.h"
#include <math.h>
static void htable_node_destroy(const htable_node* tbl);
static htable_node* htable_node_copy(const htable_node* tbl, void* buf);
static long htable_node_compare(const htable_node *self, const htable_node *oth);
static hash_t htable_hash(htable_node *self);


static htable_node_vtable hash_type = {
	.parent = {
		.hash = (hash_t(*)(const Object*))htable_hash,
		.hashable = NULL,
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
new_htable_node(Object* data, const htable *table, void* buf){
	htable_node *self = (htable_node*)memcpy(buf, &(htable_node){
		.method = &hash_type,
		.table = table,
		.data = data
	}, sizeof(htable));
	self->method->parent.hash((const Object*)self);//get the hash
	return self;
}

static void
htable_node_destroy(const htable_node* self){
	(void)self;
	//nothing to destroy...
}

static hash_t
htable_hash(htable_node *self){
	if(!self->data->method->hash){
		size_t len = 0;
		const char* hashes = CALL(self->data, hashable, (self->data, &len), ({ len = sizeof(self->data); ((const char*)&self->data); }) );
		return self->hash = CityHash64(hashes, len);
	} else {
		return self->hash = CALL(self->data, hash, (self->data), 0);
	}
}

static htable_node*
htable_node_copy(const htable_node* self, void *buf){
	return new_htable_node(self->data, self->table, buf);
}

static long
htable_node_compare(const htable_node* self, const htable_node *oth){
	//return memcomp(self,oth);
	return self->table->data_method->compare(self->data, oth->data);
}

static size_t
next_prime(size_t num){
	if(num % 2 == 0) num++;
	while(TRUE){
		size_t max_test = floor(sqrt(num));
		size_t x = 3;
		for(; x < max_test && num % x != 0; x+=2);
		if(x > max_test && num % x != 0) return num;
		num+=2;
	}
}

static inline void *
allocate_set(size_t size, int num){
	return memset(LINKED_MALLOC(size), num, size);
}


htable*
htable_new(size_t size, const Comparable_vtable *key_method){
	size = next_prime(size);
	return memcpy(LINKED_MALLOC(sizeof(htable)), &(htable){
		.m_size = size,
		.data_method = key_method,
		.m_filled = 0,
		.m_collision = 0,
		.m_array = (__typeof__(((htable*)NULL)->m_array))allocate_set(sizeof(((htable*)NULL)->m_array[0])*size, 0)
	}, sizeof(htable));
}

typedef struct htable_recompute_d {
	htable *ctable;
	__typeof__(((htable*)NULL)->m_array) array;
	size_t size;
} htable_recompute_d;

static BOOLEAN
htable_recompute_f(htable_node *data, htable_recompute_d* tbl){
	//TODO remove old data first, assuming new entry
	uint64_t idx = data->hash%tbl->size;
	if(tbl->array[idx] != NULL) tbl->ctable->m_collision++;
	tbl->array[idx] = splay_insert(tbl->array[idx], (Object*)data, &data->method->compare, FALSE);
	return TRUE;
}

static void
htable_resize(htable* tbl, double scalar, size_t isize){
	size_t next_size = next_prime(scalar*isize);
	htable_recompute_d ntbl = {
		.ctable = tbl,
		.array = (__typeof__(tbl->m_array))allocate_set(sizeof(tbl->m_array[0])*next_size, 0),
		.size = next_size
	};
	tbl->m_collision = 0;
	htable_map(tbl, DEPTH_FIRST_PRE, FALSE, &ntbl, (lMapFunc)htable_recompute_f);
	tbl->m_size = ntbl.size;
	LINKED_FREE(tbl->m_array);
	tbl->m_array = ntbl.array;
}

void
htable_insert(htable* table, Object* data, BOOLEAN copy){
	if(!table) return;
	htable_node* lnew = new_htable_node(data, table, LINKED_MALLOC(sizeof(htable_node)));
	if(table->m_filled > table->m_size/2){//make table bigger
		htable_resize(table, 2.0, table->m_size);
	}
	size_t at = lnew->hash % table->m_size;
	if(table->m_array[at] != NULL) table->m_collision++;
	table->m_array[at] = splay_insert(table->m_array[at], (Object*)lnew, &lnew->method->compare, copy);
	table->m_filled++;
}

void
htable_remove(htable* table, Object* key, const Comparable_vtable *key_method, Object** rtn, BOOLEAN destroy){
	if(!table) return;
	htable_node **rtn2 = NULL;
	const htable_node *node = new_htable_node(key, table, &(htable_node){});
	size_t at = node->hash % table->m_size;
	table->m_array[at] = splay_remove(table->m_array[at], key, key_method, (Object**)rtn2, FALSE);
	if(destroy) CALL_VOID((*rtn2)->data, destroy, ((*rtn2)->data));
	if(rtn && *rtn2) *rtn = (*rtn2)->data;
	if(*rtn2){
		if(table->m_array[at] != NULL) table->m_collision--;//means there was a collision
		table->m_filled--;
		if(table->m_filled < table->m_size/4.){//make table smaller
			htable_resize(table, 0.5, 0);
			//rehash table
		}
		LINKED_FREE(*rtn2);
	}
}

void*
htable_element(htable* table, Object* key, const Comparable_vtable *key_method){
	if(!table) return NULL;
	const htable_node *node = new_htable_node(key, table, &(htable_node){});
	size_t at = node->hash % table->m_size;
	table->m_array[at] = splay_find(table->m_array[at], key, key_method);
	return ((htable_node*)table->m_array[at]->data)->data;
}

BOOLEAN
htable_map(htable *table, const TRAVERSAL_STRATEGY strat, const BOOLEAN more_info, void* aux, const lMapFunc func){
	(void)more_info;//TODO implement more info
	if(!func) return FALSE;
	if(!table) return TRUE;
	size_t i = 0;
	for(; i < table->m_size; i++){
		if(!btree_map(table->m_array[i], strat, FALSE, aux, func)) return FALSE;
	}
	return TRUE;
}

void
htable_clear(htable *tbl){
	if(!tbl) return;
	size_t i = 0;
	for(; i < tbl->m_size; i++){
		btree_clear(tbl->m_array[i], FALSE);
	}
}
