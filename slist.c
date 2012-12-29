#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"


static long
memcomp(void *a, void* b){ return (a>b?1:(a<b?-1:0)); }

static slist*
new_slist(void* data, BOOLEAN deep_copy, slist* next, list_tspec *type){
	deep_copy = deep_copy && type && type->deep_copy;
	slist *new = (slist*)MALLOC(sizeof(slist));
	slist init = {
		.next = next,
		.data = deep_copy?type->deep_copy(data):data
	};
	memcpy(new, &init, sizeof(*new));
	return new;
}

slist*
slist_push(slist* he, void* buf, BOOLEAN copy, list_tspec *type){
	slist *lm;
	lm = new_slist(buf, copy, NULL, type);
	if(he == NULL){
		he = lm;
	} else {
		lm->next = he;
		he = lm;
	}
	return he;
}

slist*
slist_append(slist* he, void* buf, BOOLEAN copy, list_tspec* type){
	slist *lm, *he2;
	lm = new_slist(buf, copy, NULL, type);
	if(he == NULL){
		he = lm;
	} else {
		for(he2 = he; he2->next != NULL; he2 = he2->next);
		he2->next = lm;
	}
	return he;
}

slist*
slist_addOrdered(slist* he, void* buf, BOOLEAN deep_copy, BOOLEAN overwrite, list_tspec* type){
	slist *run, *lm, *runp = NULL;
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	lm = new_slist(buf, deep_copy, NULL, type);
	if(he == NULL) {
		he = lm;
	} else {
		for(run = he; run && compar(buf, run->data) > 0; run = run->next){
			runp = run;
		}
		if(overwrite && run && compar(buf, run->data) == 0){
			if(type && type->destroy)
				type->destroy(run->data);
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
slist_copy(slist* src, BOOLEAN deep_copy, list_tspec* type){
	slist *the_list, *list_start, *runner = src;
	void* new_data = runner->data;
	the_list = list_start = slist_push(NULL, new_data, deep_copy, type);
	runner = runner->next;
	for(; runner != NULL; runner = runner->next){
		void* new_data = runner->data;
		the_list = slist_append(the_list, new_data, deep_copy, type)->next;
	}
	return list_start;
}


void
slist_clear(slist *head, BOOLEAN destroy_data, list_tspec* type){
	slist *run = head, *p = head;
	destroy_data = destroy_data && type && type->destroy;
	for(; run; run = p){
		if(destroy_data) type->destroy(run->data);
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
slist_dequeue(slist *head, void** data, BOOLEAN destroy_data, list_tspec* type){
	if(!head) return NULL;
	destroy_data = destroy_data && type && type->destroy;
	slist *run = head;
	if(!run->next){
		if(data) *data = run->next->data;
		if(destroy_data) type->destroy(run->data);
		FREE(run);
		return NULL;
	}
	for(; run->next && run->next->next; run = run->next);
	if(data) *data = run->next->data;
	if(destroy_data) type->destroy(run->next->data);
	FREE(run->next);
	run->next = NULL;
	return head;
}
slist*
slist_pop(slist *head, void** data, BOOLEAN destroy_data, list_tspec* type){
	slist *run = head;
	if(!head) return head;
	destroy_data = destroy_data && type && type->destroy;
	run = head;
	head = head->next;
	if(data) *data = run->data;
	if(destroy_data) type->destroy(run->data);
	FREE(run);
	return head;
}
slist*
slist_removeViaKey(slist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec* type){
	slist *run = head, *runp = NULL;
	if(head == NULL) return head;
	lKeyCompare key_compar = (type && type->key_compar)?type->key_compar:(lKeyCompare)memcomp;
	destroy_data = destroy_data && type && type->destroy;
	for(run = head; run && (ordered?key_compar(key, run->data) <= 0:key_compar(key, run->data) != 0); run = run->next){
		runp = run;
	}
	if(run && key_compar(key, run->data) == 0){
		if(runp){
			runp->next = run->next;
			if(data) *data = run->data;
			if(destroy_data) type->destroy(run->data);
			FREE(run);
		} else {
			head = run->next;
			if(data) *data = run->data;
			if(destroy_data) type->destroy(run->data);
			FREE(run);
		}
	}
	return head;
}
slist*
slist_removeAllViaKey(slist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec* type){
	slist *run = head, *runp = NULL;
	if(head == NULL) {
		return NULL;
	}
	lKeyCompare key_compar = (type && type->key_compar)?type->key_compar:(lKeyCompare)memcomp;
	destroy_data = destroy_data && type && type->destroy;
	for(run = head; run; run = run->next){
		if(ordered && key_compar(key, run->data) > 0) break;
		if(key_compar(key, run->data) == 0){
			if(runp){
				runp->next = run->next;
				if(data) *data = run->data;
				if(destroy_data) type->destroy(run->data);
				FREE(run);
			} else {
				head = run->next;
				if(data) *data = run->data;
				if(destroy_data) type->destroy(run->data);
				FREE(run);
			}
		}
		runp = run;
	}
	return head;
}
slist*
slist_removeElement(slist *head, slist *rem, BOOLEAN destroy_data, list_tspec* type){
	slist *run = head, *runp = NULL;
	if(head == NULL) return head;
	destroy_data = destroy_data && type && type->destroy;
	for(run = head; run && run != 0; run = run->next){
		runp = run;
	}
	if(run && rem == run){
		if(runp){
			runp->next = run->next;
			if(destroy_data) type->destroy(run->data);
			FREE(run);
		} else {
			head = run->next;
			if(destroy_data) type->destroy(run->data);
			FREE(run);
		}
	}
	return head;
}


//Other Functions
BOOLEAN
slist_map(slist *head, void* aux, lMapFunc func){
	if(!func) return FALSE;
	if(!head) return TRUE;
	slist *p = head;
	for(; p->next; p = p->next) if(!func(p->data, aux)) return FALSE;
	return TRUE;
}


void**
slist_toArray(slist *head, BOOLEAN deep, list_tspec* type){
size_t len = 0;
slist* p = head;
	if(!head) return NULL;
	deep = deep && type && type->deep_copy;
	for(; p->next && p->data; p = p->next, len++);
	void **values = (void**)MALLOC((len+1)*sizeof(void*));
	for(len = 0, p = head; p->next && p->data; len++){
		if(deep){
			values[len] = type->deep_copy(p->data);
		} else {
			values[len] = p->data;
		}
	}
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}

void**
slist_toArrayReverse(slist *head, BOOLEAN deep, list_tspec* type){
size_t i = 0, len = 0;
slist* p = head;
	if(!head) return NULL;
	deep = deep && type && type->deep_copy;
	for(; p && p->data; p = p->next, i++);
	len = i;
	void **values = (void**)MALLOC((len+1)*sizeof(void*));
	for(i = 0, p = head; p && p->data; i++, p = p->next){
		if(deep){
			values[len] = type->deep_copy(p->data);
		} else {
			values[len] = p->data;
		}
	}
	for(i = 0; i < len/2; i++){
		void **k = values[len-i-1];
		values[len-i-1] = values[i];
		values[i] = k;
	}
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}

dlist*
slist_to_dlist(slist *head, BOOLEAN deep, list_tspec* type){
	slist* p = head;
	dlist* d = NULL;
	for(; p; p = p->next){
		d = dlist_append(d, p->data, deep, type);
	}
	return d;
}
//End Single Linked Transforms


void*
slist_find(slist *head, void* key, BOOLEAN ordered, list_tspec *type){
	if(!head) return NULL;
slist* p = head;
	lKeyCompare key_compar = (type && type->key_compar)?type->key_compar:(lKeyCompare)memcomp;
	for(; p->next && (ordered?key_compar(key, p->data) > 0:key_compar(key, p->data) != 0); p = p->next);
	if(p && key_compar(key, p->data) == 0) return p->data;
	return NULL;
}
