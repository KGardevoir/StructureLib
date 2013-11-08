#include "slist.h"

static inline slist*
new_slist(Object* data, BOOLEAN deep_copy, slist* next){
	slist init = {
		.next = next,
		.data = deep_copy?CALL(data, copy,data, LINKED_MALLOC(data->method->size)):data
	};
	return memcpy(LINKED_MALLOC(sizeof(slist)), &init, sizeof(slist));
}

slist*
slist_pushfront(slist* he, Object* buf, BOOLEAN copy){
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
slist_pushback(slist* he, Object* buf, BOOLEAN copy){
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
			CALL_VOID(run->data, destroy);
			run->data = lm->data;
			LINKED_FREE(lm);
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
	the_list = list_start = slist_pushfront(NULL, new_data, deep_copy);
	runner = runner->next;
	for(; runner != NULL; runner = runner->next){
		Object* new_data = runner->data;
		the_list = slist_pushback(the_list, new_data, deep_copy)->next;
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
		if(destroy_data) CALL_VOID(run->data,destroy);
		p = run->next;
		LINKED_FREE(run);
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
slist_popback(slist *head, Object** data, BOOLEAN destroy_data){
	if(!head) return NULL;
	slist *run = head;
	if(!run->next){
		if(data) *data = run->next->data;
		if(destroy_data) CALL_VOID(run->data, destroy);
		LINKED_FREE(run);
		return NULL;
	}
	for(; run->next && run->next->next; run = run->next);
	if(data) *data = run->next->data;
	if(destroy_data) CALL_VOID(run->next->data,destroy);
	LINKED_FREE(run->next);
	run->next = NULL;
	return head;
}
slist*
slist_popfront(slist *head, Object** data, BOOLEAN destroy_data){
	slist *run = head;
	if(!head) return head;
	run = head;
	head = head->next;
	if(data) *data = run->data;
	if(destroy_data) CALL_VOID(run->data, destroy);
	LINKED_FREE(run);
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
			if(destroy_data) CALL_VOID(run->data,destroy);
			LINKED_FREE(run);
		} else {
			head = run->next;
			if(destroy_data) CALL_VOID(run->data,destroy);
			LINKED_FREE(run);
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
				if(destroy_data) CALL_VOID(run->data,destroy);
				LINKED_FREE(run);
			} else {
				head = run->next;
				if(destroy_data) CALL_VOID(run->data,destroy);
				LINKED_FREE(run);
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
			if(destroy_data) CALL_VOID(run->data,destroy);
			LINKED_FREE(run);
		} else {
			head = run->next;
			if(destroy_data) CALL_VOID(run->data,destroy);
			LINKED_FREE(run);
		}
	}
	return head;
}


//Other Functions
slist*
slist_find(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered){
	if(!head) return NULL;
slist* p = head;
	for(; p->next && (ordered?key_method->compare(key, p->data) > 0:key_method->compare(key, p->data) != 0); p = p->next);
	if(p && key_method->compare(key, p->data) == 0) return p;
	return NULL;
}

BOOLEAN
slist_has(slist *head, slist* node){
	slist *run;
	SLIST_ITERATE(run, head)
		if(run == node) return TRUE;
	return FALSE;
}

slist *
slist_head(slist *head){
	return head;
}

slist *
slist_tail(slist *head){
	slist *run;
	SLIST_ITERATE(run, head)
		if(run->next == NULL) return run;
	return head;
}

slist *
slist_at(slist *head, size_t idx, BOOLEAN back){
	slist *run;
	size_t len = slist_length(head);
	if(back) idx = len-idx;
	if(idx < len){
		size_t i = 0;
		SLIST_ITERATE(run, head){
			if(i == idx) return run;
			i++;
		}
	}
	return head;
}

size_t
slist_loc(slist *head, slist *node){
	slist *run;
	size_t i = 0;
	SLIST_ITERATE(run, head){
		if(run == node) return i;
		i++;
	}
	return -1;
}

slist*
slist_reverse(slist* head){
	slist *next = NULL, *prev = NULL, *run;
	for(run = head; run != NULL; ){
		next = run->next;
		run->next = prev;
		prev = run;
		run = next;
	}
	return prev;
}
