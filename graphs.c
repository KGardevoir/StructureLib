#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

static void graph_node_destroy(graph* self){ }
static size_t graph_node_getSize(graph* self){ return sizeof(graph); }
static long graph_node_compare(graph* self, graph* oth);
static graph* graph_node_copy(graph* self);
static Comparable graph_methods = {
	.parent = {
		.copy = (anObject*(*)(const anObject *self))graph_node_copy,
		.destroy = (void(*)(const anObject *self))graph_node_destroy,
		.getSize = (size_t(*)(const anObject *self))graph_node_getSize
	},
	.compare = (long(*)(const aComparable* self, const anObject *oth))graph_node_compare
};
static long graph_node_compare(graph* self, graph* oth){ return self>oth?1:(self==oth?0:-1); }
//static long graph_node_compare(graph* self, graph* oth){
//	return ((aComparable*)self->data)->method->compare(self->data, oth->data);
//}

static graph*
graph_node_new(anObject* data, BOOLEAN copy){
	graph init = {
		.method = &graph_methods,
		.data = copy?data->method->copy(data):data,
		.edges = NULL
	};
	graph *mem = (graph*)MALLOC(sizeof(graph));
	memcpy(mem, &init, sizeof(*mem));
	return mem;
}

static graph*
graph_node_copy(graph* self){
	graph init = {
		.method = &graph_methods,
		.data = self->data,
		.edges = self->edges
	};
	graph *mem = (graph*)MALLOC(sizeof(graph));
	memcpy(mem, &init, sizeof(*mem));
	return mem;
}



graph*
graph_insert(graph* root, anObject* data, BOOLEAN copy){
	//add a new node the child of this node.
	return graph_link(root, graph_node_new(data, copy));
}

graph*
graph_link(graph* root, graph* child){
	if(child == NULL) return root;
	if(root == NULL) return child;
	//printf("%p %p %p\n", child, child->data, child->edges);
	root->edges = dlist_append(root->edges, (anObject*)child, FALSE);
	//Rather than adding ordered, order based on arrival time
	return root;
}

#if 0
graph*
graph_remove(graph* root, void* key, void** data, BOOLEAN copy, list_tspec* type){
#error "Unimplmented"
}
#endif
void
graph_clear(graph *root, BOOLEAN destroy_data){
	//TODO implement this...
}

static BOOLEAN
graph_map_filter_f(graph *child, splaytree** tree){
	*tree = splay_find(*tree, (aComparable*)child);
	if(*tree && child->method->compare((aComparable*)child, (anObject*)(*tree)->data) == 0){
		//printf("E");
		return FALSE;
	}
	//printf("N");
	return TRUE;
}

typedef struct aLong {
	const Comparable *method;
	long data;
} aLong;
static BOOLEAN
graph_print_children(graph *graphs, void* _noaux/*NULL*/){
	printf("%ld ", ((aLong*)graphs->data)->data);
	(void)_noaux;
	return TRUE;
}

static BOOLEAN
graph_map_internal(graph *root, TRAVERSAL_STRATEGY method, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	splaytree *visited = splay_insert(NULL, (aComparable*)root, FALSE);
	dlist *stk = dlist_append(NULL, (anObject*)root, FALSE);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) graph_size(root, &size, NULL);
	while(stk){
		graph *g;
		stk = dlist_dequeue(stk, (anObject**)&g, FALSE);
		visited = splay_insert(visited, (aComparable*)g, FALSE);
		stk = dlist_filter_i(stk, &visited, (lMapFunc)graph_map_filter_f, FALSE);
		if(method == DEPTH_FIRST){
			stk = dlist_concat(
					dlist_filter(g->edges, &visited,
						(lMapFunc)graph_map_filter_f, FALSE), stk);
		} else {
			stk = dlist_concat(stk,
					dlist_filter(g->edges, &visited,
						(lMapFunc)graph_map_filter_f, FALSE));
		}
		/*
		printf("<");
		dlist_map(g->edges, FALSE, NULL, (lMapFunc)graph_print_children);
		printf(">");
		printf("[");
		dlist_map(stk, FALSE, NULL, (lMapFunc)graph_print_children);
		printf("]");
		printf("(%ld);", g->data);
		*/
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = depth,
				.position = position,
				.size = size,
				.aux = aux
			};
			if(!func(pass_data?g->data:(anObject*)g, &ax))
				goto cleanup;
			position++;
		} else {
			if(!func(pass_data?g->data:(anObject*)g, aux))
				goto cleanup;
		}
	}
	bstree_clear(visited, FALSE);
	return TRUE;
cleanup:
	bstree_clear(visited, FALSE);
	dlist_clear(stk, FALSE);
	return FALSE;
}

BOOLEAN
graph_map(graph *root, TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc func){
	return graph_map_internal(root, strat, TRUE, more_info, aux, func);
}

struct graph_find_d {
	graph *rtn;
	aComparable *goal;
};

static BOOLEAN
graph_find_f(graph* node, struct graph_find_d *aux){
	if(aux->goal->method->compare(aux->goal, node->data) == 0){
		aux->rtn = node;
		return FALSE;
	}
	return TRUE;
}

graph*
graph_find(graph *root, TRAVERSAL_STRATEGY strat, aComparable* dat){
	struct graph_find_d mm = {
		.rtn = NULL,
		.goal = dat
	};
	if(!graph_map_internal(root, strat, FALSE, FALSE, &mm, (lMapFunc)graph_find_f) && mm.rtn)
		return mm.rtn;
	return NULL;
}

struct graph_size_d {
	size_t nodes;
	size_t edges;
};

static BOOLEAN
graph_size_f(graph* root, struct graph_size_d *aux){
	aux->nodes = aux->nodes + 1;
	aux->edges = aux->edges + dlist_length(root->edges);
	return TRUE;
}

void
graph_size(graph* root, size_t *nodes, size_t *edges){
	struct graph_size_d mm = {
		.nodes = 0,
		.edges = 0
	};
	graph_map_internal(root, BREADTH_FIRST, FALSE, FALSE, &mm, (lMapFunc)graph_size_f);
	*nodes = mm.nodes;
	*edges = mm.edges;
}

graph*
graph_path_key_match(graph *root, dlist *key_path){
	size_t size = dlist_length(key_path);
	size_t i = 0;
	dlist *run = key_path;
	graph *troot = root;
	while(i < size){
		dlist *descent = dlist_find(troot->edges, (aComparable*)run->data, FALSE);
		if(descent == NULL) return NULL;
		troot = (graph*)descent->data;
		i++; run = run->next;
	}
	return troot;
}


#if 0
graph*
graph_spanning(graph* root, long(edge_weight)(graph*,graph*)){
	//TODO utilize Kruskal's Algorithm
}
#endif
