#include "graph_tree.h"
#include "slist.h"
#include "splaytree.h"

typedef struct graph_tree_verify_d {
	graph *root;
	size_t num;
} graph_tree_verify_d;

graph*
graph_tree_insert(graph* root, Object* data, BOOLEAN copy){
	//add a new node the child of this node, only if it will not form a loop
	return graph_tree_link(root, graph_insert(NULL, data, copy));
}

graph*
graph_tree_link(graph* root, graph* child){
	if(child == NULL) return root;
	if(root == NULL) return child;
	//printf("%p %p %p\n", child, child->data, child->edges);
	root->edges = dlist_pushback(root->edges, (Object*)child, FALSE);
	//verify tree
	size_t root_visited = 0;
	graph_tree_iterator_pre *iter =
		graph_tree_iterator_pre_new(root, &(graph_tree_iterator_pre){});
	graph *g;
	for(g = graph_tree_iterator_pre_next(iter); g; g = graph_tree_iterator_pre_next(iter)){
		if(g == root){
			root_visited++;
		}
		if(root_visited > 1){
			break;
		}
	}
	graph_tree_iterator_pre_destroy(iter);
	if(root_visited > 1){
		root->edges = dlist_popback(root->edges, NULL, FALSE);
	}
	return root;
}

static const Object_vtable node_info_vtable = {
	.destroy = (void(*)(const Object*))free
};

typedef struct node_info {
	const Object_vtable *method;
	const size_t depth;
	const graph *node;
} node_info;

static node_info*
node_info_new(graph *node, size_t depth){
	node_info init = {
		.method = &node_info_vtable,
		.depth = depth,
		.node = node
	};
	return (node_info*)memcpy(LINKED_MALLOC(sizeof(node_info)), &init, sizeof(node_info));
}

#if 0
	typedef struct aLong {
		const Object_vtable *method;
		long data;
	} aLong;

#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC) do { printf(BSTR); dlist *_run; DLIST_ITERATE(_run, LST, printf(FMT, ACC); ); printf(ESTR); } while(0)
static BOOLEAN
graph_print_children(aLong *data){
	printf("%ld ", data->data);
	return TRUE;
}
#else
#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC)
#endif


/**
 * Iterate through the graph in postfix order, normally this is performed prefix. The space complexity is O(3*E+N) worst
 * case, time complexity is O(N) (approximately).
 *
 * @param root - the root of the graph
 * @param mem  - the memory
 */
graph_tree_iterator_post*
graph_tree_iterator_post_new(graph *root, graph_tree_iterator_post *mem){
	dlist *stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	dlist *exec = NULL;
	while(stk){
		node_info *g;
		stk = dlist_popfront(stk, (Object**)&g, FALSE);
		dlist *new_list = dlist_reverse(dlist_copy(g->node->edges, FALSE));
		dlist *run;
		DLIST_ITERATE(run, new_list)
			run->data = (void*)node_info_new((graph*)run->data, g->depth+1);
		stk = dlist_concat(new_list, stk);
	#if 0
		printf("<");
		dlist_map(g->node->edges, FALSE, NULL, (lMapFunc)graph_print_children);
		printf(">");
		printf("[");
		dlist_map(stk, FALSE, NULL, (lMapFunc)graph_print_children);
		printf("]\n");
	#endif
		exec = dlist_pushback(exec, (Object*)g, FALSE);
	}
	mem->i_stk = exec;
	mem->i_root = root;
	mem->r_depth = 0;
	mem->r_current = NULL;
	return mem;
}

graph*
graph_tree_iterator_post_next(graph_tree_iterator_post* self){
	if(!self->i_stk){
		self->r_current = NULL;
		self->r_depth = 0;
		return NULL;
	}
	node_info *g;
	self->i_stk = dlist_popback(self->i_stk, (Object**)&g, FALSE);
	self->r_depth = g->depth;
	self->r_current = (graph*)g->node;
	CALL_VOID(g, destroy);
	return self->r_current;
}

void
graph_tree_iterator_post_destroy(graph_tree_iterator_post* self){
	dlist_clear(self->i_stk, TRUE);
	self->i_root = NULL;
	self->i_stk = NULL;
	self->r_depth = 0;
	self->r_current = NULL;
}


graph_tree_iterator_pre*
graph_tree_iterator_pre_new(graph *root, graph_tree_iterator_pre* mem){
	mem->i_stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	mem->i_root = root;
	mem->r_depth = 0;
	mem->r_current = root;
	mem->p_add_children = TRUE;
	mem->i_children = NULL;
	return mem;
}

