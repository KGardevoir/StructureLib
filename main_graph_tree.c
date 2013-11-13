#include "graph_tree.h"
#include "main.h"

#define GRAPH_TEST_SIZE 25
typedef struct graph_dump_map_d {
	size_t pos;
	size_t max;
	long test[GRAPH_TEST_SIZE];
} graph_dump_map_d;

void
test_graph_tree(){
	long nums[] = {0,1,2,3,4,5,6};
	graph *g[] = {
		graph_tree_insert(NULL, (Object*)aLong_new(nums[0]), FALSE),
		graph_tree_insert(NULL, (Object*)aLong_new(nums[1]), FALSE),
		graph_tree_insert(NULL, (Object*)aLong_new(nums[2]), FALSE),
		graph_tree_insert(NULL, (Object*)aLong_new(nums[3]), FALSE),
		graph_tree_insert(NULL, (Object*)aLong_new(nums[4]), FALSE),
		graph_tree_insert(NULL, (Object*)aLong_new(nums[5]), FALSE),
		graph_tree_insert(NULL, (Object*)aLong_new(nums[6]), FALSE)
	};
	graph_tree_link(g[0], g[1]); graph_tree_link(g[0], g[4]); graph_tree_link(g[0], g[6]);
	graph_tree_link(g[1], g[2]); graph_tree_link(g[1], g[5]); graph_tree_link(g[1], g[6]);
	graph_tree_link(g[2], g[3]); graph_tree_link(g[2], g[4]);
	graph_tree_link(g[3], g[4]);
	graph_tree_link(g[4], g[0]);
	graph_tree_link(g[5], g[4]);
	graph_tree_link(g[6], g[5]);
	{
		printf("DAG Depth First Search (PRE): ");
		const long expect[] = {0,1,2,3,4,5,6};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_iterator_pre *it = graph_iterator_pre_new(g[0], &(graph_iterator_pre){});
		for(graph *node = graph_iterator_pre_next(it); node; node = graph_iterator_pre_next(it)){
			if(buffer.pos >= GRAPH_TEST_SIZE){
				break;
			} else {
				buffer.test[buffer.pos] = ((aLong*)node->data)->data;
				buffer.pos++;
			}
		}
		graph_iterator_pre_destroy(it);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Depth First Search (PRE): ");
		const long expect[] = {0,1,2,3,4,4,5,4,6,5,4,4,6,5,4};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_tree_iterator_pre *it = graph_tree_iterator_pre_new(g[0], &(graph_tree_iterator_pre){});
		for(graph *node = graph_tree_iterator_pre_next(it); node; node = graph_tree_iterator_pre_next(it)){
			//if(more->position == 0) printf("\n");
			//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
			if(buffer.pos >= GRAPH_TEST_SIZE){
				break;
			} else {
				buffer.test[buffer.pos] = ((aLong*)node->data)->data;
				buffer.pos++;
			}
		}
		graph_tree_iterator_pre_destroy(it);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Depth First Search (POST): ");
		const long expect[] = {4,3,4,2,4,5,4,5,6,1,4,4,5,6,0};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_tree_iterator_post *it = graph_tree_iterator_post_new(g[0], &(graph_tree_iterator_post){});
		for(graph *node = graph_tree_iterator_post_next(it); node; node = graph_tree_iterator_post_next(it)){
			//if(more->position == 0) printf("\n");
			//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
			if(buffer.pos >= GRAPH_TEST_SIZE){
				break;
			} else {
				buffer.test[buffer.pos] = ((aLong*)node->data)->data;
				buffer.pos++;
			}
		}
		graph_tree_iterator_post_destroy(it);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Breadth First Search: ");
		const long expect[] = {0,1,4,6,2,5,6,5,3,4,4,5,4,4,4};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_tree_iterator_breadth *it = graph_tree_iterator_breadth_new(g[0], &(graph_tree_iterator_breadth){});
		for(graph *node = graph_tree_iterator_breadth_next(it); node; node = graph_tree_iterator_breadth_next(it)){
			//if(more->position == 0) printf("\n");
			//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
			if(buffer.pos >= GRAPH_TEST_SIZE){
				break;
			} else {
				buffer.test[buffer.pos] = ((aLong*)node->data)->data;
				buffer.pos++;
			}
		}
		graph_tree_iterator_breadth_destroy(it);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Inhibited Depth First: ");
		const long expect[] = {0,1,2,5,4,6,5,4,4,6,5,4};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_tree_iterator_pre *it = graph_tree_iterator_pre_new(g[0], &(graph_tree_iterator_pre){});
		for(graph *node = graph_tree_iterator_pre_next(it); node; node = graph_tree_iterator_pre_next(it)){
			//if(more->position == 0) printf("\n");
			//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
			if(buffer.pos < GRAPH_TEST_SIZE){
				buffer.test[buffer.pos] = ((aLong*)node->data)->data;
				buffer.pos++;
			}
			if(((aLong*)node->data)->data == 2) it->p_add_children = FALSE;
		}
		graph_tree_iterator_pre_destroy(it);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		printf("Inhibited Breadth First: ");
		const long expect[] = {0,1,4,6,2,5,6,5,4,5,4,4};
		graph_dump_map_d buffer = {
			.max = ARRLENGTH(expect),
			.pos = 0,
			.test = {0}
		};
		graph_tree_iterator_breadth *it = graph_tree_iterator_breadth_new(g[0], &(graph_tree_iterator_breadth){});
		for(graph *node = graph_tree_iterator_breadth_next(it); node; node = graph_tree_iterator_breadth_next(it)){
			//if(more->position == 0) printf("\n");
			//printf("%*s%-4lu: %ld\n", (int)more->depth, "", more->depth, data->data);
			if(buffer.pos < GRAPH_TEST_SIZE){
				buffer.test[buffer.pos] = ((aLong*)node->data)->data;
				buffer.pos++;
			}
			if(((aLong*)node->data)->data == 2) it->p_add_children = FALSE;
		}
		graph_tree_iterator_breadth_destroy(it);
		compare_arrs(&expect[0], buffer.max, &buffer.test[0], buffer.pos);
	}
	{
		size_t nodes, edges;
		graph_tree_size(g[0], &nodes, &edges);
		printf("Graph Size: %s\n", (nodes == 15 && edges == 14)?"PASS":"FAIL");
	}
	graph_clear(g[0], TRUE);
}
