#include "htable.h"

#include "city.h"
#include <math.h>

static inline double collision_ratio(htable* t) { if(t->size > 0) return ((double)t->m_collision)/t->size; return 0; }
static inline BOOLEAN shrink_table_Q(htable *t, const double ratio /*degree by which to shrink, < 1*/){
	return t->m_collision < t->m_max_size*(ratio*ratio) && collision_ratio(t) < ratio*ratio;
}
static inline BOOLEAN grow_table_Q(htable *t, const double ratio /*degree by which to grow, > 1*/){
	return t->m_collision > t->m_max_size/(ratio) && collision_ratio(t) > 1/ratio;
}

#define max(a,b) \
   ({ __typeof__(a) _a = (a); \
       __typeof__(b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__(a) _a = (a); \
       __typeof__(b) _b = (b); \
     _a > _b ? _b : _a; })

static size_t next_prime(size_t n);
static void htable_node_destroy(htable_node* tbl);
static htable_node* htable_node_copy(const htable_node* tbl, void* buf);
static long htable_node_compare(const htable_node *self, const htable_node *oth);
static hash_t htable_hash(htable_node *self);

static void htable_clear_internal(htable *tbl, BOOLEAN destroy, BOOLEAN destroy_nodes);
#if 1
#include "aLong.h"
static void
dump_nodes(btree* tree){
	BOOLEAN
	map(htable_node *data, lMapFuncAux *dat, btree *tree){
		//printf("<%ld (%p) %zu: %p %p> ", ((aLong*)(data->data))->data, tree, dat->depth, tree->left, tree->right);
		printf("%ld ", ((aLong*)(data->data))->data);
		return TRUE;
	}
	btree_map(tree, DEPTH_FIRST_PRE, TRUE, NULL, (lMapFunc)map);
}
void
dump_htable(htable *t){
	size_t i = 0;
	printf("%zu/%zu, collision: %zu\n", t->size, t->m_max_size, t->m_collision);
	for(;i < t->m_max_size; i++){
		if(t->m_array[i]){ printf("%3zu: ", i); dump_nodes(t->m_array[i]->root); printf("\n"); }
	}
}
#endif

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
new_htable_node(Object* data, const Comparable_vtable *compare, void* buf){
	htable_node *self = (htable_node*)memcpy(buf, &(htable_node){
		.method = &hash_type,
		.compare = compare,
		.data = data
	}, sizeof(htable_node));
	self->method->parent.hash((const Object*)self);//get the hash
	return self;
}

static void
htable_node_destroy(htable_node* self){
	LINKED_FREE(self);
}

static hash_t
htable_hash(htable_node *self){
	if(!self->data->method->hash){
		size_t len = 0;
		const char* hashes = CALL(self->data, hashable, ({ len = sizeof(self->data); ((const char*)&self->data); }), &len);
		return self->hash = CityHash64(hashes, len);
	} else {
		return self->hash = CALL(self->data, hash, 0);
	}
}

static htable_node*
htable_node_copy(const htable_node* self, void *buf){
	return new_htable_node(self->data, self->compare, buf);
}

static long
htable_node_compare(const htable_node* self, const htable_node *oth){
	//return memcomp(self,oth);
	long c = self->compare->compare(self->data, oth->data);
	//printf("compare(self->data=%ld, oth->data=%ld)=%ld\n", ((aLong*)(self->data))->data, ((aLong*)(oth->data))->data, c);
	return c;
}


static inline void *
allocate_set(size_t size, int num){
	return memset(LINKED_MALLOC(size), num, size);
}


htable*
htable_new(size_t size, const Comparable_vtable *key_method){
	size = next_prime(max(size, 1U));
	return memcpy(LINKED_MALLOC(sizeof(htable)), &(htable){
		.m_max_size = size,
		.data_method = key_method,
		.size = 0,
		.m_collision = 0,
		.m_array = (__typeof__(((htable*)NULL)->m_array))allocate_set(sizeof(((htable*)NULL)->m_array[0])*size, 0)
	}, sizeof(htable));
}

void
htable_destroy(htable* self){
	LINKED_FREE(self->m_array);
	LINKED_FREE(self);
}

typedef struct htable_recompute_d {
	htable *ctable;
	__typeof__(((htable*)NULL)->m_array) array;
	size_t size, collision;
} htable_recompute_d;

static BOOLEAN
htable_recompute_f(htable_node *data, htable_recompute_d* tbl){
	//TODO remove old data first, assuming new entry
	uint64_t idx = data->hash%tbl->size;
	if(tbl->array[idx]) tbl->collision++;
#ifdef SPLAY_PROCESS
	if(!tbl->array[idx]) tbl->array[idx] = splaytree_new(&data->method->compare);
	splay_insert(tbl->array[idx], (Object*)data, FALSE);
#else
	if(!tbl->array[idx]) tbl->array[idx] = scapegoat_new(SCAPEGOAT_ALPHA, &data->method->compare);
	scapegoat_insert(tbl->array[idx], (Object*)data, FALSE);
#endif
	return TRUE;
}

static void
htable_resize(htable* tbl, double scalar){
	size_t next_size = next_prime(scalar*tbl->m_max_size);
	htable_recompute_d ntbl = {
		.ctable = tbl,
		.array = (__typeof__(tbl->m_array))allocate_set(sizeof(tbl->m_array[0])*next_size, 0),
		.size = next_size,
		.collision = 0
	};
	htable_map(tbl, DEPTH_FIRST_PRE, FALSE, &ntbl, (lMapFunc)htable_recompute_f);
	size_t filled = tbl->size;
	htable_clear_internal(tbl, FALSE, FALSE);
	tbl->m_max_size = ntbl.size;
	tbl->size = filled;
	tbl->m_collision = ntbl.collision;
	LINKED_FREE(tbl->m_array);
	tbl->m_array = ntbl.array;
}

void
htable_insert(htable* table, Object* data, BOOLEAN copy){
	if(!table) return;
	htable_node* lnew = new_htable_node(data, table->data_method, LINKED_MALLOC(sizeof(htable_node)));
	if(grow_table_Q(table, 2)){//make table bigger
		htable_resize(table, 2.0);
	}
	size_t at = lnew->hash % table->m_max_size;
	if(table->m_array[at]) table->m_collision++;
#ifdef SPLAY_PROCESS
	if(!table->m_array[at]) table->m_array[at] = splaytree_new(&lnew->method->compare);
	splay_insert(table->m_array[at], (Object*)lnew, copy);
#else
	if(!table->m_array[at]) table->m_array[at] = scapegoat_new(SCAPEGOAT_ALPHA, &lnew->method->compare);
	scapegoat_insert(table->m_array[at], (Object*)lnew, copy);
#endif
	table->size++;
}

Object*
htable_remove(htable* table, Object* key, const Comparable_vtable *key_method, BOOLEAN destroy){
	if(!table) return NULL;
	htable_node *rtn2 = NULL;
	const htable_node *node = new_htable_node(key, key_method, &(htable_node){});
	size_t at = node->hash % table->m_max_size;
#ifdef SPLAY_PROCESS
	rtn2 = (htable_node*)splay_remove(table->m_array[at], (Object*)node, &node->method->compare, FALSE);
	if(table->m_array[at]->size == 0){
		splaytree_destroy(table->m_array[at]);
		table->m_array[at] = NULL;
	}
#else
	//printf("Removing: %ld\n", ((aLong*)key)->data);
	rtn2 = (htable_node*)scapegoat_remove(table->m_array[at], (Object*)node, &node->method->compare, FALSE);
	if(table->m_array[at]->size == 0){
		scapegoat_destroy(table->m_array[at]);
		table->m_array[at] = NULL;
	}
#endif
	//dump_htable(table);
	Object* data = NULL;
	if(rtn2){
		data = rtn2->data;
		if(destroy) CALL_VOID(rtn2->data, destroy);
		if(table->m_array[at]) table->m_collision--;//means there was a collision
		table->size--;
		LINKED_FREE(rtn2);
		if(shrink_table_Q(table, 0.5)){//make table smaller
			htable_resize(table, 0.5);
			//rehash table
		}
	}
	return data;
}

Object*
htable_find(htable* table, Object* key, const Comparable_vtable *key_method){
	if(!table) return NULL;
	const htable_node *node = new_htable_node(key, key_method, &(htable_node){});
	size_t at = node->hash % table->m_max_size;
	if(table->m_array[at]){
#ifdef SPLAY_PROCESS
		return splay_find(table->m_array[at], (Object*)node, &node->method->compare);
#else
		return scapegoat_find(table->m_array[at], (Object*)node, &node->method->compare);
#endif
	}
	return NULL;
}

BOOLEAN
htable_map(htable *table, const TRAVERSAL_STRATEGY strat, const BOOLEAN more_info, void* aux, const lMapFunc func){
	(void)more_info;//TODO implement more info
	if(!func) return FALSE;
	if(!table) return TRUE;
	size_t i = 0;
	size_t processed = 0;
	for(; i < table->m_max_size; i++){
		if(table->m_array[i]){
			if(!btree_map(table->m_array[i]->root, strat, FALSE, aux, func)) return FALSE;
			processed += table->m_array[i]->size;
			if(processed >= table->size) break;
		}
	}
	return TRUE;
}


static BOOLEAN
htable_clear_f(htable_node* node, BOOLEAN *destroy){
	if(*destroy) CALL_VOID(node->data, destroy);
	return TRUE;
}

//Note the next "remove" operation, and many others following will be expensive!
static void
htable_clear_internal(htable *tbl, BOOLEAN destroy, BOOLEAN destroy_nodes){
	if(!tbl) return;
	size_t i = 0;
	size_t processed = 0;
	for(; i < tbl->m_max_size; i++){
	#ifdef SPLAY_PROCESS
		if(tbl->m_array[i]){
			if(destroy_nodes)
				btree_map(tbl->m_array[i]->root, DEPTH_FIRST_PRE, FALSE, &destroy, (lMapFunc)htable_clear_f);
			processed += tbl->m_array[i]->size;
			splay_clear(tbl->m_array[i], destroy_nodes);
			splaytree_destroy(tbl->m_array[i]);
			tbl->m_array[i] = NULL;
			if(processed >= tbl->size) break;
		}
	#else
		if(tbl->m_array[i]){
			if(destroy_nodes)
				btree_map(tbl->m_array[i]->root, DEPTH_FIRST_PRE, FALSE, &destroy, (lMapFunc)htable_clear_f);
			processed += tbl->m_array[i]->size;
			scapegoat_clear(tbl->m_array[i], destroy_nodes);
			scapegoat_destroy(tbl->m_array[i]);
			tbl->m_array[i] = NULL;
			if(processed >= tbl->size) break;
		}
	#endif
	}

	tbl->m_collision = 0;
	tbl->size = 0;
}

void
htable_clear(htable *tbl, BOOLEAN destroy){
	htable_clear_internal(tbl, destroy, TRUE);
}


//The below was adapted from the stack overflow post: http://codegolf.stackexchange.com/questions/10701/fastest-code-to-find-the-next-prime
#define ARRLENGTH(A) ( sizeof(A)/sizeof(A[0]) )

//legendre symbol (a|m)
//note: returns m-1 if a is a non-residue, instead of -1
static inline size_t
modular_pow(size_t base, size_t exp, size_t mod){
	size_t result = 0;
	while(exp > 0){
		if(exp % 2 == 1){
			result = (result*base)%mod;
		}
		exp >>= 1;
		base = base*base % mod;
	}
	return result;
}

#if 0
static inline size_t
legendre(size_t a, size_t m){
	return modular_pow(a, (m-1)>>1, m);
}

//strong probable prime
static inline BOOLEAN
is_sprp(size_t n, size_t b/*=2*/){
	size_t d = n-1, s = 0;
	while((d&1) == 0){
		s += 1;
		d >>= 1;
	}
	size_t x = modular_pow(b,d,n);
	if(x == 1 || x == n-1){
		return TRUE;
	}
	for(size_t r = 1; r < s; r++){
		x = x*x%n;
		if(x == 1)
			return FALSE;
		else if(x==n-1)
			return TRUE;
	}
	return FALSE;
}

// lucas probable prime
// assumes D = 1 (mod 4), (D|n) = -1
static inline BOOLEAN
is_lucas_prp(size_t n, size_t D){
	size_t P = 1, Q = (1-D) >> 2;
	//n+1 = 2**r*s where s is odd
	size_t s = n+1, r=0;
	while((s&1) == 0){
		r += 1;
		s >>= 1;
	}
	//calculate the bit reversal of (odd) s
	//e.g. 19 (10011) <=> 25 (11001)
	size_t t = 0;
	while(s > 0){
		if(s&1){
			t++;
			s--;
		} else {
			t <<= 1;
			s >>= 1;
		}
	}

	// use the same bit reversal process to calculate the sth Lucas number
	// keep track of q = Q**n as we go
	size_t U = 0, V = 2, q = 1;
	//mod_inv(2,n)
	size_t inv_2 = (n+1) >> 1;
	while(t > 0){
		if((t&1) == 1){
			//U,V of n+1
			size_t tU = ((U+V) * inv_2)%n;
			size_t tV = ((D*U + V)*inv_2)%n;
			U = tU; V = tV;
		} else {
			//U,V of n*2
			size_t tU = (U*V)%n;
			size_t tV = (V*V-2*q)%n;
			U = tU; V = tV;
			q = (q*q)%n;
			t >>= 1;
		}
	}
	//double s until we have the 2**r*sth Lucas number
	while(r > 0){
		size_t tU = (U*V)%n, tV=(V*V-2*q)%n;
		U = tU; V = tV;
		q = (q*q)%n;
		r--;
	}
	// primality check
	// if n is prime, n divides the n+1st Lucas number, given the assumptions
	return U == 0;
}

#endif

//primes less than 212
static const size_t small_primes[] = {
	    2,  3,  5,  7, 11, 13, 17, 19, 23, 29,
	   31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
	   73, 79, 83, 89, 97,101,103,107,109,113,
	  127,131,137,139,149,151,157,163,167,173,
	  179,181,191,193,197,199,211
};

//pre-calced sieve of eratosthenes for n = 2, 3, 5, 7
static const size_t indices[] = {
	    1, 11, 13, 17, 19, 23, 29, 31, 37, 41,
	   43, 47, 53, 59, 61, 67, 71, 73, 79, 83,
	   89, 97,101,103,107,109,113,121,127,131,
	  137,139,143,149,151,157,163,167,169,173,
	  179,181,187,191,193,197,199,209
};

//distances between sieve values
static const size_t offsets[] = {
	10, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6,
	 6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2, 4,
	 2, 4, 8, 6, 4, 6, 2, 4, 6, 2, 6, 6,
	 4, 2, 4, 6, 2, 6, 4, 2, 4, 2,10, 2
};

static inline size_t
bisect_size_t(size_t n, const size_t arr[], size_t length){
	size_t imin = 0, imax = length;
	while(imin < imax){
		size_t imid = (imin+imax)>>1;
		if(n > arr[imid]){
			imin = imid+1;
		} else {
			imax = imid;
		}
	}
	return imin;
}

static BOOLEAN
is_prime(size_t n){
	if(n < small_primes[ARRLENGTH(small_primes)-1]+1){
		size_t i = bisect_size_t(n, small_primes, ARRLENGTH(small_primes));//do a binary search on an array
		if(small_primes[i] == n) return TRUE;
	}
	for(size_t i = 0; i < ARRLENGTH(small_primes); i++){
		if(n%small_primes[i] == 0) return FALSE;
	}
	size_t i = small_primes[ARRLENGTH(small_primes)-1];
	//perform full trial division for however many bits size_t is
	while(i*i < n){
		for(size_t j = 0; j < ARRLENGTH(offsets); j++){
			i += offsets[j];
			if(n%i == 0)
				return FALSE;
		}
	}
	return TRUE;
	//if one wanted to abstract this further:
#if 0
	if(!is_sprp(n, 2)) return FALSE;
	int32_t a = 0, s = 2;
	while(legendre(a,n) != n-1){
		s = -s;
		a = s-a;
	}
	return is_lucas_prp(n,a);
#endif
}

//next prime strictly larger than n
static size_t
next_prime(size_t n){
	if(n < 2) return n;
	n = (n+1)|1;
	if(n < small_primes[ARRLENGTH(small_primes)-1]+1){
		while(TRUE){
			size_t i = bisect_size_t(n, small_primes, ARRLENGTH(small_primes));//do a binary search on an array
			if(small_primes[i] == n) return n;
			n+=2;
		}
	}
	size_t x = n%(small_primes[ARRLENGTH(small_primes)-1]-1);
	size_t m = bisect_size_t(x, indices, ARRLENGTH(indices));
	size_t i = n+(indices[m]-x);
	while(TRUE){
		for(size_t j = 0; j < ARRLENGTH(offsets); j++){
			if(is_prime(i)) return i;
			i += offsets[(j+m)%ARRLENGTH(offsets)];
		}
	}
}
