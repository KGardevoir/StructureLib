#include "main.h"
#include "htable.h"

static void
dump_buckets(htable *t){
	for(size_t i = 0; i < t->m_max_size; i++){
		if(t->m_array[i]){
			printf("%3zu: %zu\n", i, t->m_array[i]->size);
		}
	}
}

static void
test_insert_many(){
	htable *t = htable_new(128, &aLong_type.compare);
	const size_t NUMS = 40000;
	const size_t GAP = 307;
	size_t i;
	for(i = GAP % NUMS; i != 0; i = (i + GAP) % NUMS)
		htable_insert(t, (Object*)aLong_new(i), FALSE);//typically it is more appropriate to access via &Object->method->interface. It is OK here, because there is no polymorphism
	printf("Insert (many): %s\n", (NUMS-1 == t->size)?"PASS":"FAIL");
	//printf("%lu Nodes, %lu leaves\n", nodes, leaves);
	//printf("Tree Statistics: min: %lu, max: %lu, avg: %lu, optimal: %lu\n", min, max, avg, (size_t)(log(nodes+1)/log(2)+.5));
	htable_clear(t, TRUE);
	htable_destroy(t);
}

static void
test_find(){
	htable *t = htable_new(17, &aLong_type.compare);
	long x[] = { 8, 3, 1, 6, 4, 7, 10, 14, 13 };
	for(size_t i = 0; i < ARRLENGTH(x); i++)
		htable_insert(t, (Object*)aLong_new(x[i]), FALSE);
	BOOLEAN failure = FALSE;
	//dump_buckets(t);
	size_t i = 0;
	for(i = 0; i < ARRLENGTH(x); i++){
		aLong *obj = (aLong*)htable_find(t, (Object*)&(aLong){.method = &aLong_type, .data = x[i]}, t->data_method);
		if(!obj){
			printf("Failed to find object\n");
			//printf("%ld ", x[i]);
			failure = TRUE;
		}
	}
	printf("Find: ");
	if(failure)
		printf("FAIL");
	else
		printf("PASS");
	printf("\n");
	htable_clear(t, TRUE);
}


void
test_htable(){
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
	test_find();
}
