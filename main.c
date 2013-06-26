#include "linked_structures.h"
#include <stdio.h>
#include <math.h>

#define MAX(a,b) ({ typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ typeof(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define ARRLENGTH(A) ( sizeof(A)/sizeof(typeof(A[0])) )


typedef struct aLong_vtable {
	Object_vtable parent;
	Comparable_vtable compare;
} aLong_vtable;
typedef struct aLong {
	const aLong_vtable *method;
	long data;
} aLong;
static long aLong_compare(const aLong *self, const aLong *b){ return b->data - self->data; }
static void aLong_destroy(aLong *self) { LINKED_FREE(self); }

static aLong_vtable aLong_type = {
	.parent = {
		.hashable = NULL,
		.destroy = aLong_destroy,
		.copy = NULL,
		.equals = NULL,
		.size = sizeof(aLong)
	},
	.compare = {
		.compare = aLong_compare
	}
};

static aLong* aLong_new(long a){
	aLong init = {
		.method = &aLong_type,
		.data = a
	};
	aLong *lnew = (aLong*)LINKED_MALLOC(sizeof(init));
	memcpy(lnew, &init, sizeof(init));
	return lnew;
}

typedef struct aLong_comparator_vtable {
	Comparator_vtable compare;
} aLong_comparator_vtable;
typedef struct aLong_comparator {
	aLong_comparator_vtable *method;
} aLong_comparator;

static long aLong_comparator_compare(const aLong_comparator *self, const aLong *one, const aLong* two){
	return aLong_compare(one, two);
}

static aLong_comparator_vtable aLong_comparator_type = {
	.compare = {
		.compare = aLong_comparator_compare
	}
};


static BOOLEAN
compare_arrs(const long *expect, const size_t length, const long *got, const size_t got_length){
	size_t i = 0;
	size_t minlen = MIN(length, got_length);
	for(; i < minlen; i++){
		if(expect[i] != got[i]){
			printf("FAILURE, value %lu does not match (Got: (", i);
			for(i=0; i < got_length; i++)
				printf("%ld ", got[i]);
			printf("), Expected: (");
			for(i=0; i < length; i++)
				printf("%ld ", expect[i]);
			printf(")\n");
			return FALSE;
		}
	}
	if(length != got_length){
		printf("FAILURE, inconsitent number of nodes (Got: %ld, Expected: %ld)\n", got_length, length);
		return FALSE;
	}
	printf("PASS\n");
	return TRUE;
}

static dlist*
print_list(dlist* list){
	dlist* run = list;
	do {
		printf("<%p, %p %p>: %ld\n", run->prev, run, run->next, (long)run->data);
		run = run->next;
	} while(run != list);
	return list;
}

//extern void print_btree_structure(btree*, list_tspec*);

