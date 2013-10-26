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
	size_t min = 0, max = 0, avg = 0, leaves = 0, nodes = 0;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		splay_insert(t, (Object*)aLong_new(i), FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	for(i = 1; i < NUMS; i += 2){
		splay_remove(t, (Object*)&(aLong){.method = &aLong_type, .data = i}, t->data_method, TRUE);
	}
	btree_info(t->root, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
	printf("Remove: %s\n", (NUMS/2-1 == t->size && nodes == t->size)?"PASS":"FAIL");
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
	//btree_info(t->root, NULL, NULL, NULL, NULL, &nodes, NULL, NULL);
	//printf("%zu/%zu Nodes\n", t->size, nodes);
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));

	btree *I_node = btree_findmin(t->root);
	btree *J_node = btree_findmax(t->root);
	btree *IS_node = btree_successor(t->root, I_node, t->data_method);
	btree *JS_node = btree_predecessor(t->root, J_node, t->data_method);
	printf("Find min/max: ");
	if(I_node && J_node && JS_node && IS_node){
		aLong *I = (aLong*)I_node->data, *J = (aLong*)J_node->data, *IS = (aLong*)IS_node->data, *JS = (aLong*)JS_node->data;
		if(I->data != 1 || IS->data != 2 || J->data != NUMS-1 || JS->data != NUMS-2)
			printf("FAIL FindMin or FindMax error! Got %lu and %lu, head successor: %lu, tail predessor: %lu\n", I->data, J->data, IS->data, JS->data);
		else
			printf("PASS\n");
	} else {
		printf("FAIL, node null: min:%p, min successor: %p max: %p, max predecessor: %p\n", I_node, IS_node, J_node, JS_node);
	}

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

