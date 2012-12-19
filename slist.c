#include "linked_structures.h"

slist*
slist_push(slist* he, void* buf, BOOLEAN copy, list_tspec *type){
	slist *lm;
	lm = (slist*)type->adalloc(sizeof(slist));
	if(copy){
		lm->data = type->deep_copy(buf);
	} else {
		lm->data = buf;
	}
	lm->next = NULL;
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
	lm = (slist*)type->adalloc(sizeof(slist));
	if(copy){
		lm->data = type->deep_copy(buf);
	} else {
		lm->data = buf;
	}
	lm->next = NULL;
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
	if(!type->compar) return slist_push(he, buf, deep_copy, type); //add to front if compar is null
	slist *run, *lm, *runp = NULL;

	lm = (slist*)type->adalloc(sizeof(slist));
	if(deep_copy){
		lm->data = type->deep_copy(buf);
	} else {
		lm->data = buf;
	}
	lm->next = NULL;
	if(he == NULL) {
		he = lm;
	} else {
		for(run = he; run && type->compar(buf, run->data) > 0; run = run->next){
			runp = run;
		}
		if(overwrite && run && type->compar(buf, run->data) == 0){
			type->adfree(run->data);
			run->data = lm->data;
			type->adfree(lm);
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
	if(deep_copy) new_data = type->deep_copy(new_data);
	the_list = list_start = slist_push(NULL, new_data, 0, FALSE);
	runner = runner->next;
	for(; runner != NULL; runner = runner->next){
		void* new_data = runner->data;
		if(deep_copy) new_data = type->deep_copy(new_data);
		the_list = slist_append(the_list, new_data, 0, FALSE)->next;
	}
	return list_start;
}


void
slist_clear(slist *head, BOOLEAN free_data, list_tspec* type){
	slist *run = head, *p = head;
	for(; run; run = p){
		if(free_data) type->adfree(run->data);
		p = run->next;
		type->adfree(run);
	}
}

slist*
slist_dequeue(slist *head, void** data, BOOLEAN free_data, list_tspec* type){
	if(!head) return NULL;
	slist *run = head;
	if(!run->next){
		if(data) *data = run->next->data;
		if(free_data) type->adfree(run->data);
		type->adfree(run);
		return NULL;
	}
	for(; run->next && run->next->next; run = run->next);
	if(data) *data = run->next->data;
	if(free_data) type->adfree(run->next->data);
	type->adfree(run->next);
	run->next = NULL;
	return head;
}
slist*
slist_pop(slist *head, void** data, BOOLEAN free_data, list_tspec* type){
	slist *run = head;
	if(!head) return head;
	run = head;
	head = head->next;
	if(data) *data = run->data;
	if(free_data) type->adfree(run->data);
	type->adfree(run);
	return head;
}
slist*
slist_removeViaKey(slist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN free_data, list_tspec* type){
	slist *run = head, *runp = NULL;
	if(head == NULL) {
		return NULL;
	}
	for(run = head; run && (ordered?type->key_compar(key, run->data) <= 0:type->key_compar(key, run->data) != 0); run = run->next){
		runp = run;
	}
	if(run && type->key_compar(key, run->data) == 0){
		if(runp){
			runp->next = run->next;
			if(data) *data = run->data;
			if(free_data) type->adfree(run->data);
			type->adfree(run);
		} else {
			head = run->next;
			if(data) *data = run->data;
			if(free_data) type->adfree(run->data);
			type->adfree(run);
		}
	}
	return head;
}
slist*
slist_removeAllViaKey(slist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN free_data, list_tspec* type){
	slist *run = head, *runp = NULL;
	if(head == NULL) {
		return NULL;
	}
	for(run = head; run; run = run->next){
		if(ordered && type->key_compar(key, run->data) > 0) break;
		if(type->key_compar(key, run->data) == 0){
			if(runp){
				runp->next = run->next;
				if(data) *data = run->data;
				if(free_data) type->adfree(run->data);
				type->adfree(run);
			} else {
				head = run->next;
				if(data) *data = run->data;
				if(free_data) type->adfree(run->data);
				type->adfree(run);
			}
		}
		runp = run;
	}
	return head;
}
slist*
slist_removeElement(slist *head, slist *rem, BOOLEAN free_data, list_tspec* type){
	slist *run = head, *runp = NULL;
	if(head == NULL) {
		return NULL;
	}
	for(run = head; run && run != 0; run = run->next){
		runp = run;
	}
	if(run && rem == run){
		if(runp){
			runp->next = run->next;
			if(free_data) type->adfree(run->data);
			type->adfree(run);
		} else {
			head = run->next;
			if(free_data) type->adfree(run->data);
			type->adfree(run);
		}
	}
	return head;
}


//Other Functions
BOOLEAN
slist_map(slist *head, void* aux, lMapFunc func){
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
	for(; p->next && p->data; p = p->next, len++);
	void **values = (void**)type->adalloc((len+1)*sizeof(void*));
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
	for(; p && p->data; p = p->next, i++);
	len = i;
	void **values = (void**)type->adalloc((len+1)*sizeof(void*));
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
	for(; p->next && (ordered?type->key_compar(key, p->data) > 0:type->key_compar(key, p->data) != 0); p = p->next);
	if(p && type->compar(key, p->data) == 0) return p->data;
	return NULL;
}
