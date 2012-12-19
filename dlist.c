#include "linked_structures.h"

dlist*
dlist_push(dlist* he, void* buf, BOOLEAN deep_copy, list_tspec* type){
	return dlist_append(he, buf, deep_copy, type)->prev;
}

dlist*
dlist_append(dlist* he, void* buf, BOOLEAN deep_copy, list_tspec* type){
	dlist *lm;
	lm = (dlist*)type->adalloc(sizeof(dlist));
	if(deep_copy){
		lm->data = type->deep_copy(buf);
	} else {
		lm->data = buf;
	}
	if(he == NULL){
		lm->next = lm->prev = lm;
		he = lm;
	} else {
		lm->next = he;
		lm->prev = he->prev;
		he->prev->next = lm;
		he->prev = lm;
	}
	return he;
}

dlist*
dlist_addOrdered(dlist* he, void* buf, BOOLEAN deep_copy, list_tspec *type){
	if(!type->compar) return dlist_append(he, buf, deep_copy, type);
	dlist *lm, *run;
	lm = (dlist*)type->adalloc(sizeof(dlist));
	if(deep_copy){
		lm->data = type->deep_copy(buf);
	} else {
		lm->data = buf;
	}
	if(he == NULL){
		lm->next = lm->prev = lm;
		he = lm;
	} else {
		run = he; do{ if(type->compar(run->data, buf) >= 0){ break; } run = run->next; } while(run != he);
		//unified cases, since we enforce a FILO-like queue if they
		//are equal
		lm->next = run;
		lm->prev = run->prev;
		run->prev->next = lm;
		run->prev = lm;
		if(he == run && type->compar(run->data, buf) >= 0) he = lm;
	}
	return he;
}

dlist*
dlist_copy(dlist* src, BOOLEAN deep_copy, list_tspec* type){
	dlist* mnew = NULL, *runner = src;
	do{
		void* new_data = runner->data;
		if(deep_copy) new_data = type->deep_copy(new_data);
		mnew = dlist_push(mnew, new_data, 0, FALSE);
		runner = runner->next;
	} while(runner != src);
	return mnew;
}

void
dlist_clear(dlist *head, BOOLEAN free_data, list_tspec* type){
	dlist *run = head, *n = head;
	if(head == NULL) return;
	do{
		if(free_data) type->adfree(run->data);
		n = run->next;
		type->adfree(run);
		run = n;
	} while(run != head);
}

dlist*
dlist_dequeue(dlist *head, void** data, BOOLEAN free_data, list_tspec* type){
	if(!head) return NULL;
	if(head->next == head){
		if(data) *data = head->data;
		if(free_data) type->adfree(head->data);
		head = NULL;
	} else {
		head->next->prev = head->prev;
		head->prev->next = head->next;
		head = head->next;
		if(data) *data = head->data;
		if(free_data) type->adfree(head->data);
	}
	return head;
}

dlist*
dlist_pop(dlist *head, void** dat, BOOLEAN free_data, list_tspec* type){
	return dlist_dequeue(head->prev, dat, free_data, type);
}

dlist*
dlist_removeViaKey(dlist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN free_data, list_tspec* type){
	dlist *run = head;
	if(head == NULL){
		return NULL;
	}
	do{ if(ordered?type->key_compar(key, run->data) >= 0:type->key_compar(key, run->data) == 0) break; run = run->next; } while(run != head);
	if(type->key_compar(key, run->data) == 0){
		head = dlist_removeElement(head, run, free_data, type);
	}
	return head;
}

dlist*
dlist_removeViaAllKey(dlist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN free_data, list_tspec* type){
	dlist *run = head;
	if(head == NULL){
		return NULL;
	}
	do{
		if(ordered && type->key_compar(key, run->data) > 0) break;
		if(type->key_compar(key, run->data) == 0)
			head = dlist_removeElement(head, run, free_data, type);
		run = run->next;
	} while(run != head);
	return head;
}

dlist*
dlist_removeElement(dlist *head, dlist *rem, BOOLEAN free_data, list_tspec* type){
	if(head == rem) head = dlist_dequeue(head, NULL, free_data, type);
	else head = dlist_dequeue(rem, NULL, free_data, type);
	return head;
}

//Other Functions
BOOLEAN
dlist_map(dlist *head, void* aux, lMapFunc func){
	dlist *run = head;
	do{
		if(!func(run->data, aux)) return FALSE;
		run = run->next;
	} while(run != head);
	return TRUE;
}


