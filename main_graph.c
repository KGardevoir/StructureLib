#include "graph.h"
#include "main.h"

#define GRAPH_TEST_SIZE 25
typedef struct graph_dump_map_d {
	size_t pos;
	size_t max;
	long test[GRAPH_TEST_SIZE];
} graph_dump_map_d;


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
		graph_iterator_pre* it = graph_iterator_pre_new(g[0], &(graph_iterator_pre){});
		for(graph *g = graph_iterator_pre_next(it); g; g = graph_iterator_pre_next(it)){
			if(buffer.pos >= GRAPH_TEST_SIZE){
				break;
			} else {
				buffer.test[buffer.pos] = ((aLong*)g->data)->data;
				buffer.pos++;
			}
		}
		graph_iterator_pre_destroy(it);
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
		graph_iterator_post* it = graph_iterator_post_new(g[0], &(graph_iterator_post){});
		for(graph *g = graph_iterator_post_next(it); g; g = graph_iterator_post_next(it)){
			if(buffer.pos >= GRAPH_TEST_SIZE){
				break;
			} else {
				buffer.test[buffer.pos] = ((aLong*)g->data)->data;
				buffer.pos++;
			}
		}
		graph_iterator_post_destroy(it);
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
		graph_iterator_breadth* it = graph_iterator_breadth_new(g[0], &(graph_iterator_breadth){});
		for(graph *g = graph_iterator_breadth_next(it); g; g = graph_iterator_breadth_next(it)){
			if(buffer.pos >= GRAPH_TEST_SIZE){
				break;
			} else {
				buffer.test[buffer.pos] = ((aLong*)g->data)->data;
				buffer.pos++;
			}
		}
		graph_iterator_breadth_destroy(it);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		size_t nodes, edges;
		graph_size(g[0], &nodes, &edges);
		printf("Graph Size: %s\n", (nodes == ARRLENGTH(nums) && edges == 13)?"PASS":"FAIL");
	}

	graph_clear(g[0], TRUE);
}

