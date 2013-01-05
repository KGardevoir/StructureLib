#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

static long
memcomp(void *a, void* b){ return (a>b?1:(a<b?-1:0)); }

static dlist*
new_dlist(void* data, BOOLEAN deep_copy, dlist* prev, dlist* next, list_tspec* type){
	deep_copy = deep_copy && type && type->deep_copy;
	dlist *new = MALLOC(sizeof(dlist));
	dlist init = {
		.next = (next==NULL&&prev==NULL)?new:next,
		.prev = (next==NULL&&prev==NULL)?new:prev,
		.data = deep_copy?type->deep_copy(data):data
	};
	memcpy(new, &init, sizeof(*new));
	return new;
}

dlist*
dlist_push(dlist* he, void* buf, BOOLEAN deep_copy, list_tspec* type){
	return dlist_append(he, buf, deep_copy, type)->prev;
}

dlist*
dlist_append(dlist* he, void* buf, BOOLEAN deep_copy, list_tspec* type){
	dlist *lm;
	lm = new_dlist(buf, deep_copy, NULL, NULL, type);
	if(he == NULL){
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
	dlist *lm, *run;
	lm = new_dlist(buf, deep_copy, NULL, NULL, type);
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	if(he == NULL){
		he = lm;
	} else {
		run = he; do{ if(compar(run->data, buf) >= 0){ break; } run = run->next; } while(run != he);
		//unified cases, since we enforce a FILO-like queue if they
		//are equal
		lm->next = run;
		lm->prev = run->prev;
		run->prev->next = lm;
		run->prev = lm;
		if(he == run && compar(run->data, buf) >= 0) he = lm;
	}
	return he;
}

dlist*
dlist_copy(dlist* src, BOOLEAN deep_copy, list_tspec* type){
	dlist* mnew = NULL, *runner = src;
	do{
		void* new_data = runner->data;
		mnew = dlist_push(mnew, new_data, deep_copy, type);
		runner = runner->next;
	} while(runner != src);
	return mnew;
}

void
dlist_clear(dlist *head, BOOLEAN destroy_data, list_tspec* type){
	dlist *run = head, *n = head;
	if(head == NULL) return;
	destroy_data = destroy_data && type && type->destroy;
	do{
		if(destroy_data) type->destroy(run->data);
		n = run->next;
		FREE(run);
		run = n;
	} while(run != head);
}

size_t
dlist_length(dlist *head){
	if(head == NULL) return 0;
	dlist *run = head;
	size_t i = 0;
	do {
		i++;
		run = run->next;
	} while(run != head);
	return i;
}

dlist*
dlist_dequeue(dlist *head, void** data, BOOLEAN destroy_data, list_tspec* type){
	if(!head) return NULL;
	destroy_data = destroy_data && type && type->destroy;
	if(head->next == head){
		if(data) *data = head->data;
		if(destroy_data) type->destroy(head->data);
		FREE(head);
		head = NULL;
	} else {
		dlist* tmp = head;
		head = head->next;
		tmp->next->prev = tmp->prev;
		tmp->prev->next = tmp->next;
		if(data) *data = tmp->data;
		if(destroy_data) type->destroy(head->data);
		FREE(tmp);
	}
	return head;
}

dlist*
dlist_pop(dlist *head, void** dat, BOOLEAN destroy_data, list_tspec* type){
	return dlist_dequeue(head->prev, dat, destroy_data, type);
}

dlist*
dlist_removeViaKey(dlist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec* type){
	dlist *run = head;
	if(!head) return head;
	lKeyCompare key_compar = (type && type->key_compar)?type->key_compar:(lKeyCompare)memcomp;
	do {
		if(ordered?key_compar(key, run->data) >= 0:key_compar(key, run->data) == 0) break;
		run = run->next;
	} while(run != head);
	if(key_compar(key, run->data) == 0){
		head = dlist_removeElement(head, run, destroy_data, type);
	}
	return head;
}

dlist*
dlist_removeViaAllKey(dlist *head, void** data, void* key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec* type){
	dlist *run = head;
	if(!head) return head;
	lKeyCompare key_compar = (type && type->key_compar)?type->key_compar:(lKeyCompare)memcomp;
	do{
		if(ordered && key_compar(key, run->data) > 0) break;
		if(key_compar(key, run->data) == 0)
			head = dlist_removeElement(head, run, destroy_data, type);
		run = run->next;
	} while(run != head);
	return head;
}

dlist*
dlist_removeElement(dlist *head, dlist *rem, BOOLEAN destroy_data, list_tspec* type){
	if(head == rem) head = dlist_dequeue(head, NULL, destroy_data, type);
	else head = dlist_dequeue(rem, NULL, destroy_data, type);
	return head;
}

static inline BOOLEAN
dlist_map_internal(dlist *head, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	dlist *run = head;
	size_t depth = 0, length = 0;
	if(more_info) length = dlist_length(head);
	if(!func) return FALSE;
	if(!head) return TRUE;
	do{
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.position = depth,
				.depth = depth,
				.size = length,
				.aux = aux
			};
			if(!func(pass_data?run->data:run, (void*)&ax)) return FALSE;
		} else {
			if(!func(pass_data?run->data:run, aux)) return FALSE;
		}
		run = run->next;
		depth++;
	} while(run != head);
	return TRUE;
}

