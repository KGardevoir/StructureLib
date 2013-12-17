#include "graph.h"
#include "slist.h"
#include "splaytree.h"

static void graph_node_destroy(graph* self){ dlist_clear(self->edges, FALSE); }
static char* graph_node_hashable(const graph* self, size_t *size){ *size = self->method->parent.size; return (char*)self; }
static long graph_node_compare(graph* self, graph* oth);
static graph* graph_node_copy(graph* self, void* mem);
static graph_vtable _graph_vtable = {
		.parent = {
			.copy = (Object*(*)(const Object *self, void* mem))graph_node_copy,
			.destroy  = (void(*)(const Object *self))graph_node_destroy,
			.hashable = (char*(*)(const Object *self, size_t* size))graph_node_hashable,
			.equals   = NULL,
			.size = sizeof(graph)
		},
		.comparable = {
			.compare = (long(*)(const void* self, const void *oth))graph_node_compare
		}
	};
static long graph_node_compare(graph* self, graph* oth){ return self>oth?1:(self==oth?0:-1); }
//static long graph_node_compare(graph* self, graph* oth){
//	return ((aComparable*)self->data)->method->compare(self->data, oth->data);
//}

static inline graph*
graph_node_new(graph* self, Object* data, BOOLEAN copy){
	graph init = {
		.method = &_graph_vtable,
		.data = copy?CALL(data,copy,data, LINKED_MALLOC(self->method->parent.size)):data,
		.edges = NULL
	};
	//graph *mem = (graph*)LINKED_MALLOC(sizeof(graph));
	memcpy(self, &init, sizeof(init));
	return self;
}

static graph*
graph_node_copy(graph* self, void* mem){
	graph init = {
		.method = &_graph_vtable,
		.data = self->data,
		.edges = self->edges
	};
	memcpy(mem, &init, sizeof(*mem));
	return mem;
}


graph*
graph_insert(graph* root, Object* data, BOOLEAN copy){
	//add a new node the child of this node.
	return graph_link(root, graph_node_new(LINKED_MALLOC(_graph_vtable.parent.size), data, copy));
}

graph*
graph_link(graph* root, graph* child){
	if(child == NULL) return root;
	if(root == NULL) return child;
	//printf("%p %p %p\n", child, child->data, child->edges);
	root->edges = dlist_pushback(root->edges, (Object*)child, FALSE);
	//Rather than adding ordered, order based on arrival time
	return root;
}

#if 0
graph*
graph_remove(graph* root, void* key, void** data, BOOLEAN copy, list_tspec* type){
#error "Unimplmented"
}
#endif

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

static BOOLEAN
graph_map_filter_f(node_info *child, splaytree* tree){
	Object *found = splay_find(tree, (Object*)child->node, &child->node->method->comparable);
	if(found){
		//printf("E");
		return FALSE;
	}
	//printf("N");
	return TRUE;
}

#if 0
#include "aLong.h"
#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC) do { printf(BSTR); dlist *_run; DLIST_ITERATE(_run, LST) printf(FMT, ACC); printf(ESTR); } while(0)
static BOOLEAN
graph_print_children_f(aLong *data){
	printf("%ld ", data->data);
	return TRUE;
}

static void
graph_print_children(dlist *head){
	dlist *run;
}
#else
#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC)
#endif

static graph*
graph_iterator_post_next(graph_iterator_post *self){
	if(!self->i_stk){
		self->iterator.r_current = NULL;
		self->iterator.r_depth = 0;
		return NULL;
	}
	while(TRUE){//expand nodes to maximum (left-most)
		node_info *g;
		size_t prev_size = dlist_length(self->i_stk);
		self->i_stk = dlist_popfront(self->i_stk, (Object**)&g, FALSE);
		splay_insert(self->i_visited, (Object*)g->node, FALSE);
		//stk = dlist_filter_i(stk, &visited, (lMapFunc)graph_map_filter_f, FALSE);
		dlist *new_list = dlist_copy(g->node->edges, FALSE);
		dlist *run;
		DLIST_ITERATE(run, new_list)
			run->data = (void*)node_info_new((graph*)run->data, g->depth+1);

		new_list = dlist_filter_i(new_list, self->i_visited, (lMapFunc)graph_map_filter_f, TRUE);
		self->i_stk = dlist_concat(dlist_pushback(new_list, (Object*)g, FALSE), self->i_stk);
		size_t new_size = dlist_length(self->i_stk);
		if(new_size <= prev_size) break;
	}//expand till we can't expand any more.
	node_info *g;
	self->i_stk = dlist_popfront(self->i_stk, (Object**)&g, FALSE);
	splay_insert(self->i_processed, (Object*)g->node, FALSE);
	self->i_stk = dlist_filter_i(self->i_stk, self->i_processed, (lMapFunc)graph_map_filter_f, TRUE);//clear entries we have now processed

	self->iterator.r_current = (graph*)g->node;
	self->iterator.r_depth = g->depth;
	LINKED_FREE(g);
	return self->iterator.r_current;
}

