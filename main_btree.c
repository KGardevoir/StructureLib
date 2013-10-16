#include "btree.h"
#include <math.h>
#include "main.h"
#include <limits.h>

#define MIN(a,b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define ARRLENGTH(A) ( sizeof(A)/sizeof(typeof(A[0])) )

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
test_find(){
	btree *t = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i = 0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t = btree_insert(t, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	}
	aLong tmp = {
		.method = &aLong_type,
		.data = x[6]
	};
	btree *f = btree_find(t, (Object*)&tmp, &aLong_type.compare);
	printf("Find: ");
	if(f && ((aLong*)f->data)->data == x[6]){
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}
	btree_clear(t, TRUE);
}

static void
test_insert(){
	btree *t = NULL;
	long v[] = {0, -1, 1};
	size_t i = 1;
	for(i = 0; i < ARRLENGTH(v); i++)
		t = btree_insert(t, (Object*)aLong_new(v[i]), &aLong_type.compare, FALSE);
	printf("Insert: ");
	size_t fail_mode = 0;
	if( !t->left || ((aLong*)t->left->data)->data != v[1] ){
		fail_mode |= 1;
	}
	if( !t->right || ((aLong*)t->right->data)->data != v[2] ){
		fail_mode |= 2;
	}
	if( ((aLong*)t->data)->data != v[0] ) {
		fail_mode |= 4;
	}
	if(fail_mode){
		printf("FAIL ");
	} else {
		printf("PASS");
	}
	if(fail_mode & 1){
		printf("(LHS incorrect)");
	}
	if(fail_mode & 2){
		printf("(RHS incorrect)");
	}
	if(fail_mode & 4){
		printf("(root incorrect)");
	}
	printf("\n");
	btree_clear(t, TRUE);
}

static void
test_lots_insert(){
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i = 0;
	btree *t = NULL;
	for(i = 0; i < ARRLENGTH(x); i++)
		t = btree_insert(t, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	size_t size = 0;
	btree_info(t, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	printf("Insert (Many): ");
	if(size != ARRLENGTH(x)){
		printf("FAIL");
	} else {
		printf("PASS");
	}
	printf("\n");
}

static void
test_remove(){
	btree *t = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i = 0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t = btree_insert(t, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	}
	aLong tmp = {
		.method = &aLong_type,
		.data = x[6]
	};
	aLong *mp=NULL;

	//btree_map(t, DEPTH_FIRST_PRE, TRUE, NULL, (lMapFunc)btree_print_map_f);

	btree *f = btree_remove(t, (Object*)&tmp, &aLong_type.compare, (Object**)&mp, FALSE);
	printf("Remove: ");
	if(f && mp->data == tmp.data){
		printf("PASS\n");
		t = f;
	} else {
		printf("FAIL: %p %ld %ld\n", f, mp?mp->data:LONG_MIN, tmp.data);
	}
	btree_clear(t, TRUE);
}

static void
test_map_pre(){
	btree *t1 = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i=  0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	}
	const long expect2[] = {0,1,2,2,3,3,1,2,3};
	btree_dump_map_d buffer = {
		.max = ARRLENGTH(x)-1,
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	btree_map(t1, DEPTH_FIRST_PRE, TRUE, &buffer, (lMapFunc)btree_dump_map_f);
	printf("Pre Order (Element Order): ");
	compare_arrs(&x[0], buffer.max, &buffer.test1[0], buffer.pos);
	printf("Pre Order (Level Order): ");
	compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	btree_clear(t1, TRUE);
}

static void
test_map_infix(){
	btree *t1 = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i=  0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	}
	const long expect1[] = {1,3,4,6,7,8,10,13,14};
	const long expect2[] = {2,1,3,2,3,0,1,3};
	btree_dump_map_d buffer = {
		.max = ARRLENGTH(x)-1,
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	btree_map(t1, DEPTH_FIRST_IN, TRUE, &buffer, (lMapFunc)btree_dump_map_f);
	printf("In Order (Element Order): ");
	compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
	printf("In Order (Level Order): ");
	compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	btree_clear(t1, TRUE);
}

static void
test_map_post(){
	btree *t1 = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i=  0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	}
	const long expect1[] = {1,4,7,6,3,13,14,10,8};
	const long expect2[] = {2,3,3,2,1,3,2,1};
	btree_dump_map_d buffer = {
		.max = ARRLENGTH(x)-1,
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	btree_map(t1, DEPTH_FIRST_POST, TRUE, &buffer, (lMapFunc)btree_dump_map_f);
	printf("Post Order (Element Order): ");
	compare_arrs(&expect1[0], buffer.max, &buffer.test1[0], buffer.pos);
	printf("Post Order (Level Order): ");
	compare_arrs(&expect2[0], buffer.max, &buffer.test2[0], buffer.pos);
	btree_clear(t1, TRUE);
}

static void
test_info(){
	btree *t1 = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i =  0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	}
	size_t idepth, isize, fdepth, fsize;
	btree_info(t1, NULL, &idepth, NULL, NULL, &isize, NULL, NULL);
	t1 = btree_balance(t1);
	//btree_map(t1, DEPTH_FIRST_IN, TRUE, NULL, (lMapFunc)btree_print_map_f);
	btree_info(t1, NULL, &fdepth, NULL, NULL, &fsize, NULL, NULL);
	BOOLEAN pass = (fdepth<=idepth)&&fsize==isize&&fdepth+1==(size_t)ceil(log(fsize+1)/log(2));
	printf("Info: ");
	if(pass)
		printf("PASS\n");
	else
		printf("FAIL (size,depth):i(%lu, %lu), f(%lu, %lu); opt:(%lu)\n", isize, idepth, fsize, fdepth, (size_t)ceil(log(fsize+1)/log(2)));
	//btree_map(t1, DEPTH_FIRST_IN, TRUE, NULL, (lMapFunc)btree_print_map_f);
	btree_clear(t1, TRUE);
}

static void
test_balance(){
	btree *t2 = NULL;
	long x[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
	size_t i=  0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t2 = btree_insert(t2, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE);
	}
	//a very linear tree
	t2 = btree_balance(t2);
	size_t idepth = 0, isize = 0, fdepth = 0, fsize = 0;
	btree_info(t2, NULL, &idepth, NULL, NULL, &isize, NULL, NULL);
	//printf("Tree 2 Balanced (%ld nodes, %ld depth)...\n", size, max);//TODO test this better
	//btree_map(t2, DEPTH_FIRST_IN, TRUE, NULL, (lMapFunc)btree_print_map_f);
	btree_info(t2, NULL, &fdepth, NULL, NULL, &fsize, NULL, NULL);
	BOOLEAN pass = (fdepth<=idepth)&&fsize==isize&&(fdepth+1==(size_t)ceil(log(fsize+1)/log(2)));
	printf("Rebalanced: ");
	if(pass)
		printf("PASS\n");
	else
		printf("FAIL (size,depth):i(%lu, %lu), f(%lu, %lu); opt:(%lu)\n", isize, idepth, fsize, fdepth, (size_t)ceil(log(fsize+1)/log(2)));

}

void
test_btree(){
	test_insert();
	test_lots_insert();
	test_find();
	test_remove();
	test_map_pre();
	test_map_post();
	test_map_infix();
}

