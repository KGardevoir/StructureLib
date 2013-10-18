#include "splaytree.h"
#include "main.h"

static void
test_insert_many(){
	splaytree *t = NULL;
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i;
	size_t min, max, avg, nodes, leaves;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		t = splay_insert(t, (Object*)aLong_new(i), &aLong_type.compare, FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	btree_info(t, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
	printf("Insert (many): %s\n", (NUMS-1 == nodes)?"PASS":"FAIL");
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
	btree_clear(t, TRUE);
}

static void
test_remove(){
	splaytree *t = NULL;
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i = 0;
	size_t min, max, avg, nodes, leaves;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		t = splay_insert(t, (Object*)aLong_new(i), &aLong_type.compare, FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	for(i = 1; i < NUMS; i += 2){
		aLong *anew = aLong_new(i);
		t = splay_remove(t, (Object*)anew, &aLong_type.compare, NULL, TRUE);
		LINKED_FREE(anew);
	}
	btree_info(t, &min, &max, &avg, &leaves, &nodes, NULL, NULL);
	printf("Remove: %s\n", (NUMS/2-1 == nodes)?"PASS":"FAIL");
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
	btree_clear(t, TRUE);
}

static void
test_find_min_max(){
	splaytree *t = NULL;
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		t = splay_insert(t, (Object*)aLong_new(i), &aLong_type.compare, FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));

	long I;
	t = splay_find(t, btree_findmin(t)->data, &aLong_type.compare); I = ((aLong*)t->data)->data;
	long nm = ((aLong*)btree_successor(t, t, &aLong_type.compare)->data)->data;
	long J;
	t = splay_find(t, btree_findmax(t)->data, &aLong_type.compare); J = ((aLong*)t->data)->data;
	long K = ((aLong*)btree_predessor(t, t, &aLong_type.compare)->data)->data;
	printf("Find min/max: ");
	if(I != 1 || nm != 2 || J != NUMS-1 || K != NUMS-2)
		printf("FAIL FindMin or FindMax error! Got %lu and %lu, head successor: %lu, tail predessor: %lu\n", I, J, K, nm);
	else
		printf("PASS\n");

	btree_clear(t, TRUE);
}

static void
test_find(){
	splaytree *t = NULL;
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		t = splay_insert(t, (Object*)aLong_new(i), &aLong_type.compare, FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism

	printf("Find: ");
	BOOLEAN failed = FALSE;
	for(i = 2; i < NUMS; i+=2){
		long k;
		aLong tmp = {
			.method = &aLong_type,
			.data = i
		};
		t = splay_find(t, (Object*)&tmp, &tmp.method->compare); k = ((aLong*)t->data)->data;
		if(k != (long)i){
			printf("FAIL\n");
			failed = TRUE;
			break;
		}
	}
	if(!failed) printf("PASS\n");
	btree_clear(t, TRUE);
}

void
test_splay(){
	{
		struct rusage start, end;
		getrusage(RUSAGE_SELF, &start);
		test_insert_many();
		getrusage(RUSAGE_SELF, &end);
		struct timeval diff;
		timersub(&end.ru_utime, &start.ru_utime, &diff);
		printf("Insert time: %lf\n", diff.tv_sec+diff.tv_usec/1e9);
	}
	test_remove();
	test_find_min_max();
	test_find();
}

