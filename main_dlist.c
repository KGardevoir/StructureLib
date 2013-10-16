#include "dlist.h"
#include "main.h"

#define MIN(a,b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define ARRLENGTH(A) ( sizeof(A)/sizeof(__typeof__(A[0])) )

static BOOLEAN
compare_arrs(const long *expect, const size_t length, const long *got, const size_t got_length){
	size_t i = 0;
	size_t minlen = MIN(length, got_length);
	BOOLEAN failure = FALSE;
	for(; i < minlen; i++){
		if(expect[i] != got[i]){
			printf("FAILURE, value %lu does not match (Got: (", i);
			for(i=0; i < got_length; i++)
				printf("%ld ", got[i]);
			printf("), Expected: (");
			for(i=0; i < length; i++)
				printf("%ld ", expect[i]);
			printf(")\n");
			failure = TRUE;
		}
	}
	if(length != got_length){
		printf("FAILURE, inconsitent number of nodes (Got: %ld, Expected: %ld)\n", got_length, length);
	}
	if(!failure) printf("PASS\n");
	return !failure;
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

static dlist*
make_dlist(const long a[], size_t s){
	size_t i = 0;
	dlist* newl = NULL;
	for(; i < s; i++){
		newl = dlist_pushback(newl, (Object*)aLong_new(a[i]), FALSE);
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
test_order(){
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

static void
test_order2(){
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

static void
test_order3(){
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
}

static void
test_sort(){
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
}

static void
test_popback(){
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
	l1 = dlist_popfront(l1, NULL, TRUE);
	dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
	printf("List 2 Popback (Element Order): ");
	compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
	printf("List 2 Popback (Index Order): ");
	compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	dlist_clear(l1, TRUE);
}

static void
test_reverse(){
	const long expect1[] = { 1, 2, 3, 4, 5 };
	const long expect2[] = { 2, 1 };
	const long init1[] = { 5, 4, 3, 2, 1};
	const long init2[] = { 1, 2 } ;
	dlist_dump_map_d buffer = {
		.max = ARRLENGTH(expect1),
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	dlist *l1 = make_dlist(init1, ARRLENGTH(init1));
	l1 = dlist_reverse(l1);
	dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
	printf("List 1 Reverse: "); compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
	dlist_clear(l1, TRUE);
	l1 = NULL;
	memset(&buffer, 0, sizeof(buffer));
	buffer.max = ARRLENGTH(init2);
	l1 = make_dlist(init2, buffer.max);
	l1 = dlist_reverse(l1);
	dlist_map(l1, TRUE, &buffer, (lMapFunc)dlist_dump_map_f);
	dlist_clear(l1, TRUE);
	printf("List 2 Reverse: "); compare_arrs(&expect2[0], buffer.max, &buffer.test1[0], buffer.pos);
}

void
test_dlist(){
	test_order();
	test_order2();
	test_order3();
	test_sort();
	test_popback();
	test_reverse();
}

