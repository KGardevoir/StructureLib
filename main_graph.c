#include "graph.h"
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

#define GRAPH_TEST_SIZE 25
typedef struct graph_dump_map_d {
	size_t pos;
	size_t max;
	long test[GRAPH_TEST_SIZE];
} graph_dump_map_d;

static BOOLEAN
graph_dump_map_f(aLong* data, lMapFuncAux* more, graph* node){
	(void)node;
	graph_dump_map_d *aux = more->aux;
	//if(more->position == 0) printf("\n");
	//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
	if(aux->pos >= GRAPH_TEST_SIZE){
		return FALSE;
	} else {
		aux->test[aux->pos] = data->data;
		aux->pos++;
		return TRUE;
	}
}



void
test_graph(){
	long nums[] = {0,1,2,3,4,5,6};
	graph *g[] = {
		graph_insert(NULL, (Object*)aLong_new(nums[0]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[1]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[2]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[3]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[4]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[5]), FALSE),
		graph_insert(NULL, (Object*)aLong_new(nums[6]), FALSE)
	};
	graph_link(g[0], g[1]); graph_link(g[0], g[4]); graph_link(g[0], g[6]);
	graph_link(g[1], g[2]); graph_link(g[1], g[5]); graph_link(g[1], g[6]);
	graph_link(g[2], g[3]); graph_link(g[2], g[4]); graph_link(g[2], g[4]);
	graph_link(g[3], g[4]);
	graph_link(g[4], g[0]);
	graph_link(g[5], g[4]);
	graph_link(g[6], g[5]);
	{
		printf("Depth First Search (PRE): ");
		const long expect[] = {0,1,2,3,4,5,6};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_map(g[0], DEPTH_FIRST, TRUE, &buffer, (lMapFunc)graph_dump_map_f);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Depth First Search (POST): ");
		const long expect[] = {4,3,2,5,6,1,0};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_map(g[0], DEPTH_FIRST_POST, TRUE, &buffer, (lMapFunc)graph_dump_map_f);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Breadth First Search: ");
		const long expect[] = {0,1,4,6,2,5,3};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_map(g[0], BREADTH_FIRST, TRUE, &buffer, (lMapFunc)graph_dump_map_f);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		size_t nodes, edges;
		graph_size(g[0], &nodes, &edges);
		printf("Graph Size: %s\n", (nodes == ARRLENGTH(nums) && edges == 13)?"PASS":"FAIL");
	}

	graph_clear(g[0], TRUE);
}

