#include "udlist.h"

static inline udlist_node*
udlist_node_new(udlist_node *next, udlist_node *prev, size_t n_ele){
	udlist_node nl = {
		.next = next,
		.prev = prev,
		.n_filled = 0,
	};
	return memcpy(LINKED_MALLOC(sizeof(udlist)+sizeof(Object*[n_ele])), &nl, sizeof(udlist));
}

static udlist_node*
udlist_node_split(udlist* head, udlist_node *node, size_t idx){
	if(idx < node->n_filled){
		udlist_node *next = udlist_node_new(node->next, node, head->n_elements);
		node->next->prev = next;
		node->next = next;
		memcpy(&next->elements[0], &node->elements[idx], (next->n_filled = head->n_elements-idx)*sizeof(next->elements[0]));
		node->n_filled = idx;
		return next;
	}
	return node;
}
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })

//make node have at least n_nodes in it, one must be certain that n_nodes < head->n_elements
static udlist_node*
udlist_node_merge(udlist *head, udlist_node *node, size_t n_nodes){
	while(node->n_filled < n_nodes && node->next != head->root){
		size_t cpy_num = min(n_nodes-node->n_filled, node->next->n_filled);
		memcpy(&node->elements[node->n_filled], &node->next->elements[0], cpy_num*sizeof(node->elements[0]));
		node->next->n_filled -= cpy_num;
		node->n_filled += cpy_num;
		if(node->next->n_filled == 0){
			LINKED_FREE(node->next);
			node->next = node->next->next;
			node->next->prev = node;
		}
	}
	return node;
}

static Object*
udlist_node_remove(udlist *head, udlist_node *node, size_t idx){
	if(idx >= node->n_filled) return NULL;
	Object *data = node->elements[idx];
	if(idx != node->n_filled-1) memmove(&node->elements[idx], &node->elements[idx+1], (node->n_filled-1)-idx);
	node->n_filled--;
	head->size--;
	if(node->n_filled < head->n_elements/2){
		udlist_node_merge(head, node, head->n_elements/2);
	}
	return data;
}

static void
udlist_node_insert(udlist* head, udlist_node *node, size_t pos, Object *obj){
	if(node->n_filled == head->n_elements){
		udlist_node_split(head, node, head->n_elements/2);
	}
	if(pos > node->n_filled){
		pos = node->n_filled;
	} else if(pos < node->n_filled){
		memcpy(&node->elements[pos+1], &node->elements[pos], sizeof(node->elements[0]));
	}
	node->elements[pos] = obj;
	node->n_filled++;
	head->size++;
}

udlist*
udlist_new(size_t n_elements){
	udlist list = {
		.size = 0,
		.n_elements = n_elements,
		.root = NULL
	};
	return memcpy(LINKED_MALLOC(sizeof(list)), &list, sizeof(list));
}

void
udlist_destroy(udlist* head){
	LINKED_FREE(head);
}

void
udlist_pushback(udlist *head, Object *obj, BOOLEAN copy){
	udlist_node *n;
	if(head->root){
		n = head->root->prev;
	} else {
		n = udlist_node_new(NULL, NULL, head->n_elements);
		n->next = n;
		n->prev = n;
		head->root = n;
	}
	udlist_node_insert(head, n, n->n_filled, copy?CALL(obj, copy, obj, LINKED_MALLOC(obj->method->size)):obj);
}

void
udlist_pushfront(udlist *head, Object *obj, BOOLEAN copy){
	if(head->root){
		udlist_node *n = head->root;
		udlist_node_insert(head, n, 0, copy?CALL(obj, copy, obj, LINKED_MALLOC(obj->method->size)):obj);
	} else {
		udlist_pushback(head, obj, copy);
		return;
	}
}

void
udlist_insert(udlist *head, Object *obj, size_t idx, BOOLEAN copy){
	if(idx == 0 || !head->root){
		udlist_pushfront(head, obj, copy);
	} else if(idx == head->size){
		udlist_pushback(head, obj, copy);
	} else {
		size_t cidx = 0;
		udlist_node *run = head->root;
		do {
			if(cidx + run->n_filled > idx){
				break;
			}
			cidx += run->n_filled;
			run = run->next;
		} while(head->root != run);
		udlist_node_insert(head, run, idx - cidx, copy?CALL(obj, copy, obj, LINKED_MALLOC(obj->method->size)):obj);
	}
}