static void
graph_iterator_post_destroy(graph_iterator_post* self){
	splay_clear(self->i_visited, FALSE);
	splay_clear(self->i_processed, FALSE);
	splaytree_destroy(self->i_visited);
	splaytree_destroy(self->i_processed);
	self->i_visited = NULL;
	self->i_processed = NULL;
	dlist_clear(self->i_stk, TRUE);
	self->i_root = NULL;
	self->iterator.r_depth = 0;
	self->iterator.r_current = NULL;
}

const static struct Iterator_vtable graph_iterator_post_vtable = {
	.parent = {
		.destroy = (void(*)(const Object*))graph_iterator_post_destroy
	},
	.next = (void*(*)(const Object*))graph_iterator_post_next
};

graph_iterator_post*
graph_iterator_post_new(graph* root, graph_iterator_post *mem){
	mem->i_visited = splaytree_new(&root->method->comparable);
	mem->i_processed = splaytree_new(&root->method->comparable);
	mem->i_root = root;
	mem->i_stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	mem->iterator.r_depth = 0;
	mem->iterator.r_current = NULL;
	mem->iterator.p_add_children = TRUE;
	mem->iterator.method = &graph_iterator_post_vtable;
	return mem;
}

static graph*
graph_iterator_pre_next(graph_iterator_pre* self){
	if(self->i_children){
		if(self->iterator.p_add_children){
			self->i_stk = dlist_concat(self->i_children, self->i_stk);
		} else {
			dlist_clear(self->i_children, TRUE);
		}
		self->i_children = NULL;
	}
	self->i_stk = dlist_filter_i(self->i_stk, self->i_visited, (lMapFunc)graph_map_filter_f, TRUE);
	if(!self->i_stk){
		self->iterator.r_current = NULL;
		self->iterator.r_depth = 0;
		return NULL;
	}
	node_info *g;
	self->i_stk = dlist_popfront(self->i_stk, (Object**)&g, FALSE);
	splay_insert(self->i_visited, (Object*)g->node, FALSE);

	dlist *new_list = dlist_copy(g->node->edges, FALSE);
	dlist *run = NULL;
	DLIST_ITERATE(run, new_list){
		run->data = (void*)node_info_new((graph*)run->data, g->depth+1);
	}
	self->i_children = new_list;

	self->iterator.r_current = (graph*)g->node;
	self->iterator.r_depth = g->depth;
	self->iterator.p_add_children = TRUE;
	LINKED_FREE(g);
	return self->iterator.r_current;
}

static void
graph_iterator_pre_destroy(graph_iterator_pre* self){
	splay_clear(self->i_visited, FALSE);
	splaytree_destroy(self->i_visited);
	self->i_visited = NULL;
	dlist_clear(self->i_stk, TRUE);
	dlist_clear(self->i_children, TRUE);
	self->i_root = NULL;
	self->i_stk = NULL;
	self->iterator.r_depth = 0;
	self->iterator.r_current = NULL;
	self->iterator.p_add_children = TRUE;
}

const static struct Iterator_vtable graph_iterator_pre_vtable = {
	.parent = {
		.destroy = (void(*)(const Object*))graph_iterator_pre_destroy
	},
	.next = (void*(*)(const Object*))graph_iterator_pre_next
};

graph_iterator_pre*
graph_iterator_pre_new(graph *root, graph_iterator_pre* mem){
	mem->i_root = root;
	mem->i_stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	mem->i_visited = splaytree_new(&root->method->comparable);
	mem->i_children = NULL;
	mem->iterator.r_depth = 0;
	mem->iterator.r_current = NULL;
	mem->iterator.p_add_children = TRUE;
	mem->iterator.method = &graph_iterator_pre_vtable;
	return mem;
}


static graph*
graph_iterator_breadth_next(graph_iterator_breadth* self){
	if(self->i_children){
		if(self->iterator.p_add_children){
			self->i_stk = dlist_concat(self->i_stk, self->i_children);
		} else {
			dlist_clear(self->i_children, TRUE);
		}
		self->i_children = NULL;
	}
	self->i_stk = dlist_filter_i(self->i_stk, self->i_visited, (lMapFunc)graph_map_filter_f, TRUE);
	if(!self->i_stk){
		self->iterator.r_current = NULL;
		self->iterator.r_depth = 0;
		return NULL;
	}
	node_info *g;
	self->i_stk = dlist_popfront(self->i_stk, (Object**)&g, FALSE);
	splay_insert(self->i_visited, (Object*)g->node, FALSE);

	dlist *new_list = dlist_copy(g->node->edges, FALSE);
	dlist *run = NULL;
	DLIST_ITERATE(run, new_list){
		run->data = (void*)node_info_new((graph*)run->data, g->depth+1);
	}
	self->i_children = new_list;

	self->iterator.r_current = (graph*)g->node;
	self->iterator.r_depth = g->depth;
	self->iterator.p_add_children = TRUE;
	LINKED_FREE(g);
	return self->iterator.r_current;
}

