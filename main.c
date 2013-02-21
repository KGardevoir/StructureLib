#include <stdio.h>
#include <math.h>
#include "linked_structures.h"
#define MAX(a,b) ({ typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ typeof(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define ARRLENGTH(A) ( sizeof(A)/sizeof(typeof(A[0])) )
long long_cmp(void *a, void *b){ return (long)((long)(a) - (long)(b)); }

BOOLEAN
compare_arrs(const long *expect, const size_t length, const long *got, const size_t got_length){
	size_t i = 0;
	if(length != got_length){
		printf("FAILURE, inconsitent number of nodes (Got: %ld, Expected: %ld)\n", got_length, length);
		return FALSE;
	}
	for(; i < length; i++){
		if(expect[i] != got[i]){
			printf("FAILURE, value %lu does not match (Got: (", i);
			for(i=0; i < length; i++)
				printf("%ld ", got[i]);
			printf("), Expected: (");
			for(i=0; i < length; i++)
				printf("%ld ", expect[i]);
			printf(")\n");
			return FALSE;
		}
	}
	printf("PASS\n");
	return TRUE;
}

dlist*
print_list(dlist* list){
	dlist* run = list;
	do {
		printf("<%p, %p %p>: %ld\n", run->prev, run, run->next, (long)run->data);
		run = run->next;
	} while(run != list);
	return list;
}

//extern void print_bstree_structure(bstree*, list_tspec*);

