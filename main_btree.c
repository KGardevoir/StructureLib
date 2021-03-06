#include "btree.h"
#include <math.h>
#include "main.h"
#include <limits.h>

#define BSTREE_TEST_SIZE 32
typedef struct btree_dump_map_d {
	size_t pos;
	size_t max;
	long test1[BSTREE_TEST_SIZE];
	long test2[BSTREE_TEST_SIZE];
} btree_dump_map_d;

/*
static BOOLEAN
btree_print_map_f(aLong *data, lMapFuncAux*more, btree *node){
	printf("%*s%lu: %ld (%p,%p)\n", (int)more->depth, "", more->depth, data->data, node->left?node->left->data:NULL, node->right?node->right->data:NULL);
	return TRUE;
}
*/

static void
test_find(){
	btree *t = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i = 0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t = btree_insert(t, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
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
		t = btree_insert(t, (Object*)aLong_new(v[i]), &aLong_type.compare, FALSE, NULL);
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
		t = btree_insert(t, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
	size_t size = 0;
	btree_info(t, NULL, NULL, NULL, NULL, &size, NULL, NULL);
	printf("Insert (Many): ");
	if(size != ARRLENGTH(x)){
		printf("FAIL");
	} else {
		printf("PASS");
	}
	printf("\n");
	btree_clear(t, TRUE);
}

static void
test_remove(){
	btree *t = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i = 0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t = btree_insert(t, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
	}
	const aLong tmp = {
		.method = &aLong_type,
		.data = x[6]
	};
	//btree_map(t, DEPTH_FIRST_PRE, TRUE, NULL, (lMapFunc)btree_print_map_f);
	BOOLEAN success;
	btree *f = btree_remove(t, (Object*)&tmp, &aLong_type.compare, NULL, TRUE, &success);
	printf("Remove: ");
	if(success){
		printf("PASS\n");
		t = f;
	} else {
		printf("FAIL\n");
	}
	btree_clear(t, TRUE);
}

static void
test_map_pre(){
	btree *t1 = NULL;
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	size_t i = 0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
	}
	const long expect2[] = {0,1,2,2,3,3,1,2,3};
	btree_dump_map_d buffer = {
		.max = ARRLENGTH(x)-1,
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	btree_iterator_pre *it = btree_iterator_pre_new(t1, &(btree_iterator_pre){});
	for(btree *node = btree_iterator_pre_next(it); node; node = btree_iterator_pre_next(it), buffer.pos++){
		buffer.test1[buffer.pos] = ((aLong*)node->data)->data;
		buffer.test2[buffer.pos] = it->r_depth;
		if(buffer.pos >= buffer.max)
			break;
	}
	btree_iterator_pre_destroy(it);
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
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
	}
	const long expect1[] = {1,3,4,6,7,8,10,13,14};
	const long expect2[] = {2,1,3,2,3,0,1,3};
	btree_dump_map_d buffer = {
		.max = ARRLENGTH(x)-1,
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	btree_iterator_in *it = btree_iterator_in_new(t1, &(btree_iterator_in){});
	for(btree *node = btree_iterator_in_next(it); node; node = btree_iterator_in_next(it), buffer.pos++){
		buffer.test1[buffer.pos] = ((aLong*)node->data)->data;
		buffer.test2[buffer.pos] = it->r_depth;
		if(buffer.pos >= buffer.max)
			break;
	}
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
	size_t i =  0;
	for(i = 0; i < ARRLENGTH(x); i++){
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
	}
	const long expect1[] = {1,4,7,6,3,13,14,10,8};
	const long expect2[] = {2,3,3,2,1,3,2,1};
	btree_dump_map_d buffer = {
		.max = ARRLENGTH(x)-1,
		.pos = 0,
		.test1 = {0},
		.test2 = {0}
	};
	btree_iterator_post *it = btree_iterator_post_new(t1, &(btree_iterator_post){});
	for(btree *node = btree_iterator_post_next(it); node; node = btree_iterator_post_next(it), buffer.pos++){
		buffer.test1[buffer.pos] = ((aLong*)node->data)->data;
		buffer.test2[buffer.pos] = it->r_depth;
		if(buffer.pos >= buffer.max)
			break;
	}
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
		t1 = btree_insert(t1, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
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
		t2 = btree_insert(t2, (Object*)aLong_new(x[i]), &aLong_type.compare, FALSE, NULL);
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
	btree_clear(t2, TRUE);
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
	//test_map_breadth();
	test_balance();
	test_info();
}

