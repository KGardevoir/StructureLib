#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

static long
memcomp(void *a, void* b){ return (a>b?1:(a<b?-1:0)); }
static long
graph_memcomp(graph *a, graph *b){ return memcomp(a->data, b->data); }
static list_tspec graph_type_default = {
	.compar = (lCompare)graph_memcomp
};

static graph*
new_graph_node(void* data, BOOLEAN copy, list_tspec* type){
	graph init = {
		.data = copy?type->deep_copy(data):data,
		.edges = NULL
	};
	graph *mem = MALLOC(sizeof(graph));
	memcpy(mem, &init, sizeof(*mem));
	return mem;
}

graph*
graph_insert(graph* root, void* data, BOOLEAN copy, list_tspec* type){
	//add a new node the child of this node.
	return graph_link(root, new_graph_node(data, copy, type), type);
}

graph*
graph_link(graph* root, graph* child, list_tspec* type){
	if(child == NULL) return root;
	if(root == NULL) return child;
	if(!type) type = &graph_type_default;
	//printf("%p %p %p\n", child, child->data, child->edges);
	root->edges = dlist_addOrdered(root->edges, child, FALSE, type);
	return root;
}

#if 0
graph*
graph_remove(graph* root, void* key, void** data, BOOLEAN copy, list_tspec* type){
#error "Unimplmented"
}
#endif
void
graph_clear(graph *root, BOOLEAN destroy_data, list_tspec *type){

}

static BOOLEAN
graph_map_filter_f(graph *child, splaytree** tree){
	*tree = splay_find(*tree, child, NULL);
	if(*tree && (*tree)->data == child){
		//printf("E");
		return FALSE;
	}
	//printf("N");
	return TRUE;
}

static BOOLEAN
graph_print_children(graph *graphs, void* _noaux/*NULL*/){
	printf("%ld ", (long)graphs->data);
	(void)_noaux;
	return TRUE;
}

static BOOLEAN
graph_map_internal(graph *root, TRAVERSAL_STRATEGY method, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	splaytree *visited = splay_insert(NULL, root, FALSE, NULL);
	dlist *stk = dlist_append(NULL, root, FALSE, NULL);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) graph_size(root, &size, NULL);
	while(stk){
		graph *g;
		stk = dlist_dequeue(stk, (void**)&g, FALSE, NULL);
		visited = splay_insert(visited, g, FALSE, NULL);
		stk = dlist_filter_i(stk, &visited, (lMapFunc)graph_map_filter_f, FALSE, NULL);
		if(method == DEPTH_FIRST){
			stk = dlist_concat(
					dlist_filter(g->edges, &visited,
						(lMapFunc)graph_map_filter_f, FALSE, NULL), stk);
		} else {
			stk = dlist_concat(stk,
					dlist_filter(g->edges, &visited,
						(lMapFunc)graph_map_filter_f, FALSE, NULL));
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
			if(!func(pass_data?g->data:g, &ax))
				goto cleanup;
			position++;
		} else {
			if(!func(pass_data?g->data:g, aux))
				goto cleanup;
		}
	}
	bstree_clear(visited, FALSE, NULL);
	return TRUE;
cleanup:
	bstree_clear(visited, FALSE, NULL);
	dlist_clear(stk, FALSE, NULL);
	return FALSE;
}

BOOLEAN
graph_map(graph *root, TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc func){
	return graph_map_internal(root, strat, TRUE, more_info, aux, func);
}

struct graph_find_d {
	list_tspec *type;
	BOOLEAN key_compar;
	graph *rtn;
	void *goal;
};

static BOOLEAN
graph_find_f(graph* node, struct graph_find_d *aux){
	if((aux->type->key_compar?
			aux->type->compar(aux->goal, node->data):
			aux->type->key_compar(aux->goal, node->data)) == 0){
		aux->rtn = node;
		return FALSE;
	}
	return TRUE;
}

graph*
graph_find(graph *root, TRAVERSAL_STRATEGY strat, void* dat, list_tspec* type){
	struct graph_find_d mm = {
		.type = type,
		.key_compar = FALSE,
		.rtn = NULL,
		.goal = dat
	};
	if(!graph_map_internal(root, strat, FALSE, FALSE, &mm, (lMapFunc)graph_find_f) && mm.rtn)
		return mm.rtn;
	return NULL;
}

graph*
graph_find_via_key(graph *root, TRAVERSAL_STRATEGY strat, void* dat, list_tspec* type){
	struct graph_find_d mm = {
		.type = type,
		.key_compar = TRUE,
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

static BOOLEAN
graph_path_f_tr(void **data, dlist* path){
	*data = dlist_append(path, *data, FALSE, NULL);
	return TRUE;
}

graph*
graph_path_key_match(graph *root, dlist *key_path, list_tspec *type){
	size_t size = dlist_length(key_path);
	size_t i = 0;
	dlist *run = key_path;
	graph *troot = root;
	while(i < size){
		dlist *descent = dlist_find(troot->edges, run->data, FALSE, type);
		if(descent == NULL) return NULL;
		troot = descent->data;
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
