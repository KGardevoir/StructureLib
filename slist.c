#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

static slist*
new_slist(Object* data, BOOLEAN deep_copy, slist* next){
	slist *lnew = (slist*)MALLOC(sizeof(slist));
	slist init = {
		.next = next,
		.data = deep_copy?data->method->copy(data, MALLOC(data->method->size)):data
	};
	memcpy(lnew, &init, sizeof(*lnew));
	return lnew;
}

slist*
slist_push(slist* he, Object* buf, BOOLEAN copy){
	slist *lm;
	lm = new_slist(buf, copy, NULL);
	if(he == NULL){
		he = lm;
	} else {
		lm->next = he;
		he = lm;
	}
	return he;
}

slist*
slist_append(slist* he, Object* buf, BOOLEAN copy){
	slist *lm, *he2;
	lm = new_slist(buf, copy, NULL);
	if(he == NULL){
		he = lm;
	} else {
		for(he2 = he; he2->next != NULL; he2 = he2->next);
		he2->next = lm;
	}
	return he;
}

slist*
slist_addOrdered(slist* he, Object* buf, const Comparable_vtable* buf_method, BOOLEAN deep_copy, BOOLEAN overwrite){
	slist *run, *lm, *runp = NULL;
	lm = new_slist((Object*)buf, deep_copy, NULL);
	if(he == NULL) {
		he = lm;
	} else {
		for(run = he; run && buf_method->compare(buf, run->data) > 0; run = run->next){
			runp = run;
		}
		if(overwrite && run && buf_method->compare(buf, run->data) == 0){
			run->data->method->destroy(run->data);
			run->data = lm->data;
			FREE(lm);
		} else {
			if(runp){
				runp->next = lm;
				lm->next = run;
			} else {
				lm->next = he;
				he = lm;
			}
		}
	}
	return he;
}

slist*
slist_copy(slist* src, BOOLEAN deep_copy){
	slist *the_list = NULL, *list_start = NULL, *runner = src;
	Object* new_data = runner->data;
	the_list = list_start = slist_push(NULL, new_data, deep_copy);
	runner = runner->next;
	for(; runner != NULL; runner = runner->next){
		Object* new_data = runner->data;
		the_list = slist_append(the_list, new_data, deep_copy)->next;
	}
	return list_start;
}

slist*
slist_concat(slist *src, slist *tail){
	if(src == NULL) return tail;
	if(tail == NULL) return src;
	slist *runner = src;
	for(; runner->next != NULL; runner = runner->next);
	runner->next = tail;
	return src;
}


void
slist_clear(slist *head, BOOLEAN destroy_data){
	slist *run = head, *p = head;
	for(; run; run = p){
		if(destroy_data) run->data->method->destroy(run->data);
		p = run->next;
		FREE(run);
	}
}

size_t
slist_length(slist *head){
	if(head == NULL) return 0;
	slist *run = head;
	size_t i = 0;
	for(; run; run = run->next, i++);
	return i;
}

slist*
slist_dequeue(slist *head, Object** data, BOOLEAN destroy_data){
	if(!head) return NULL;
	slist *run = head;
	if(!run->next){
		if(data) *data = run->next->data;
		if(destroy_data) run->data->method->destroy(run->data);
		FREE(run);
		return NULL;
	}
	for(; run->next && run->next->next; run = run->next);
	if(data) *data = run->next->data;
	if(destroy_data) run->next->data->method->destroy(run->next->data);
	FREE(run->next);
	run->next = NULL;
	return head;
}
slist*
slist_pop(slist *head, Object** data, BOOLEAN destroy_data){
	slist *run = head;
	if(!head) return head;
	run = head;
	head = head->next;
	if(data) *data = run->data;
	if(destroy_data) run->data->method->destroy(run->data);
	FREE(run);
	return head;
}