//Other Functions
BOOLEAN
dlist_map(dlist *head, BOOLEAN more_info, void* aux, lMapFunc func){
	return dlist_map_internal(head, TRUE, more_info, aux, func);
}

struct dlist_filter_d {
	void *aux;
	BOOLEAN deep;
	list_tspec *type;
	lMapFunc func;
	dlist *list;
};

static BOOLEAN
dlist_filter_f(void *data, struct dlist_filter_d *aux){
	if(aux->func(data, aux->aux)){
		aux->list = dlist_append(aux->list, data, aux->deep, aux->type);
	}
	return TRUE;
}

dlist*
dlist_filter(dlist *head, void* aux, lMapFunc func, BOOLEAN deep, list_tspec* type){
	struct dlist_filter_d dd = {
		.aux = aux,
		.deep = deep,
		.type = type,
		.func = func,
		.list = NULL
	};
	dlist_map(head, FALSE, &dd, (lMapFunc)dlist_filter_f);
	return dd.list;
}

struct dlist_transform_d {
	void* aux;
	lTransFunc func;
};

BOOLEAN
dlist_transform_f(dlist* node, struct dlist_transform_d* aux){
	return aux->func(&node->data, aux->aux);
}

/**
 * Transform data
 */
dlist*
dlist_transform(dlist *head, void* aux, lTransFunc func){
	struct dlist_transform_d dd = {
		.aux = aux,
		.func = func
	};
	dlist_map_internal(head, FALSE, FALSE, &dd, (lMapFunc)dlist_transform_f);
	return head;
}

void**
dlist_toArray(dlist *head, BOOLEAN deep_copy, list_tspec* type){
size_t len = 0;
dlist* p = head;
	if(!head) return NULL;
	deep_copy = deep_copy && type && type->deep_copy;
	do{ p = p->next; len++; } while(p != head);
	void **values = (void**)MALLOC((len+1)*sizeof(void*));
	len = 0; p = head;
	do{
		if(deep_copy){
			values[len] = type->deep_copy(p->data);
		} else {
			values[len] = p->data;
		}
		p = p->next; len++;
	} while(p != head);
	values[len] = NULL;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}
void**
dlist_toArrayReverse(dlist *head, BOOLEAN deep_copy, list_tspec* type){
size_t i = 0, len = 0;
dlist* p = head;
	if(!head) return NULL;
	deep_copy = deep_copy && type && type->deep_copy;
	do{ p = p->next; i++; } while(p != head);
	void **values = (void**)MALLOC((len+1)*sizeof(void*));
	len = 0; p = head;
	do{
		if(deep_copy){
			values[len] = type->deep_copy(p->data);
		} else {
			values[len] = p->data;
		}
		p = p->prev; len++;
	} while(p != head);
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
	lCompare compar = (type && type->compar)?type->compar:(lCompare)memcomp;
	if(compar(src->data, dst->data) < 0){ //always merge into list with smaller head
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
			} while(rundst != dst && compar(rundst->data, runsrc->data) < 0); //find start
			if(rundst == dst && compar(rundst->data, runsrc->data) != 0) l1_done = TRUE;
		}
		//ldp("---------------\nInterstate 1", dst, rundst);
		if(!l2_done){
			do {
				runsrc_end = runsrc_end->next;
			} while(runsrc_end != runsrc && compar(rundst->data, runsrc_end->data) >= 0);
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
	if(dsthead == NULL) return srchead;//handle empty lists
	if(srchead == NULL) return dsthead;
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
	lKeyCompare key_compar = (type && type->key_compar)?type->key_compar:(lKeyCompare)memcomp;
	dlist* p = head;
	do {
		if(ordered?(key_compar(key,p->data) <= 0):(key_compar(key,p->data) == 0)) break;
		p = p->next;
	} while(p != head);
	if(key_compar(key, p->data) == 0) return p;
	return NULL;
}

BOOLEAN
dlist_has(dlist *head, dlist *node){
	if(!head) return FALSE;
	dlist *p = head;
	do{
		if(p == node) return TRUE;
		p = p->next;
	} while(p != head);
	return FALSE;
}
