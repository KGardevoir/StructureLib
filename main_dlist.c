#include "dlist.h"
#include "main.h"

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
	dlist *run;
	size_t i = 0;
	DLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
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
	dlist *run;
	size_t i = 0;
	DLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
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
	dlist *run;
	size_t i = 0;
	DLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
	printf("List 3 (Element Order): ");
	compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
	printf("List 3 (Index Order): ");
	compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	dlist_clear(l1, TRUE);
}

static void
test_macros(){
	const long expect[] = { 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11 };
	const long expect2[] = { 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
	dlist *l = make_dlist(expect, ARRLENGTH(expect)), *run;
	long test_array[ARRLENGTH(expect)];
	size_t i = 0;
	DLIST_ITERATE(run, l){
		test_array[i] = ((aLong*)(run->data))->data;
		i++;
	}
	printf("Macro, Iterate Forward: "); compare_arrs(&expect[0], ARRLENGTH(expect), &test_array[0], i);
	i = 0;
	DLIST_ITERATE_REVERSE(run, dlist_tail(l)){
		test_array[i] = ((aLong*)(run->data))->data;
		i++;
	}
	printf("Macro, Iterate Backward: "); compare_arrs(&expect2[0], ARRLENGTH(expect2), &test_array[0], i);
	dlist_clear(l, TRUE);
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
	dlist *run;
	size_t i = 0;
	DLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
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
	dlist *run;
	size_t i = 0;
	DLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
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
	dlist *run;
	size_t i = 0;
	DLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
	printf("List 1 Reverse: "); compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
	dlist_clear(l1, TRUE);
	l1 = NULL;
	memset(&buffer, 0, sizeof(buffer));
	buffer.max = ARRLENGTH(init2);
	l1 = make_dlist(init2, buffer.max);
	l1 = dlist_reverse(l1);
	DLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
	dlist_clear(l1, TRUE);
	printf("List 2 Reverse: "); compare_arrs(&expect2[0], buffer.max, &buffer.test1[0], buffer.pos);
}

void
test_dlist(){
	test_order();
	test_order2();
	test_order3();
	test_sort();
	test_macros();
	test_popback();
	test_reverse();
}

