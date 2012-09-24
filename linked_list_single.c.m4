include(linked_structures.h.name.m4)dnl
-{#include}- "H_NAME"

void
SLL_PREFIX-{}-print(SLL_NAME* head, TSPEC_NAME* type){
	SLL_NAME *run;
	size_t i = 0;
	if(!head){
		printf("Empty List\n");
		return;
	}
	for(run = head; run; run = run->next, i++){
		char *t;
		printf("list[%u]: %s\n", (unsigned int)i, t = type->strfunc(run->data));
		type->adfree(t);
	}
}

SLL_NAME*
SLL_PREFIX-{}-push(SLL_NAME* he, void* buf, BOOLEAN copy, TSPEC_NAME *type){
	SLL_NAME *lm;
	lm = (SLL_NAME*)type->adalloc(sizeof(SLL_NAME));
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
SLL_NAME*
SLL_PREFIX-{}-append(SLL_NAME* he, void* buf, BOOLEAN copy, TSPEC_NAME* type){
	SLL_NAME *lm, *he2;
	lm = (SLL_NAME*)type->adalloc(sizeof(SLL_NAME));
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
SLL_NAME*
SLL_PREFIX-{}-addOrdered(SLL_NAME* he, void* buf, BOOLEAN deep_copy, BOOLEAN overwrite, TSPEC_NAME* type){
	if(!type->compar) return SLL_PREFIX-{}-push(he, buf, deep_copy, type); //add to front if compar is null
	SLL_NAME *run, *lm, *runp = NULL;

	lm = (SLL_NAME*)type->adalloc(sizeof(SLL_NAME));
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

SLL_NAME*
SLL_PREFIX-{}-copy(SLL_NAME* src, BOOLEAN deep_copy, TSPEC_NAME* type){
	SLL_NAME *the_list, *list_start, *runner = src;
	void* new_data = runner->data;
	if(deep_copy) new_data = type->deep_copy(new_data);
	the_list = list_start = SLL_PREFIX-{}-push(NULL, new_data, 0, FALSE);
	runner = runner->next;
	for(; runner != NULL; runner = runner->next){
		void* new_data = runner->data;
		if(deep_copy) new_data = type->deep_copy(new_data);
		the_list = SLL_PREFIX-{}-append(the_list, new_data, 0, FALSE)->next;
	}
	return list_start;
}


void
SLL_PREFIX-{}-clear(SLL_NAME *head, BOOLEAN free_data, TSPEC_NAME* type){
	SLL_NAME *run = head, *p = head;
	for(; run; run = p){
		if(free_data) type->adfree(run->data);
		p = run->next;
		type->adfree(run);
	}
}

SLL_NAME*
SLL_PREFIX-{}-dequeue(SLL_NAME *head, void** data, BOOLEAN free_data, TSPEC_NAME* type){
	if(!head) return NULL;
	SLL_NAME *run = head, *p = head;
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
SLL_NAME*
SLL_PREFIX-{}-pop(SLL_NAME *head, void** data, BOOLEAN free_data, TSPEC_NAME* type){
	SLL_NAME *run = head;
	if(!head) return head;
	run = head;
	head = head->next;
	if(data) *data = run->data;
	if(free_data) type->adfree(run->data);
	type->adfree(run);
	return head;
}
SLL_NAME*
SLL_PREFIX-{}-removeViaKey(SLL_NAME *head, void** data, void* key, BOOLEAN ordered, BOOLEAN free_data, TSPEC_NAME* type){
	SLL_NAME *run = head, *runp = NULL;
	if(head == NULL) {
		return NULL;
	}
	for(run = head; run && (ordered?type->key_compar(key, run->data) > 0:type->key_compar(key, run->data) != 0); run = run->next){
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


//Other Functions
#if 0
#error Unimplemented
SLL_NAME*
SLL_PREFIX-{}-map(SLL_NAME *head, SLL_NAME *(func)(void*,SLL_NAME*)){}
#endif

ifdef(-{SLL_TRANSFORMS}-,-{
void**
SLL_PREFIX-{}-toArray(SLL_NAME *head, BOOLEAN deep, TSPEC_NAME* type){
size_t len = 0;
SLL_NAME* p = head;
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
SLL_PREFIX-{}-toArrayReverse(SLL_NAME *head, BOOLEAN deep, TSPEC_NAME* type){
size_t i = 0, len = 0;
SLL_NAME* p = head;
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

ifdef(-{DCLL}-,-{dnl
DCLL_NAME* 
SLL_NAME-{}-_to_-{}-DCLL_NAME-{}-(SLL_NAME *head, BOOLEAN deep, TSPEC_NAME* type){
	SLL_NAME* p = head;
	DCLL_NAME* d = NULL;
	for(; p; p = p->next){
		d = DCLL_PREFIX-{}-append(d, NULL, p->data, deep, type);
	}
	return d;
}
}-)dnl
//End Single Linked Transforms
}-)dnl

ifdef(-{SLL_MERGE}-,-{
#error Single Linked Merging functions are unimplemented
void
ldp(char* t, SLL_NAME* l, SLL_NAME* n){
	SLL_NAME *r = l;
	if(r == n) printf("%s: |%ld|", t, r->data);
	else printf("%s: %ld", t, r->data);
	r = r->next;
	while(r != l){
		if(r == n) printf(", |%ld|", r->data);
		else printf(", %ld", r->data);
		r = r->next;
	}
	printf("\n");
}

SLL_NAME*
SLL_PREFIX-{}-merge(SLL_NAME* dst, SLL_NAME* src, TSPEC_NAME* type){
	if(type->compar){//merge sorted, assume both lists are already sorted
		if(type->compar(src->data, dst->data) < 0){ //always merge into list with smaller head
			SLL_NAME *tmp = dst;
			dst = src;
			src = tmp;
		}
		//puts("================");
		//ldp("Merge Out", dst, NULL);
		//ldp("Merge In", src, NULL);
		SLL_NAME *rundst = dst, *runsrc = src;
		BOOLEAN l1_done = FALSE, l2_done = FALSE;
		do{
			SLL_NAME *runsrc_end = runsrc;
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
				SLL_PREFIX-{}-split(runsrc, runsrc_end);
			}
			//ldp("Merging", runsrc, NULL);
			SLL_PREFIX-{}-concat(rundst, runsrc);
			//ldp("Merged", dst, rundst);
			runsrc = runsrc_end;
		} while(!l2_done); //Theta(n) time
		//ldp("Done", dst, NULL);
		//printf("List Mostly Merged...\n");
		return dst;
	} else {
		return SLL_PREFIX-{}-concat(dst, src);
	}
}

SLL_NAME*
SLL_PREFIX-{}-sort(SLL_NAME* head, TSPEC_NAME* type){//this can be faster, using "natural" mergesort
	SLL_NAME *left = head, *right = head->next;
	if(left != right){
		/*do{
			if(type->compar(right->data, right->prev->data) < 0) break;
			right = right->next;
		} while(right != head); //find runs which are already sorted
		*/
		SLL_NAME *tortise = head, *hare = head;
		do{
			tortise = tortise->next;
			hare = hare->next->next;
		} while(hare->prev != head && hare != head); //find middle (either we jump over it or land on it)
		right = tortise;
		//ldp("->Middle", head, right);
		SLL_PREFIX-{}-split(left, right);
		left = SLL_PREFIX-{}-sort(left, type);
		right = SLL_PREFIX-{}-sort(right, type);
		return SLL_PREFIX-{}-merge(left, right, type);
	}
	return head;
}

SLL_NAME*
SLL_PREFIX-{}-split(SLL_NAME* h1, SLL_NAME* h2){
	return h2;
}

SLL_NAME*
SLL_PREFIX-{}-concat(SLL_NAME *dst, SLL_NAME* src){
	SLL_NAME *t = dst;
	for(;t->next != NULL; t = t->next);
	t->next = src;
	return src;
}
//End Single Linked Merging
}-)dnl

void*
SLL_PREFIX-{}-find(SLL_NAME *head, void* key, BOOLEAN ordered, TSPEC_NAME *type){
	if(!head) return NULL;
SLL_NAME* p = head;
	for(; p->next && (ordered?type->key_compar(key, p->data) < 0:type->key_compar(key, p->data) != 0); p = p->next);
	if(p && type->compar(key, p->data) == 0) return p->data;
	return NULL;
}