graph*
graph_tree_iterator_pre_next(graph_tree_iterator_pre* self){
	if(self->i_children){
		if(self->p_add_children){
			self->i_stk = dlist_concat(self->i_children, self->i_stk);
		} else {
			dlist_clear(self->i_children, TRUE);
		}
		self->i_children = NULL;
	}
	if(!self->i_stk){
		self->r_current = NULL;
		self->r_depth = 0;
		self->p_add_children = TRUE;
		return NULL;
	}
	node_info *g;
	self->i_stk = dlist_popfront(self->i_stk, (Object**)&g, FALSE);
	dlist *new_list = dlist_copy(g->node->edges, FALSE);
	dlist *run;
	DLIST_ITERATE(run, new_list){
		run->data = (void*)node_info_new((graph*)run->data, g->depth+1);
	}
	self->i_children = new_list;
	//visit g->node
	self->r_depth = g->depth;
	self->r_current = (graph*)g->node;
	self->p_add_children = TRUE;
	CALL_VOID(g, destroy);
	return self->r_current;
}

void
graph_tree_iterator_pre_destroy(graph_tree_iterator_pre* self){
	dlist_clear(self->i_stk, TRUE);
	dlist_clear(self->i_children, TRUE);
	self->i_root = NULL;
	self->i_stk = NULL;
	self->r_depth = 0;
	self->r_current = NULL;
	self->p_add_children = TRUE;
}


graph_tree_iterator_breadth*
graph_tree_iterator_breadth_new(graph *root, graph_tree_iterator_breadth* mem){
	mem->i_stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	mem->i_root = root;
	mem->r_depth = 0;
	mem->r_current = root;
	mem->p_add_children = TRUE;
	mem->i_children = NULL;
	return mem;
}

graph*
graph_tree_iterator_breadth_next(graph_tree_iterator_breadth* self){
	if(self->i_children){
		if(self->p_add_children){
			self->i_stk = dlist_concat(self->i_stk, self->i_children);
		} else {
			dlist_clear(self->i_children, TRUE);
		}
	}
	if(!self->i_stk){
		self->r_current = NULL;
		self->r_depth = 0;
		self->p_add_children = TRUE;
		return NULL;
	}
	node_info *g;
	self->i_stk = dlist_popfront(self->i_stk, (Object**)&g, FALSE);

	dlist *new_list = dlist_copy(g->node->edges, FALSE);
	dlist *run;
	DLIST_ITERATE(run, new_list){
		run->data = (void*)node_info_new((graph*)run->data, self->r_depth+1);
	}
	self->i_children = new_list;

	self->r_current = (graph*)g->node;
	self->r_depth = g->depth;
	self->p_add_children = TRUE;
	CALL_VOID(g, destroy);
	return self->r_current;
}

void
graph_tree_iterator_breadth_destroy(graph_tree_iterator_breadth* self){
	dlist_clear(self->i_stk, TRUE);
	dlist_clear(self->i_children, TRUE);
	self->i_root = NULL;
	self->i_stk = NULL;
	self->i_children = NULL;
	self->r_depth = 0;
	self->r_current = NULL;
	self->p_add_children = TRUE;
}

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define MAX(a,b) ( (a)>(b)?a:b )

graph*
graph_tree_find(graph *root, TRAVERSAL_STRATEGY strat, void* key, const Comparable_vtable* key_method){
	void *iter = (void*)&(char[MAX(sizeof(graph_tree_iterator_breadth), MAX(sizeof(graph_tree_iterator_pre), sizeof(graph_tree_iterator_post)))]){0};
	void (*destroy)(void*);
	graph* (*next)(void*);
	switch(strat){
		//select iterator and next function
		case DEPTH_FIRST_POST:
			iter    = graph_tree_iterator_post_new(root, iter);
			destroy = graph_tree_iterator_post_destroy;
			next    = graph_tree_iterator_post_next;
			break;
		case DEPTH_FIRST_PRE:
			iter    = graph_tree_iterator_pre_new(root, iter);
			destroy = graph_tree_iterator_pre_destroy;
			next    = graph_tree_iterator_pre_next;
			break;
		default:
		case BREADTH_FIRST:
			iter    = graph_tree_iterator_breadth_new(root, iter);
			destroy = graph_tree_iterator_breadth_destroy;
			next    = graph_tree_iterator_breadth_next;
			break;
	}
	graph *rtn = NULL;
	graph *g;
	for(g = next(iter); g; g = next(iter)){
		if(key_method->compare(key, g->data) == 0){
			rtn = g;
			break;
		}
	}
	destroy(iter);
	return rtn;
}

void
graph_tree_size(graph* root, size_t *nodes, size_t *edges){
	size_t i_nodes = 0;
	size_t i_edges = 0;
	graph_tree_iterator_pre *iter =
		graph_tree_iterator_pre_new(root, &(graph_tree_iterator_pre){});
	graph *g;
	for(g = graph_tree_iterator_pre_next(iter); g; g = graph_tree_iterator_pre_next(iter)){
		i_nodes++;
		i_edges += dlist_length(g->edges);
	}
	graph_tree_iterator_pre_destroy(iter);
	if(nodes) *nodes = i_nodes;
	if(edges) *edges = i_edges;
}
