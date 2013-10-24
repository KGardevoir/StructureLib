#include "splaytree.h"
#include "main.h"

static void
test_insert_many(){
	splaytree *t = splaytree_new(&aLong_type.compare);
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i;
	size_t min, max, avg, leaves, nodes;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS){
		splay_insert(t, (Object*)aLong_new(i), FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	}
	btree_info(t->root, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
	printf("Insert (many): %s\n", (NUMS-1 == nodes)?"PASS":"FAIL");
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
	splay_clear(t, TRUE);
	splaytree_destroy(t);
}

static void
test_remove(){
	splaytree *t = splaytree_new(&aLong_type.compare);
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i = 0;
	size_t min, max, avg, leaves;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		splay_insert(t, (Object*)aLong_new(i), FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	for(i = 1; i < NUMS; i += 2){
		splay_remove(t, (Object*)&(aLong){.method = &aLong_type, .data = i}, t->data_method, TRUE);
	}
	btree_info(t->root, &min, &max, &avg, &leaves, NULL, NULL, NULL);
	printf("Remove: %s\n", (NUMS/2-1 == t->size)?"PASS":"FAIL");
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
	splay_clear(t, TRUE);
	splaytree_destroy(t);
}

static void
test_find_min_max(){
	splaytree *t = splaytree_new(&aLong_type.compare);
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		splay_insert(t, (Object*)aLong_new(i), FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));

	aLong *I = (aLong*)splay_find(t, btree_findmin(t->root)->data, t->data_method);
	aLong *IS = (aLong*)btree_successor(t->root, t->root, t->data_method)->data;
	aLong *J = (aLong*)splay_find(t, btree_findmax(t->root)->data, t->data_method);
	aLong *JS = (aLong*)btree_predessor(t->root, t->root, &aLong_type.compare)->data;
	printf("Find min/max: ");
	if(!I || !J || !IS || !JS || I->data != 1 || IS->data != 2 || J->data != NUMS-1 || JS->data != NUMS-2)
		printf("FAIL FindMin or FindMax error! Got %lu and %lu, head successor: %lu, tail predessor: %lu\n", I->data, J->data, IS->data, JS->data);
	else
		printf("PASS\n");

	splay_clear(t, TRUE);
	splaytree_destroy(t);
}

static void
test_find(){
	splaytree *t = splaytree_new(&aLong_type.compare);
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		splay_insert(t, (Object*)aLong_new(i), FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism

	printf("Find: ");
	BOOLEAN failed = FALSE;
	for(i = 2; i < NUMS; i+=2){
		aLong *k;
		aLong tmp = {
			.method = &aLong_type,
			.data = i
		};
		k = (aLong*)splay_find(t, (Object*)&tmp, &tmp.method->compare);
		if(k->data != (long)i){
			printf("FAIL\n");
			failed = TRUE;
			break;
		}
	}
	if(!failed) printf("PASS\n");
	splay_clear(t, TRUE);
	splaytree_destroy(t);
}

void
test_splay(){
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
	test_find_min_max();
	test_find();
}