slist*
slist_remove(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data){
	slist *run = head, *runp = NULL;
	if(head == NULL) return head;
	for(run = head; run && (ordered?key_method->compare(key, run->data) <= 0:key_method->compare(key, run->data) != 0); run = run->next){
		runp = run;
	}
	if(run && key_method->compare(key, run->data) == 0){
		if(runp){
			runp->next = run->next;
			if(destroy_data) run->data->method->destroy(run->data);
			FREE(run);
		} else {
			head = run->next;
			if(destroy_data) run->data->method->destroy(run->data);
			FREE(run);
		}
	}
	return head;
}
slist*
slist_removeAll(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data){
	slist *run = head, *runp = NULL;
	if(head == NULL) {
		return NULL;
	}
	for(run = head; run; run = run->next){
		if(ordered && key_method->compare(key, run->data) > 0) break;
		if(key_method->compare(key, run->data) == 0){
			if(runp){
				runp->next = run->next;
				if(destroy_data) run->data->method->destroy(run->data);
				FREE(run);
			} else {
				head = run->next;
				if(destroy_data) run->data->method->destroy(run->data);
				FREE(run);
			}
		}
		runp = run;
	}
	return head;
}
slist*
slist_removeElement(slist *head, slist *rem, BOOLEAN destroy_data){
	slist *run = head, *runp = NULL;
	if(head == NULL) return head;
	for(run = head; run && run != 0; run = run->next){
		runp = run;
	}
	if(run && rem == run){
		if(runp){
			runp->next = run->next;
			if(destroy_data) run->data->method->destroy(run->data);
			FREE(run);
		} else {
			head = run->next;
			if(destroy_data) run->data->method->destroy(run->data);
			FREE(run);
		}
	}
	return head;
}


//Other Functions
BOOLEAN
slist_map(slist *head, BOOLEAN more_info, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!head) return TRUE;
	slist *p = head;
	size_t depth = 0;
	size_t length = 0;
	if(more_info) length = slist_length(head);
	for(; p->next; p = p->next){
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.position = depth,
				.depth = depth,
				.size = length,
				.aux = aux
			};
			if(!func(p->data, &ax)) return FALSE;
		} else {
			if(!func(p->data, aux)) return FALSE;
		}
		depth++;
	}
	return TRUE;
}


Object**
slist_toArray(slist *head, size_t *size, BOOLEAN deep){
size_t len = 0;
slist* p = head;
	if(!head) return NULL;
	for(; p->next; p = p->next, len++);
	Object **values = (Object**)MALLOC((len+1)*sizeof(Object*));
	len = 0;
	SLIST_ITERATE(p, head, {
		if(deep){
			values[len] = p->data->method->copy(p->data, MALLOC(p->data->method->size));
		} else {
			values[len] = p->data;
		}
		len++;
	});
	values[len] = NULL;
	*size = len;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}

Object**
slist_toArrayReverse(slist *head, size_t *size, BOOLEAN deep){
size_t i = 0, len = 0;
	Object **values = slist_toArray(head, &len, deep);
	if(values == NULL) return NULL;
	*size = len;
	for(i = 0; i < len/2; i++){//flip them around
		Object *k = values[len-i-1];
		values[len-i-1] = values[i];
		values[i] = k;
	}
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}

slist*
array_toSlist(Object** arr, size_t size, BOOLEAN deep){
	if(arr == NULL) return NULL;
	if(size == 0) return NULL;
	size_t i = size;
	slist *head = NULL;
	for(; --i; ){
		head = slist_push(head, arr[i], deep);
	}
	return head;
}

dlist*
slist_toDlist(slist *head, BOOLEAN deep){
	slist* p = head;
	dlist* d = NULL;
	for(; p; p = p->next){
		d = dlist_append(d, p->data, deep);
	}
	return d;
}
//End Single Linked Transforms


slist*
slist_find(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered){
	if(!head) return NULL;
slist* p = head;
	for(; p->next && (ordered?key_method->compare(key, p->data) > 0:key_method->compare(key, p->data) != 0); p = p->next);
	if(p && key_method->compare(key, p->data) == 0) return p;
	return NULL;
}
