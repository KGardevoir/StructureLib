#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

static graph*
new_graph_node(void* key, void* data, BOOLEAN copy, list_tspec* type){
	graph init = {
		.key = key,
		.data = copy?type->deep_copy(data):data,
		.edges = NULL
	}
	graph *mem = MALLOC(sizeof(graph));
	memcpy(mem, &init, sizeof(*mem));
	return mem;
}

graph*
graph_insert(graph* root, void* key, void* data, BOOLEAN copy, list_tspec* type){
	//add a new node the child of this node.
	root->edges = dlist_append(root->edges, new_graph_node(key, data, copy), false, type);
	return root;
}

graph*
graph_link(graph* root, graph* child){

}