static void
test_splay(){
	splaytree *t = NULL;
	const size_t NUMS = 40000;
	const size_t GAP  = 307;
	printf("Checking Splay Tree -----------------------------\n");
	//printf("Checking... (no bad output means success)--------\n");
	{
		long i;
		size_t min, max, avg, nodes, leaves;
		for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
			t = splay_insert(t, (Object*)aLong_new(i), &aLong_type.compare, FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
		btree_info(t, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
		printf("Inserts: %s\n", (NUMS-1 == nodes)?"PASS":"FAIL");
		//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
		//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
		for(i = 1; i < NUMS; i += 2){
			aLong *anew = aLong_new(i);
			t = splay_remove(t, (Object*)anew, &aLong_type.compare, NULL, TRUE);
			LINKED_FREE(anew);
		}
		btree_info(t, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
		printf("Removes: %s\n", (NUMS/2-1 == nodes)?"PASS":"FAIL");
		//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
		//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
	}

	{
		long i;
		t = splay_find(t, btree_findmin(t)->data, &aLong_type.compare); i = ((aLong*)t->data)->data;
		long nm = ((aLong*)btree_successor(t, t, &aLong_type.compare)->data)->data;
		long j;
		t = splay_find(t, btree_findmax(t)->data, &aLong_type.compare); j = ((aLong*)t->data)->data;
		long k = ((aLong*)btree_predessor(t, t, &aLong_type.compare)->data)->data;
		if(i != 2 || nm != 4 || j != NUMS-2 || k != NUMS-4)
			printf("FindMin or FindMax error! Got %lu and %lu, head successor: %lu, tail predessor: %lu\n", i, j, k, nm);
	}
	{
		long i;
		for(i = 2; i < NUMS; i+=2){
			long k;
			aLong tmp = {
				.method = &aLong_type,
				.data = i
			};
			t = splay_find(t, (Object*)&tmp, &tmp.method->compare); k = ((aLong*)t->data)->data;
			if(k != i)
				printf("Error: find fails for %ld\n",i);
		}
		for(i = 1; i < NUMS; i+=2){
			long k;
			aLong tmp = {
				.method = &aLong_type,
				.data = i
			};
			t = splay_find(t, (Object*)&tmp, &aLong_type.compare); k = ((aLong*)t->data)->data;
			if(k == i)
				printf("Error: Found deleted item %ld\n",i);
		}
	}
	btree_clear(t,TRUE);
	printf("Finished Checking Splay Trees -------------------\n");
}

static dlist*
make_dlist(const long a[], size_t s){
	long i = 0;
	dlist* newl = NULL;
	for(; i < s; i++){
		newl = dlist_append(newl, (Object*)aLong_new(a[i]), FALSE);
	}
	return newl;
}
#define DLIST_TEST_SIZE 12
typedef struct dlist_dump_map_d {
	size_t pos;
	size_t max;
	long test1[DLIST_TEST_SIZE];
	long test2[DLIST_TEST_SIZE];
} dlist_dump_map_d;

static BOOLEAN
dlist_dump_map_f(aLong *data, lMapFuncAux *more){
	//printf("%*s%-4lu: %ld\n", (int)aux->depth, "", aux->depth, node);
	dlist_dump_map_d *aux = more->aux;
	aux->test1[aux->pos] = data->data;
	aux->test2[aux->pos] = (long)more->position;
	aux->pos++;
	if(aux->pos >= aux->max)
		return FALSE;
	return TRUE;
}

static void
test_dlist(){
	printf("Checking dlists ---------------------------------\n");
	{
		const long expect1[] = { 1, 3, 5, 6, 9, 10, 13 };
		const long expect2[] = { 0, 1, 2, 3, 4,  5,  6 };
		dlist_dump_map_d buffer = {
			.max = ARRLENGTH(expect1),
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		dlist *l1 = make_dlist(expect1, buffer.max);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 1 (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 1 (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, TRUE);
	}
	{
		const long expect1[] = { 7, 6, 5, 4, 3, 2, 1, 8, 11, 14,  9, 10 };
		const long expect2[] = { 0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11 };
		dlist_dump_map_d buffer = {
			.max = ARRLENGTH(expect1),
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		dlist *l1 = make_dlist(expect1, buffer.max);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 2 (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 2 (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, TRUE);
	}
	{
		const long expect1[] = { 3, 4, 7, 8, 11, 12, 14, 15, 16, 17, 18 };
		const long expect2[] = { 0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11 };
		dlist_dump_map_d buffer = {
			.max = ARRLENGTH(expect1),
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		dlist *l1 = make_dlist(expect1, buffer.max);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 3 (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 3 (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, TRUE);
		l1 = NULL;
	}
	{
		const long expect1[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14 };
		const long expect2[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11 };
		const long init[]    = { 7, 6, 5, 4, 3, 2, 1, 8, 11, 14,  9, 10 };
		dlist_dump_map_d buffer = {
			.max = ARRLENGTH(expect1),
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		aLong_comparator cmp = {
			.method = &aLong_comparator_type
		};
		dlist *l1 = make_dlist(init, buffer.max);
		l1 = dlist_sort(l1, &cmp, &aLong_comparator_type.compare);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 2 Sort (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 2 Sort (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, TRUE);
		l1 = NULL;
	}
	{
		const long expect1[] = { 6, 5, 4, 3, 2, 1, 8, 11, 14,  9, 10 };
		const long expect2[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11 };
		const long init[]    = { 7, 6, 5, 4, 3, 2, 1, 8, 11, 14,  9, 10 };
		dlist_dump_map_d buffer = {
			.max = ARRLENGTH(expect1),
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		dlist *l1 = make_dlist(init, ARRLENGTH(init));
		l1 = dlist_dequeue(l1, NULL, TRUE);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 2 Dequeue (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 2 Dequeue (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, TRUE);
		l1 = NULL;
	}
	printf("Finished dlists ---------------------------------\n");
}

#define BSTREE_TEST_SIZE 32
typedef struct btree_dump_map_d {
	size_t pos;
	size_t max;
	long test1[BSTREE_TEST_SIZE];
	long test2[BSTREE_TEST_SIZE];
} btree_dump_map_d;

static BOOLEAN
btree_dump_map_f(aLong* data, lMapFuncAux* more){
	//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
	btree_dump_map_d *aux = more->aux;
	aux->test1[aux->pos] = data->data;
	aux->test2[aux->pos] = (long)more->depth;
	aux->pos++;
	if(aux->pos >= aux->max)
		return FALSE;
	return TRUE;
}

static BOOLEAN
btree_print_map_f(aLong *data, lMapFuncAux*more){
	printf("%*s%lu: %ld\n", (int)more->depth, "", more->depth, data->data);
	return TRUE;
}


static void
test_btree(){
	btree *t1 = NULL, *t2 = NULL, *t3 = NULL;
	printf("Checking btree ---------------------------------\n");
	long x1[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	//long x2[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	long x2[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
	//long x2[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
	const size_t x1_size = sizeof(x1)/sizeof(x1[0]);
	const size_t x2_size = sizeof(x2)/sizeof(x2[0]);
	//long x3[] = { 3, 4, 7, 8, 11, 12, 14, 15, 16, 17, 18 };
	size_t i = 0;
	for(i = 0; i < sizeof(x1)/sizeof(x1[0]); i++){
		t1 = btree_insert(t1, (Object*)aLong_new(x1[i]), &aLong_type.compare, FALSE);
	}
	for(i = 0; i < sizeof(x2)/sizeof(x2[0]); i++){
		t2 = btree_insert(t2, (Object*)aLong_new(x2[i]), &aLong_type.compare, FALSE);
	}
	//for(i = 0; i < sizeof(x2)/sizeof(x2[0]); i++){
	//	t2 = splay_insert(t2, (void*)x2[i], FALSE, &type);
	//}
	//for(i = 0; i < sizeof(x3)/sizeof(x3[0]); i++){
	//	t3 = splay_insert(t3, (void*)x3[i], FALSE, &type);
	//}
	aLong tmp = {
		.method = &aLong_type,
		.data = x1[5]
	};
	btree *f = btree_find(t1, (Object*)&tmp, &aLong_type.compare);
	aLong *mp;
	if(((aLong*)f->data)->data != x1[5])
		printf("Error: Not able to find element\n");
	t1 = btree_remove(t1, (Object*)&tmp, &aLong_type.compare, (Object**)&mp, FALSE);
	if(mp->data != x1[5])
		printf("Error: Unable to remove element\n");
	mp->method->parent.destroy((Object*)mp);
	printf("Tree Structures:\n");
	{
		const long expect1[] = {8,3,1,6,4,10,14,13};
		const long expect2[] = {0,1,2,2,3,1,2,3};
		btree_dump_map_d buffer = {
			.max = x1_size-1,
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		btree_map(t1, DEPTH_FIRST_PRE, TRUE, &buffer, (lMapFunc)btree_dump_map_f);
		printf("Pre Order (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("Pre Order (Level Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	}
	{
		const long expect1[] = {1,3,4,6,8,10,13,14};
		const long expect2[] = {2,1,3,2,0,1,3,2};
		btree_dump_map_d buffer = {
			.max = x1_size-1,
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		btree_map(t1, DEPTH_FIRST_IN, TRUE, &buffer, (lMapFunc)btree_dump_map_f);
		printf("In Order (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("In Order (Level Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	}
	{
		const long expect1[] = {1,4,6,3,13,14,10,8};
		const long expect2[] = {2,3,2,1,3,2,1,0};
		btree_dump_map_d buffer = {
			.max = x1_size-1,
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		btree_map(t1, DEPTH_FIRST_POST, TRUE, &buffer, (lMapFunc)btree_dump_map_f);
		printf("Post Order (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("Post Order (Level Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	}
#if 1
	{
		size_t idepth, isize, fdepth, fsize;
		btree_info(t1, NULL, &idepth, NULL, NULL, &isize, NULL, NULL);
		//btree_map(t1, DEPTH_FIRST_IN, TRUE, NULL, (lMapFunc)btree_print_map_f);
		t1 = btree_balance(t1);
		btree_info(t1, NULL, &fdepth, NULL, NULL, &fsize, NULL, NULL);
		BOOLEAN pass = (fdepth<=idepth)&&fsize==isize&&fdepth+1==(size_t)ceil(log(fsize+1)/log(2));
		printf("Tree 1 Rebalanced: ");
		if(pass)
			printf("PASS\n");
		else
			printf("FAIL (size,depth):i(%lu, %lu), f(%lu, %lu); opt:(%lu)\n", isize, idepth, fsize, fdepth, (size_t)ceil(log(fsize+1)/log(2)));
		//btree_map(t1, DEPTH_FIRST_IN, TRUE, NULL, (lMapFunc)btree_print_map_f);
	}
#endif
#if 1
	{
		//a very linear tree
		t2 = btree_balance(t2);
		size_t idepth = 0, isize = 0, fdepth = 0, fsize = 0;
		btree_info(t2, NULL, &idepth, NULL, NULL, &isize, NULL, NULL);
		//printf("Tree 2 Balanced (%ld nodes, %ld depth)...\n", size, max);//TODO test this better
		//btree_map(t2, DEPTH_FIRST_IN, TRUE, NULL, (lMapFunc)btree_print_map_f);
		btree_info(t2, NULL, &fdepth, NULL, NULL, &fsize, NULL, NULL);
		BOOLEAN pass = (fdepth<=idepth)&&fsize==isize&&(fdepth+1==(size_t)ceil(log(fsize+1)/log(2)));
		printf("Tree 2 Rebalanced: ");
		if(pass)
			printf("PASS\n");
		else
			printf("FAIL (size,depth):i(%lu, %lu), f(%lu, %lu); opt:(%lu)\n", isize, idepth, fsize, fdepth, (size_t)ceil(log(fsize+1)/log(2)));
	}
#endif
	btree_clear(t1, TRUE);
	btree_clear(t2, TRUE);
	btree_clear(t3, TRUE);
	printf("Finished btree ---------------------------------\n");
}


#define GRAPH_TEST_SIZE 7
typedef struct graph_dump_map_d {
	size_t pos;
	size_t max;
	long test[GRAPH_TEST_SIZE];
} graph_dump_map_d;

static BOOLEAN
graph_dump_map_f(aLong* data, lMapFuncAux* more){
	graph_dump_map_d *aux = more->aux;
	if(more->position == 0) printf("\n");
	//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
	aux->test[aux->pos] = data->data;
	aux->pos++;
	if(aux->pos >= aux->max)
		return FALSE;
	return TRUE;
}

static void
test_graph(){
	printf("Checking graphs ---------------------------------\n");
	long nums[] = {0,1,2,3,4,5,6};
	graph *g[] = {
		graph_insert(NULL, (Object*)aLong_new(nums[0]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[1]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[2]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[3]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[4]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[5]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[6]), FALSE)
	};
	graph_link(g[0], g[1]); graph_link(g[0], g[4]); graph_link(g[0], g[6]);
	graph_link(g[1], g[2]); graph_link(g[1], g[5]); graph_link(g[1], g[6]);
	graph_link(g[2], g[3]); graph_link(g[2], g[4]); graph_link(g[2], g[4]);
	graph_link(g[3], g[4]);
	graph_link(g[4], g[0]);
	graph_link(g[5], g[4]);
	graph_link(g[6], g[5]);
	{
		printf("Depth First Search (PRE): ");
		const long expect[] = {0,1,2,3,4,5,6};
		graph_dump_map_d buffer = {
			.max = GRAPH_TEST_SIZE,
			.pos = 0,
			.test = {0}
		};
		graph_map(g[0], DEPTH_FIRST, TRUE, TRUE, &buffer, (lMapFunc)graph_dump_map_f);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Depth First Search (POST): ");
		const long expect[] = {4,3,2,5,6,1,0};
		graph_dump_map_d buffer = {
			.max = GRAPH_TEST_SIZE,
			.pos = 0,
			.test = {0}
		};
		graph_map(g[0], DEPTH_FIRST_POST, TRUE, TRUE, &buffer, (lMapFunc)graph_dump_map_f);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Breadth First Search: ");
		const long expect[] = {0,1,4,6,2,5,3};
		graph_dump_map_d buffer = {
			.max = GRAPH_TEST_SIZE,
			.pos = 0,
			.test = {0}
		};
		graph_map(g[0], BREADTH_FIRST, TRUE, TRUE, &buffer, (lMapFunc)graph_dump_map_f);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		size_t nodes, edges;
		graph_size(g[0], &nodes, &edges);
		printf("Graph Size: %s\n", (nodes == GRAPH_TEST_SIZE && edges == 13)?"PASS":"FAIL");
	}

	graph_clear(g[0], TRUE); //FIXME since this is apparently unimplented, we leak tons of memory
	printf("Finished graphs ---------------------------------\n");
}

static void
test_htable(){
	
}

int
main(int argc, char** argv){
	test_dlist();
	test_btree();
	test_splay();
	test_graph();
	return 0;
}
