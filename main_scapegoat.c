#include "scapegoat.h"
#include "main.h"
#include <limits.h>
#include <math.h>

static const double alpha_weight = 0.75;

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
btree_print_map_f(aLong *data, lMapFuncAux*more, btree *node){
	printf("%*s%lu: %ld (%p,%p)\n", (int)more->depth, "", more->depth, data->data, node->left?node->left->data:NULL, node->right?node->right->data:NULL);
	return TRUE;
}


static void
test_insert(){
	scapegoat *t = scapegoat_new(alpha_weight, &aLong_type.compare);//an average scapegoat (0.5 would be a very rigorously balanced tree)
	long x[] = {1,0,-1};
	size_t i = 0;
	for(; i < ARRLENGTH(x); i++){
		scapegoat_insert(t, (Object*)aLong_new(x[i]), FALSE);
	}
	long expect[] = {-1, 0, 1};
	btree_dump_map_d buffer = {
		.max = ARRLENGTH(x),
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	btree_map(t->root, DEPTH_FIRST_IN, TRUE, &buffer, (lMapFunc)btree_dump_map_f);
	printf("In Order (Element Order): ");
	compare_arrs(&expect[0], buffer.max, &buffer.test1[0], buffer.pos);

	//btree_map(t->root, DEPTH_FIRST_IN, TRUE, NULL, (lMapFunc)btree_print_map_f);//I'm not sure what it's actually supposed to look like...
	scapegoat_clear(t, TRUE);
	scapegoat_destroy(t);
}

static void
test_insert_many(){
	scapegoat *t = scapegoat_new(alpha_weight, &aLong_type.compare);//an average scapegoat (0.5 would be a very rigorously balanced tree)
	//long x[] = {1,2,3,4,5,6,7,8,9,10};
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i = 0;
	size_t min, max, avg, nodes, leaves;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		scapegoat_insert(t, (Object*)aLong_new(i), FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	//btree_map(t->root, DEPTH_FIRST_PRE, TRUE, NULL, (lMapFunc)btree_print_map_f);//I'm not sure what it's actually supposed to look like...
	btree_info(t->root, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
	printf("Insert (many): %s\n", (NUMS-1 == nodes)?"PASS":"FAIL");
	scapegoat_clear(t, TRUE);
	scapegoat_destroy(t);
}

static void
test_remove(){
	scapegoat *t = scapegoat_new(alpha_weight, &aLong_type.compare);//an average scapegoat (0.5 would be a very rigorously balanced tree)
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	//long x[] = {1,2,3,4,5,6,7,8,9,10};
	size_t i = 0;
	for(; i < ARRLENGTH(x); i++){
		scapegoat_insert(t, (Object*)aLong_new(x[i]), FALSE);
	}

	aLong tmp = {
		.method = &aLong_type,
		.data = x[6]
	};
	aLong *mp = (aLong*)scapegoat_remove(t, (Object*)&tmp, &aLong_type.compare, FALSE);
	printf("Remove: ");
	if(mp && mp->data == tmp.data){
		printf("PASS\n");
	} else {
		printf("FAIL: %ld %ld\n", mp?mp->data:LONG_MIN, tmp.data);
	}
	mp->method->parent.destroy((Object*)mp);

	//btree_map(t->root, DEPTH_FIRST_PRE, TRUE, NULL, (lMapFunc)btree_print_map_f);//I'm not sure what it's actually supposed to look like...
	scapegoat_clear(t, TRUE);
	scapegoat_destroy(t);

}

static void
test_find(){
	scapegoat *t = scapegoat_new(alpha_weight, &aLong_type.compare);//an average scapegoat (0.5 would be a very rigorously balanced tree)
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	//long x[] = {1,2,3,4,5,6,7,8,9,10};
	size_t i = 0;
	for(; i < ARRLENGTH(x); i++){
		scapegoat_insert(t, (Object*)aLong_new(x[i]), FALSE);
	}

	aLong tmp = {
		.method = &aLong_type,
		.data = x[6]
	};
	aLong *f = (aLong*)scapegoat_find(t, (Object*)&tmp, &aLong_type.compare);
	printf("Find: ");
	if(f){
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}

	//btree_map(t->root, DEPTH_FIRST_PRE, TRUE, NULL, (lMapFunc)btree_print_map_f);//I'm not sure what it's actually supposed to look like...
	scapegoat_clear(t, TRUE);
	scapegoat_destroy(t);

}

void
test_scapegoat(){
	test_insert();
#if 0
	{
		struct rusage start, end;
		getrusage(RUSAGE_SELF, &start);
		test_performance();
		getrusage(RUSAGE_SELF, &end);
		struct timeval diff;
		timersub(&end.ru_utime, &start.ru_utime, &diff);
		printf("Insert time: %lf\n", diff.tv_sec+diff.tv_usec/1e9);
	}
#endif
	test_insert_many();
	test_remove();
	//test_remove_many();
	//test_remove_root();
	test_find();
}
