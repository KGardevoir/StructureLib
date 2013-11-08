#include "slist.h"
#include "main.h"

static void
print_list(slist* list){
	slist* run;
	SLIST_ITERATE(run, list){
		printf("<%p %p>: %ld\n", run, run->next, ((aLong*)run->data)->data);
		run = run->next;
	}
}

#define SLIST_TEST_SIZE 12
typedef struct slist_dump_map_d {
	size_t pos;
	size_t max;
	long test1[SLIST_TEST_SIZE];
	long test2[SLIST_TEST_SIZE];
} slist_dump_map_d;

static BOOLEAN
slist_dump_map_f(aLong *data, lMapFuncAux *more){
	//printf("%*s%-4lu: %ld\n", (int)aux->depth, "", aux->depth, node);
	slist_dump_map_d *aux = more->aux;
	aux->test1[aux->pos] = data->data;
	aux->test2[aux->pos] = (long)more->position;
	aux->pos++;
	if(aux->pos >= aux->max)
		return FALSE;
	return TRUE;
}

static slist*
make_list(const long arr[], size_t len){
	slist *list = NULL;
	for(size_t i = len; i--;){
		list = slist_pushfront(list, (Object*)aLong_new(arr[i]), FALSE);
	}
	return list;
}

static void
test_order(){
	const long expect1[] = { 1, 3, 5, 6, 9, 10, 13 };
	const long expect2[] = { 0, 1, 2, 3, 4,  5,  6 };
	slist_dump_map_d buffer = {
		.max = ARRLENGTH(expect1),
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	slist *l1 = make_list(&expect1[0], buffer.max);
	slist *run;
	size_t i = 0;
	SLIST_ITERATE(run, l1){
		buffer.test1[buffer.pos] = ((aLong*)run->data)->data;
		buffer.test2[buffer.pos] = i;
		buffer.pos++;
		if(buffer.pos >= buffer.max)
			break;
		i++;
	}
	printf("Element Order: ");
	compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
	printf("Index Order: ");
	compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	slist_clear(l1, TRUE);
}

static void
test_macros(){
	const long expect[] = { 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11 };
	slist *l = make_list(expect, ARRLENGTH(expect)), *run;
	long test_array[ARRLENGTH(expect)];
	size_t i = 0;
	SLIST_ITERATE(run, l){
		test_array[i] = ((aLong*)(run->data))->data;
		i++;
	}
	printf("Macro, Iterate Forward: "); compare_arrs(&expect[0], ARRLENGTH(expect), &test_array[0], i);
	slist_clear(l, TRUE);
}

static void
test_popback(){
	const long data[] = { 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11 };
	const long expect[] = { 1, 2, 3, 4, 5, 6, 7,  8,  9, 10 };
	slist *l = make_list(data, ARRLENGTH(data)), *run;
	long test_array[ARRLENGTH(expect)];
	size_t i = 0;
	l = slist_popback(l, NULL, TRUE);
	SLIST_ITERATE(run, l){
		test_array[i] = ((aLong*)(run->data))->data;
		i++;
	}
	printf("Popback: "); compare_arrs(&expect[0], ARRLENGTH(expect), &test_array[0], i);
	slist_clear(l, TRUE);
}

static void
test_reverse(){
	const long data[] = { 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11 };
	const long expect[] = { 11,10,9,8,7,6,5,4,3,2,1 };
	slist *l = make_list(data, ARRLENGTH(data)), *run;
	long test_array[ARRLENGTH(expect)];
	size_t i = 0;
	l = slist_reverse(l);
	SLIST_ITERATE(run, l){
		test_array[i] = ((aLong*)(run->data))->data;
		i++;
	}
	printf("Reverse: "); compare_arrs(&expect[0], ARRLENGTH(expect), &test_array[0], i);
	slist_clear(l, TRUE);
}

void
test_slist(){
	test_order();
	test_macros();
	test_popback();
	test_reverse();
}