void
udlist_addOrdered(udlist* head, Object* obj, const Comparable_vtable* buf_method, BOOLEAN copy){
	udlist_node *run = head->root;
	do {
		size_t idx = 0;
		for(; idx < run->n_filled; idx++){
			if(buf_method->compare(obj, run->elements[idx]) > 0){
				udlist_node_insert(head, run, idx, copy?CALL(obj, copy, obj, LINKED_MALLOC(obj->method->size)):obj);
				return;
			}
		}
		run = run->next;
	} while(head->root != run);
	udlist_pushback(head, obj, copy);
}

void
udlist_compact(udlist *head){//modify list so every node contains 1 less than the maximum size
	udlist_node *run = head->root;
	do {
		udlist_node_merge(head, run, head->n_elements);
		run = run->next;
	} while(head->root != run);
}

udlist* //copy into a very large chunk
udlist_copy(udlist* head, BOOLEAN copy){
	udlist *cpy = udlist_new(head->size);
	udlist_node *run = head->root;
	do {
		size_t idx = 0;
		for(; idx < run->n_filled; idx++){
			udlist_pushback(cpy, run->elements[idx], copy);
		}
		run = run->next;
	} while(head->root != run);
	return cpy;
}

void
udlist_clear(udlist *head, BOOLEAN destroy_data){
	udlist_node *run = head->root;
	if(!head) return;
	if(head->root){
		do {
			if(destroy_data){
				size_t i = 0;
				for(; i < run->n_filled; i++){
					CALL_VOID(run->elements[i], destroy);
				}
			}
			run = run->next;
			LINKED_FREE(run->prev);
		} while(head->root != run);
	}
}

Object*
udlist_popback(udlist* head, BOOLEAN destroy_data){
	Object *data = udlist_node_remove(head, head->root->prev, head->root->prev->n_filled);
	if(destroy_data) CALL_VOID(data, destroy);
	return data;
}

Object*
udlist_popfront(udlist* head, BOOLEAN destroy_data){
	Object *data = udlist_node_remove(head, head->root, 0);
	if(destroy_data) CALL_VOID(data, destroy);
	return data;
}

Object*
udlist_delete(udlist *head, size_t idx, BOOLEAN destroy_data){
	if(idx >= head->size) return NULL;
	udlist_node *run = head->root;
	size_t cidx = 0;
	do {
		if(cidx + run->n_filled > idx){
			Object *data = udlist_node_remove(head, run, idx-cidx);
			if(destroy_data) CALL_VOID(data, destroy);
			return data;
		}
		cidx += run->n_filled;
		run = run->next;
	} while(head->root != run);
	return NULL;
}

void
udlist_removeAll(udlist* head, void *key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data){
	if(!head) return;
	if(!head->root) return;
	udlist_node *run = head->root;
	size_t i = 0;
	do{
		for(i = 0; i < run->n_filled;){
			if(ordered && key_method->compare(key, run->elements[i]) > 0) return;
			if(key_method->compare(key, run->elements[i]) == 0){
				udlist_node_remove(head, run, i);
				continue;
			}
			i++;
		}
		run = run->next;
	} while(run != head->root);
}

Object*
udlist_remove(udlist* head, void *key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data){
	if(!head) return NULL;
	if(!head->root) return NULL;
	udlist_node *run = head->root;
	size_t i = 0;
	do {
		for(i = 0; i < run->n_filled; i++){
			if(ordered?key_method->compare(key, run->elements[i]) >= 0:key_method->compare(key, run->elements[i]) == 0) goto end;
		}
		run = run->next;
	} while(run != head->root);
end:
	if(key_method->compare(key, run->elements[i]) == 0){
		return udlist_node_remove(head, run, i);
	}
	return NULL;
}

Object**
udlist_at(udlist *head, size_t idx){
	if(idx >= head->size) return NULL;
	udlist_node *run = head->root;
	size_t cidx = 0;
	do {
		if(cidx + run->n_filled > idx){
			return &run->elements[idx-cidx];
		}
		cidx += run->n_filled;
		run = run->next;
	} while(head->root != run);
	return NULL;
}