void
test_splay(){
	splaytree *t = NULL; list_tspec type = {.compar = (lCompare)long_cmp};
	const size_t NUMS = 40000;
	const size_t GAP  = 307;
	printf("Checking Splay Tree -----------------------------\n");
	//printf("Checking... (no bad output means success)--------\n");
	{
		long i;
		size_t min, max, avg, nodes, leaves;
		for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
			t = splay_insert(t, (void*)i, FALSE, &type);
		bstree_info(t, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
		printf("Inserts: %s\n", (NUMS == nodes)?"PASS":"FAIL");
		//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
		//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
		for(i = 1; i < NUMS; i += 2)
			t = splay_remove(t, (void*)i, NULL, FALSE, &type);
		bstree_info(t, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
		printf("Removes: %s\n", (NUMS/2 == nodes)?"PASS":"FAIL");
		//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
		//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
	}

	{
		long i;
		t = splay_find(t, bstree_findmin(t)->data, &type); i = (long)t->data;
		long nm = (long)bstree_successor(t, t, &type)->data;
		long j;
		t = splay_find(t, bstree_findmax(t)->data, &type); j = (long)t->data;
		long k = (long)bstree_predessor(t, t, &type)->data;
		if(i != 2 || nm != 4 || j != NUMS-2 || k != NUMS-4)
			printf("FindMin or FindMax error! Got %lu and %lu, head successor: %lu, tail predessor: %lu\n", i, j, k, nm);
	}
	{
		long i;
		for(i = 2; i < NUMS; i+=2){
			long k;
			t = splay_find(t, (void*)i, &type); k = (long)t->data;
			if(k != i)
				printf("Error: find fails for %ld\n",i);
		}
		for(i = 1; i < NUMS; i+=2){
			long k;
			t = splay_find(t, (void*)i, &type); k = (long)t->data;
			if(k == i)
				printf("Error: Found deleted item %ld\n",i);
		}
	}
	bstree_clear(t,FALSE,NULL);
	printf("Finished Checking Splay Trees -------------------\n");
}

dlist*
make_dlist(const long a[], size_t s, list_tspec* type){
	long i = 0;
	dlist* newl = NULL;
	for(; i < s; i++){
		newl = dlist_append(newl, (void*)a[i], 0, type);
	}
	return newl;
}
#define DLIST_TEST_SIZE 12
typedef struct dlist_dump_map_f {
	size_t pos;
	size_t max;
	long test1[DLIST_TEST_SIZE];
	long test2[DLIST_TEST_SIZE];
} dlist_dump_map_d;

BOOLEAN
dlist_dump_map_f(void *data, lMapFuncAux *more){
	//printf("%*s%-4lu: %ld\n", (int)aux->depth, "", aux->depth, node);
	dlist_dump_map_d *aux = more->aux;
	aux->test1[aux->pos] = (long)data;
	aux->test2[aux->pos] = (long)more->position;
	aux->pos++;
	if(aux->pos >= aux->max)
		return FALSE;
	return TRUE;
}

void
test_dlist(){
	printf("Checking dlists ---------------------------------\n");
	list_tspec list_type = {
		.destroy = NULL,
		.compar = (lCompare)long_cmp,
		.key_compar = (lKeyCompare)long_cmp,
		.deep_copy = NULL
	};
	{
		const long expect1[] = { 1, 3, 5, 6, 9, 10, 13 };
		const long expect2[] = { 0, 1, 2, 3, 4,  5,  6 };
		dlist_dump_map_d buffer = {
			.max = ARRLENGTH(expect1),
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		dlist *l1 = make_dlist(expect1, buffer.max, &list_type);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 1 (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 1 (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, FALSE, &list_type);
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
		dlist *l1 = make_dlist(expect1, buffer.max, &list_type);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 2 (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 2 (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, FALSE, &list_type);
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
		dlist *l1 = make_dlist(expect1, ARRLENGTH(expect1), &list_type);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 3 (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 3 (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, FALSE, &list_type);
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
		dlist *l1 = make_dlist(init, buffer.max, &list_type);
		l1 = dlist_sort(l1, &list_type);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 2 Sort (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 2 Sort (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, FALSE, &list_type);
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
		dlist *l1 = make_dlist(init, ARRLENGTH(init), &list_type);
		l1 = dlist_dequeue(l1, NULL, FALSE, &list_type);
		dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
		printf("List 2 Dequeue (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("List 2 Dequeue (Index Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
		dlist_clear(l1, FALSE, &list_type);
	}
	printf("Finished dlists ---------------------------------\n");
}

#define BSTREE_TEST_SIZE 8
typedef struct bstree_dump_map_d {
	size_t pos;
	size_t max;
	long test1[BSTREE_TEST_SIZE];
	long test2[BSTREE_TEST_SIZE];
} bstree_dump_map_d;

static BOOLEAN
bstree_dump_map_f(void* data, lMapFuncAux* more){
	//printf("%*s%-4lu: %ld\n", (int)aux->depth, "", aux->depth, node);
	bstree_dump_map_d *aux = more->aux;
	aux->test1[aux->pos] = (long)data;
	aux->test2[aux->pos] = (long)more->depth;
	aux->pos++;
	if(aux->pos >= aux->max)
		return FALSE;
	return TRUE;
}


void
test_bstree(){
	bstree *t1 = NULL, *t2 = NULL, *t3 = NULL; list_tspec type = {.compar = (lCompare)long_cmp};
	printf("Checking bstree ---------------------------------\n");
	//long x1[] = { 1, 3, 5, 6, 9, 10, 13 };
	//long x2[] = { 7, 6, 5, 4, 3, 2, 1, 8, 11, 14, 9, 10 };
	long x2[] = {8, 3, 1, 6, 4, 7, 10, 14, 13};
	//long x3[] = { 3, 4, 7, 8, 11, 12, 14, 15, 16, 17, 18 };
	size_t i = 0;
	for(; i < sizeof(x2)/sizeof(x2[0]); i++){
		t1 = bstree_insert(t1, (void*)x2[i], FALSE, &type);
	}
	//for(i = 0; i < sizeof(x2)/sizeof(x2[0]); i++){
	//	t2 = splay_insert(t2, (void*)x2[i], FALSE, &type);
	//}
	//for(i = 0; i < sizeof(x3)/sizeof(x3[0]); i++){
	//	t3 = splay_insert(t3, (void*)x3[i], FALSE, &type);
	//}
	bstree *f = bstree_find(t1, (void*)x2[5], &type);
	long mp;
	if((long)f->data != x2[5])
		printf("Error: Not able to find element\n");
	t1 = bstree_remove(t1, (void*)x2[5], (void**)&mp, FALSE, &type);
	if(mp != x2[5])
		printf("Error: Unable to remove element\n");
	printf("Tree Structures:\n");
	{
		const long expect1[] = {8,3,1,6,4,10,14,13};
		const long expect2[] = {1,2,3,3,4,2,3,4};
		bstree_dump_map_d buffer = {
			.max = BSTREE_TEST_SIZE,
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		bstree_map(t1, DEPTH_FIRST_PRE, TRUE, &buffer, (lMapFunc)bstree_dump_map_f);
		printf("Pre Order (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("Pre Order (Level Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	}
	{
		const long expect1[] = {1,3,4,6,8,10,13,14};
		const long expect2[] = {3,2,4,3,1,2,4,3};
		bstree_dump_map_d buffer = {
			.max = BSTREE_TEST_SIZE,
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		bstree_map(t1, DEPTH_FIRST_IN, TRUE, &buffer, (lMapFunc)bstree_dump_map_f);
		printf("In Order (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("In Order (Level Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	}
	{
		const long expect1[] = {1,4,6,3,13,14,10,8};
		const long expect2[] = {2,3,2,1,3,2,1,0};
		bstree_dump_map_d buffer = {
			.max = BSTREE_TEST_SIZE,
			.pos = 0,
			.test1 = {0},
			.test2 = {0}
		};
		bstree_map(t1, DEPTH_FIRST_POST, TRUE, &buffer, (lMapFunc)bstree_dump_map_f);
		printf("Post Order (Element Order): ");
		compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
		printf("Post Order (Level Order): ");
		compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	}
	size_t min, max, avg, nodes;
	bstree_info(t1, &min, &max, &avg, NULL, &nodes, NULL, NULL);
	//printf("Tree Statistics: min:%lu, max:%lu, avg:%lu, size: %lu\n", min, max, avg, nodes);
	bstree_clear(t1, FALSE, &type);
	bstree_clear(t2, FALSE, &type);
	bstree_clear(t3, FALSE, &type);
	printf("Finished bstree ---------------------------------\n");
}


#define GRAPH_TEST_SIZE 7
typedef struct graph_dump_map_d {
	size_t pos;
	size_t max;
	long test[GRAPH_TEST_SIZE];
} graph_dump_map_d;

static BOOLEAN
graph_dump_map_f(void* data, graph_dump_map_d* aux){
	aux->test[aux->pos] = (long)data;
	aux->pos++;
	if(aux->pos >= aux->max)
		return FALSE;
	return TRUE;
}

void
test_graph(){
	printf("Checking graphs ---------------------------------\n");
	long nums[] = {0,1,2,3,4,5,6};
	graph *g[] = {
		graph_insert(NULL, (void*)nums[0], FALSE, NULL),
		graph_insert(NULL, (void*)nums[1], FALSE, NULL),
		graph_insert(NULL, (void*)nums[2], FALSE, NULL),
		graph_insert(NULL, (void*)nums[3], FALSE, NULL),
		graph_insert(NULL, (void*)nums[4], FALSE, NULL),
		graph_insert(NULL, (void*)nums[5], FALSE, NULL),
		graph_insert(NULL, (void*)nums[6], FALSE, NULL)
	};
	graph_link(g[0], g[1], NULL); graph_link(g[0], g[4], NULL); graph_link(g[0], g[6], NULL);
	graph_link(g[1], g[2], NULL); graph_link(g[1], g[5], NULL); graph_link(g[1], g[6], NULL);
	graph_link(g[2], g[3], NULL); graph_link(g[2], g[4], NULL); graph_link(g[2], g[4], NULL);
	graph_link(g[3], g[4], NULL);
	graph_link(g[4], g[0], NULL);
	graph_link(g[5], g[4], NULL);
	graph_link(g[6], g[5], NULL);
	{
		printf("Depth First Search: ");
		const long expect[] = {0,1,2,3,4,5,6};
		graph_dump_map_d buffer = {
			.max = GRAPH_TEST_SIZE,
			.pos = 0,
			.test = {0}
		};
		graph_map(g[0], DEPTH_FIRST, FALSE, &buffer, (lMapFunc)graph_dump_map_f);
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
		graph_map(g[0], BREADTH_FIRST, FALSE, &buffer, (lMapFunc)graph_dump_map_f);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		size_t nodes, edges;
		graph_size(g[0], &nodes, &edges);
		printf("Graph Size: %s\n", (nodes == GRAPH_TEST_SIZE && edges == 13)?"PASS":"FAIL");
	}

	graph_clear(g[0], FALSE, NULL);
	printf("Finished graphs ---------------------------------\n");
}

void
test_htable(){
	
}

int
main(int argc, char** argv){
	test_dlist();
	test_bstree();
	test_splay();
	test_graph();
	return 0;
}
