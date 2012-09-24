include(linked_structures.h.name.m4)dnl
-{#include}- "H_NAME"

void
DCLL_PREFIX-{}-print(DCLL_NAME* head, TSPEC_NAME* type){
	DCLL_NAME *run = head;
	size_t i = 0;
	do{
		char* t;
		printf("list[%u]: %s\n", (unsigned int)i, t = type->strfunc(run->data));
		type->adfree(t);
		run = run->next;
		i++;
	} while(run != head);
}

DCLL_NAME*
DCLL_PREFIX-{}-push(DCLL_NAME* he, DCLL_NAME** addedTo, void* buf, BOOLEAN deep_copy, TSPEC_NAME* type){
	return DCLL_PREFIX-{}-append(he, addedTo, buf, deep_copy, type)->prev;
}

DCLL_NAME*
DCLL_PREFIX-{}-append(DCLL_NAME* he, DCLL_NAME** addedTo, void* buf, BOOLEAN deep_copy, TSPEC_NAME* type){
	DCLL_NAME *lm;
	lm = (DCLL_NAME*)type->adalloc(sizeof(DCLL_NAME));
	if(deep_copy){
		lm->data = type->deep_copy(buf);
	} else {
		lm->data = buf;
	}
	if(addedTo) *addedTo = lm;
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

DCLL_NAME*
DCLL_PREFIX-{}-addOrdered(DCLL_NAME* he, DCLL_NAME **addedTo, void* buf, BOOLEAN deep_copy, BOOLEAN overwrite, TSPEC_NAME *type){
	if(!type->compar) return DCLL_PREFIX-{}-append(he, addedTo, buf, deep_copy, type);
	DCLL_NAME *lm, *run;
	lm = (DCLL_NAME*)type->adalloc(sizeof(DCLL_NAME));
	if(deep_copy){
		lm->data = type->deep_copy(buf);
	} else {
		lm->data = buf;
	}
	if(addedTo) *addedTo = lm;
	if(he == NULL){
		lm->next = lm->prev = lm;
		he = lm;
	} else {
		run = he; do{ if(type->compar(run->data, buf) >= 0){ break; } run = run->next; } while(run != he);
		if(overwrite && type->compar(buf, run->data) == 0){
			type->adfree(run->data);
			run->data = lm->data;
			type->adfree(lm);
		} else {
			//unified cases, since we enforce a FILO-like queue if they
			//are equal
			lm->next = run;
			lm->prev = run->prev;
			run->prev->next = lm;
			run->prev = lm;
			if(he == run && type->compar(run->data, buf) >= 0) he = lm;
		}
	}
	return he;
}

DCLL_NAME*
DCLL_PREFIX-{}-copy(DCLL_NAME* src, BOOLEAN deep_copy, TSPEC_NAME* type){
	DCLL_NAME* mnew, *runner = src;
	do{
		void* new_data = runner->data;
		if(deep_copy) new_data = type->deep_copy(new_data);
		mnew = DCLL_PREFIX-{}-push(mnew, NULL, new_data, 0, FALSE);
		runner = runner->next;
	} while(runner != src);
	return mnew;
}

void
DCLL_PREFIX-{}-clear(DCLL_NAME *head, BOOLEAN free_data, TSPEC_NAME* type){
	DCLL_NAME *run = head, *n = head;
	if(head == NULL) return;
	do{
		if(free_data) type->adfree(run->data);
		n = run->next;
		type->adfree(run);
		run = n;
	} while(run != head);
}

DCLL_NAME*
DCLL_PREFIX-{}-dequeue(DCLL_NAME *head, void** data, BOOLEAN free_data, TSPEC_NAME* type){
	if(!head) return NULL;
	if(head->next == head){
		if(data) *data = head->data;
		if(free_data) type->adfree(head->data);
		head = NULL;
	} else {
		head->next->prev = head->prev;
		head->prev->next = head->next;
		DCLL_NAME *tmp = head;
		head = head->next;
		if(data) *data = head->data;
		if(free_data) type->adfree(head->data);
	}
	return head;
}

DCLL_NAME*
DCLL_PREFIX-{}-pop(DCLL_NAME *head, void** dat, BOOLEAN free_data, TSPEC_NAME* type){
	return DCLL_PREFIX-{}-dequeue(head->prev, dat, free_data, type);
}

DCLL_NAME*
DCLL_PREFIX-{}-removeViaKey(DCLL_NAME *head, void** data, void* key, BOOLEAN ordered, BOOLEAN free_data, TSPEC_NAME* type){
	DCLL_NAME *run = head;
	if(head == NULL){
		return NULL;
	}
	do{ if(ordered?type->key_compar(key, run->data) <= 0:type->key_compar(key, run->data) == 0) break; run = run->next; } while(run != head);
	if(type->key_compar(key, run->data) == 0){
		head = DCLL_PREFIX-{}-removeElement(head, run, free_data, type);
	}
	return head;
}

DCLL_NAME*
DCLL_PREFIX-{}-removeElement(DCLL_NAME *head, DCLL_NAME *rem, BOOLEAN free_data, TSPEC_NAME* type){
	if(head == rem) head = DCLL_PREFIX-{}-dequeue(head, NULL, free_data, type);
	else DCLL_PREFIX-{}-dequeue(rem, NULL, free_data, type);
	return head;
}

//Other Functions
DCLL_NAME*
DCLL_PREFIX-{}-map(DCLL_NAME *head, DCLL_NAME* (func)(void*,DCLL_NAME*)){
	DCLL_NAME *run = head, *tmp = NULL;
	do{
		tmp = func(run->data, tmp);
		run = run->next;
	} while(run != head);
}

ifdef(-{DCLL_TRANSFORMS}-,-{
void**
DCLL_PREFIX-{}-toArray(DCLL_NAME *head, BOOLEAN deep_copy, TSPEC_NAME* type){
size_t len = 0;
DCLL_NAME* p = head;
	if(!head) return NULL;
	do{ p = p->next; len++; } while(p != head);
	void **values = (void**)type->adalloc((len+1)*sizeof(void*));
	len = 0; p = head; do{ if(deep_copy){ values[len] = type->deep_copy(p->data); } else { values[len] = p->data; } p = p->next; len++;} while(p != head);
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}
void**
DCLL_PREFIX-{}-toArrayReverse(DCLL_NAME *head, BOOLEAN deep_copy, TSPEC_NAME* type){
size_t i = 0, len = 0;
DCLL_NAME* p = head;
	if(!head) return NULL;
	do{ p = p->next; i++; } while(p != head);
	void **values = (void**)type->adalloc((len+1)*sizeof(void*));
	len = 0; p = head; do{ if(deep_copy){ values[len] = type->deep_copy(p->data); } else { values[len] = p->data; } p = p->prev; len++;} while(p != head);
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}
ifdef(-{SLL}-,-{dnl
SLL_NAME*
DCLL_NAME-{}-_to_-{}-SLL_NAME-{}-(DCLL_NAME *head, BOOLEAN deep_copy, TSPEC_NAME* type){
	SLL_NAME* p = NULL;
	DCLL_NAME* d = head;
	if(!d) return NULL;
	do{ p = SLL_PREFIX-{}-push(p, d->data, deep_copy, type); d = d->prev; } while(d != head);
	return p;
}}-)
}-)dnl

ifdef(-{DCLL_MERGE}-,-{
#if 0 //debug
void
ldp(char* t, DCLL_NAME* l, DCLL_NAME* n){
	DCLL_NAME *r = l;
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

DCLL_NAME*
DCLL_PREFIX-{}-merge(DCLL_NAME* dst, DCLL_NAME* src, TSPEC_NAME *type){
	if(type->compar){//merge sorted, assume both lists are already sorted
		if(type->compar(src->data, dst->data) < 0){ //always merge into list with smaller head
			DCLL_NAME *tmp = dst;
			dst = src;
			src = tmp;
		}
		//puts("================");
		//ldp("Merge Out", dst, NULL);
		//ldp("Merge In", src, NULL);
		DCLL_NAME *rundst = dst, *runsrc = src;
		BOOLEAN l1_done = FALSE, l2_done = FALSE;
		do{
			DCLL_NAME *runsrc_end = runsrc;
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
				DCLL_PREFIX-{}-split(runsrc, runsrc_end);
			}
			//ldp("Merging", runsrc, NULL);
			DCLL_PREFIX-{}-concat(rundst, runsrc);
			//ldp("Merged", dst, rundst);
			runsrc = runsrc_end;
		} while(!l2_done); //Theta(n) time
		//ldp("Done", dst, NULL);
		//printf("List Mostly Merged...\n");
		return dst;
	} else {
		return DCLL_PREFIX-{}-concat(dst, src);
	}
}

DCLL_NAME*
DCLL_PREFIX-{}-sort(DCLL_NAME* head, TSPEC_NAME* type){//this can be faster, using "natural" mergesort
	DCLL_NAME *left = head, *right = head->next;
	if(left != right){
		/*do{
			if(type->compar(right->data, right->prev->data) < 0) break;
			right = right->next;
		} while(right != head); //find runs which are already sorted
		*/
		DCLL_NAME *tortise = head, *hare = head;
		do{
			tortise = tortise->next;
			hare = hare->next->next;
		} while(hare->prev != head && hare != head); //find middle (either we jump over it or land on it)
		right = tortise;
		//ldp("->Middle", head, right);
		DCLL_PREFIX-{}-split(left, right);
		left = DCLL_PREFIX-{}-sort(left, type);
		right = DCLL_PREFIX-{}-sort(right, type);
		return DCLL_PREFIX-{}-merge(left, right, type);
	}
	return head;
}

DCLL_NAME*
DCLL_PREFIX-{}-split(DCLL_NAME* h1, DCLL_NAME* h2){
	DCLL_PREFIX-{}-concat(h1, h2);//turns out Concatonation and Splitting are synonymous ops
	//ldp("List 1", h1, NULL);
	//ldp("List 2", h2, NULL);
	return h2;
}

DCLL_NAME* //umad?
DCLL_PREFIX-{}-concat(DCLL_NAME* dsthead, DCLL_NAME* srchead){
	dsthead->prev->next = srchead;
	srchead->prev->next = dsthead;
	DCLL_NAME *tmp = srchead->prev; //now swap
	srchead->prev = dsthead->prev;
	dsthead->prev = tmp;
	return dsthead;
}
//End Merging
}-)dnl

DCLL_NAME*
DCLL_PREFIX-{}-find(DCLL_NAME *head, void* key, BOOLEAN ordered, TSPEC_NAME* type){
	if(!head) return NULL;
	DCLL_NAME* p = head;
	do { if(ordered?type->key_compar(key,p->data) <= 0:type->key_compar(key,p->data) == 0) break; p = p->next; } while(p != head);
	if(type->key_compar(key, p->data) == 0) return p;
	return NULL;
}