void**
dlist_toArray(dlist *head, BOOLEAN deep_copy, list_tspec* type){
size_t len = 0;
dlist* p = head;
	if(!head) return NULL;
	do{ p = p->next; len++; } while(p != head);
	void **values = (void**)type->adalloc((len+1)*sizeof(void*));
	len = 0; p = head; do{ if(deep_copy){ values[len] = type->deep_copy(p->data); } else { values[len] = p->data; } p = p->next; len++;} while(p != head);
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}
void**
dlist_toArrayReverse(dlist *head, BOOLEAN deep_copy, list_tspec* type){
size_t i = 0, len = 0;
dlist* p = head;
	if(!head) return NULL;
	do{ p = p->next; i++; } while(p != head);
	void **values = (void**)type->adalloc((len+1)*sizeof(void*));
	len = 0; p = head; do{ if(deep_copy){ values[len] = type->deep_copy(p->data); } else { values[len] = p->data; } p = p->prev; len++;} while(p != head);
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}
slist*
dlist_to_slist(dlist *head, BOOLEAN deep_copy, list_tspec* type){
	slist* p = NULL;
	dlist* d = head;
	if(!d) return NULL;
	do{ p = slist_push(p, d->data, deep_copy, type); d = d->prev; } while(d != head);
	return p;
}


#if 0 //debug
void
ldp(char* t, dlist* l, dlist* n){
	dlist *r = l;
	if(r == n) printf("%s: |%ld|", t, r->data);
	else printf("%s: %ld", t, r->data);
	r = r->next;
	while(r != l){
		if(r == n) printf(", |%ld|", r->data);
		else printf(", %ld", r->data);
		//verify links
		if(r->prev->next != r || r->next->prev != r){
			printf("BAD LINK <%p, %p, %p>\n", r->prev, r, r->next);
			break;
		}
		r = r->next;
	}
	printf("\n");
}
#endif

dlist*
dlist_merge(dlist* dst, dlist* src, list_tspec *type){
	if(type->compar){//merge sorted, assume both lists are already sorted
		if(type->compar(src->data, dst->data) < 0){ //always merge into list with smaller head
			dlist *tmp = dst;
			dst = src;
			src = tmp;
		}
		//puts("================");
		//ldp("Merge Out", dst, NULL);
		//ldp("Merge In", src, NULL);
		dlist *rundst = dst, *runsrc = src;
		BOOLEAN l1_done = FALSE, l2_done = FALSE;
		do{
			dlist *runsrc_end = runsrc;
			if(!l1_done){//never iterate through once we have gone entirely through the list
				do{
					rundst = rundst->next;
				} while(rundst != dst && type->compar(rundst->data, runsrc->data) < 0); //find start
				if(rundst == dst && type->compar(rundst->data, runsrc->data) != 0) l1_done = TRUE;
			}
			//ldp("---------------\nInterstate 1", dst, rundst);
			if(!l2_done){
				do {
					runsrc_end = runsrc_end->next;
				} while(runsrc_end != runsrc && type->compar(rundst->data, runsrc_end->data) >= 0);
				if(runsrc_end == runsrc) l2_done = TRUE;
				//ldp("Interstate 2", runsrc, runsrc_end);
				(void)dlist_split(runsrc, runsrc_end);
			}
			//ldp("Merging", runsrc, NULL);
			(void)dlist_concat(rundst, runsrc);
			//ldp("Merged", dst, rundst);
			runsrc = runsrc_end;
		} while(!l2_done); //Theta(n) time
		//ldp("Done", dst, NULL);
		//printf("List Mostly Merged...\n");
		return dst;
	} else {
		return dlist_concat(dst, src);
	}
}

dlist*
dlist_sort(dlist* head, list_tspec* type){//this can be faster, using "natural" mergesort
	dlist *left = head, *right = head->next;
	if(left != right){
		/*do{
			if(type->compar(right->data, right->prev->data) < 0) break;
			right = right->next;
		} while(right != head); //find runs which are already sorted
		*/
		dlist *tortise = head, *hare = head;
		do{
			tortise = tortise->next;
			hare = hare->next->next;
		} while(hare->prev != head && hare != head); //find middle (either we jump over it or land on it)
		right = tortise;
		//ldp("->Middle", head, right);
		(void)dlist_split(left, right);//suppress warning
		left = dlist_sort(left, type);
		right = dlist_sort(right, type);
		return dlist_merge(left, right, type);
	}
	return head;
}

dlist*
dlist_split(dlist* h1, dlist* h2){
	h2 = dlist_concat(h1, h2);//turns out Concatonation and Splitting are synonymous ops
	//ldp("List 1", h1, NULL);
	//ldp("List 2", h2, NULL);
	return h2;
}

dlist* //umad?
dlist_concat(dlist* dsthead, dlist* srchead){
	dsthead->prev->next = srchead;
	srchead->prev->next = dsthead;
	dlist *tmp = srchead->prev; //now swap
	srchead->prev = dsthead->prev;
	dsthead->prev = tmp;
	return dsthead;
}
//End Merging

dlist*
dlist_find(dlist *head, const void* key, BOOLEAN ordered, list_tspec* type){
	if(!head) return NULL;
	dlist* p = head;
	do {
		if(ordered?(type->key_compar(key,p->data) <= 0):(type->key_compar(key,p->data) == 0)) break;
		p = p->next;
	} while(p != head);
	if(type->key_compar(key, p->data) == 0) return p;
	return NULL;
}