static void
graph_iterator_breadth_destroy(graph_iterator_breadth* self){
	splay_clear(self->i_visited, FALSE);
	splaytree_destroy(self->i_visited);
	self->i_visited = NULL;
	dlist_clear(self->i_stk, TRUE);
	dlist_clear(self->i_children, TRUE);
	self->i_root = NULL;
	self->i_stk = NULL;
	self->iterator.r_depth = 0;
	self->iterator.r_current = NULL;
	self->iterator.p_add_children = TRUE;
}

const static struct Iterator_vtable graph_iterator_breadth_vtable = {
	.parent = {
		.destroy = (void(*)(const Object*))graph_iterator_breadth_destroy
	},
	.next = (void*(*)(const Object*))graph_iterator_breadth_next
};
graph_iterator_breadth*
graph_iterator_breadth_new(graph *root, graph_iterator_breadth* mem){
	mem->i_root = root;
	mem->i_stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	mem->i_visited = splaytree_new(&root->method->comparable);
	mem->i_children = NULL;
	mem->iterator.r_depth = 0;
	mem->iterator.r_current = NULL;
	mem->iterator.p_add_children = TRUE;
	mem->iterator.method = &graph_iterator_breadth_vtable;
	return mem;
}

//#include "aLong.h"
void
graph_clear(graph *root, BOOLEAN destroy_data){
	slist *head = NULL;
	graph_iterator_breadth *it = graph_iterator_breadth_new(root, &(graph_iterator_breadth){});
	size_t i = 0;
	graph *g;
	for(g = it->iterator.method->next((Object*)it); g; g = it->iterator.method->next((Object*)it)){
		if(destroy_data) CALL_VOID(g->data, destroy);
		head = slist_pushfront(head, (Object*)g, FALSE);
	}
	it->iterator.method->parent.destroy((Object*)it);
	slist *iter;
	SLIST_ITERATE(iter, head){
		CALL_VOID(iter->data, destroy);
		LINKED_FREE(iter->data);
	}
	slist_clear(head, FALSE);
}

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define MAX(a,b) ( (a)>(b)?a:b )

graph*
graph_find(graph *root, TRAVERSAL_STRATEGY strat, void* key, const Comparable_vtable* key_method){
	graph_iterator *iter = (void*)&(char[MAX(sizeof(graph_iterator_breadth), MAX(sizeof(graph_iterator_pre), sizeof(graph_iterator_post)))]){0};
	switch(strat){
		//select iterator and next function
		case DEPTH_FIRST_POST:
			iter    = (graph_iterator*)graph_iterator_post_new(root, (graph_iterator_post*)iter);
			break;
		case DEPTH_FIRST_PRE:
			iter    = (graph_iterator*)graph_iterator_pre_new(root, (graph_iterator_pre*)iter);
			break;
		default:
		case BREADTH_FIRST:
			iter    = (graph_iterator*)graph_iterator_breadth_new(root, (graph_iterator_breadth*)iter);
			break;
	}
	graph *rtn = NULL;
	graph *g;
	for(g = iter->method->next((Object*)iter); g; g = iter->method->next((Object*)iter)){
		if(key_method->compare(key, g->data) == 0){
			rtn = g;
			break;
		}
	}
	iter->method->parent.destroy((Object*)iter);
	return rtn;
}

void
graph_size(graph* root, size_t *nodes, size_t *edges){
	size_t i_nodes = 0;
	size_t i_edges = 0;
	graph_iterator_pre *iter =
		graph_iterator_pre_new(root, &(graph_iterator_pre){});
	graph *g;
	for(g = graph_iterator_pre_next(iter); g; g = graph_iterator_pre_next(iter)){
		i_nodes++;
		i_edges += dlist_length(g->edges);
	}
	graph_iterator_pre_destroy(iter);
	if(nodes) *nodes = i_nodes;
	if(edges) *edges = i_edges;
}

graph*
graph_path_key_match(graph *root, dlist *key_path){
	size_t size = dlist_length(key_path);
	size_t i = 0;
	dlist *run = key_path;
	graph *troot = root;
	while(i < size){
		dlist *descent = dlist_find(troot->edges, (void*)run->data, &root->method->comparable, FALSE);
		if(descent == NULL) return NULL;
		troot = (graph*)descent->data;
		i++; run = run->next;
	}
	return troot;
}

dlist*
graph_topological_sort(graph* tree){
	dlist *list = NULL;
	graph_iterator_post *iter =
		graph_iterator_post_new(tree, &(graph_iterator_post){});
	graph *g;
	for(g = graph_iterator_post_next(iter); g; g = graph_iterator_post_next(iter)){
		list = dlist_pushback(list, (Object*)tree, FALSE);
	}
	graph_iterator_post_destroy(iter);
	return list;
}

#if 0
graph*
graph_spanning(graph* root, long(edge_weight)(graph*,graph*)){
	//TODO utilize Kruskal's Algorithm
}
#endif
