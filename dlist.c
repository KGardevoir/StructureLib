#include "dlist.h"
#include "assert.h"

static inline dlist*
new_dlist(Object* data, BOOLEAN deep_copy, dlist* prev, dlist* next){
	dlist *lnew = (dlist*)MALLOC(sizeof(dlist));
	dlist init = {
		.next = (next==NULL&&prev==NULL)?lnew:next,
		.prev = (next==NULL&&prev==NULL)?lnew:prev,
		.data = deep_copy?data->method->copy(data, MALLOC(data->method->size)):data
	};
	memcpy(lnew, &init, sizeof(*lnew));
	return lnew;
}

dlist*
dlist_push(dlist* he, Object* buf, BOOLEAN deep_copy){
	return dlist_append(he, buf, deep_copy)->prev;
}

dlist*
dlist_append(dlist* he, Object* buf, BOOLEAN deep_copy){
	dlist *lm;
	lm = new_dlist(buf, deep_copy, NULL, NULL);
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
dlist_addOrdered(dlist* he, void* buf, const Comparable_vtable* buf_method, BOOLEAN deep_copy){
	dlist *lm, *run;
	lm = new_dlist((Object*)buf, deep_copy, NULL, NULL);
	if(he == NULL){
		he = lm;
	} else {
		run = he; do{ if(buf_method->compare(buf, run->data) > 0){ break; } run = run->next; } while(run != he);
		//unified cases, since we enforce a FILO-like queue if they
		if(he == run && buf_method->compare(buf, run->data) > 0){
			//are equal (insert before)
			lm->next = run;
			lm->prev = run->prev;
			run->prev->next = lm;
			run->prev = lm;
		} else {
			//insert after
			run->next->prev = lm;
			lm->next = run->next;
			run->next = lm;
			lm->prev = run;
			if( he == run ) he = lm;
		}
		//assert( lm == he       || (buf->method->compare(lm->prev->data, lm->data) <= 0) );
		//assert( lm->next == he || (buf->method->compare(lm->next->data, lm->data) >= 0) );
	}
	return he;
}

dlist*
dlist_copy(dlist* src, BOOLEAN deep_copy){
	if(!src) return NULL;
	dlist* mnew = NULL, *runner = src;
	do{
		Object* new_data = runner->data;
		mnew = dlist_push(mnew, new_data, deep_copy);
		runner = runner->next;
	} while(runner != src);
	return mnew;
}

void
dlist_clear(dlist *head, BOOLEAN destroy_data){
	dlist *run = head, *n = head;
	if(head == NULL) return;
	do{
		if(destroy_data) run->data->method->destroy(run->data);
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

/**
 * Remove head node and return new head
 */
dlist*
dlist_dequeue(dlist *head, Object** data, BOOLEAN destroy_data){
	if(!head) return NULL;
	if(head->next == head){
		if(data) *data = head->data;
		if(destroy_data) head->data->method->destroy(head->data);
		FREE(head);
		head = NULL;
	} else {
		dlist* tmp = head;
		head = head->next;
		tmp->next->prev = tmp->prev;
		tmp->prev->next = tmp->next;
		if(data) *data = tmp->data;
		if(destroy_data) tmp->data->method->destroy(tmp->data);
		FREE(tmp);
	}
	return head;
}

/**
 * Remove node at end of list and return new head
 */
dlist*
dlist_pop(dlist *head, Object** dat, BOOLEAN destroy_data){
	return dlist_dequeue(head->prev, dat, destroy_data);
}

dlist*
dlist_remove(dlist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data){
	dlist *run = head;
	if(!head) return head;
	do {
		if(ordered?key_method->compare(key, run->data) >= 0:key_method->compare(key, run->data) == 0) break;
		run = run->next;
	} while(run != head);
	if(key_method->compare(key, run->data) == 0){
		head = dlist_removeElement(head, run, destroy_data);
	}
	return head;
}

dlist*
dlist_removeAll(dlist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data){
	dlist *run = head;
	if(!head) return head;
	do{
		if(ordered && key_method->compare(key, run->data) > 0) break;
		if(key_method->compare(key, run->data) == 0)
			head = dlist_removeElement(head, run, destroy_data);
		run = run->next;
	} while(run != head);
	return head;
}

dlist*
dlist_removeElement(dlist *head, dlist *rem, BOOLEAN destroy_data){
	if(head == rem) head = dlist_dequeue(head, NULL, destroy_data);
	else head = dlist_dequeue(rem, NULL, destroy_data);
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
			if(!func(pass_data?run->data:(Object*)run, (void*)&ax)) return FALSE;
		} else {
			if(!func(pass_data?run->data:(Object*)run, aux)) return FALSE;
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
	lMapFunc func;
	dlist *list;
};

static BOOLEAN
dlist_filter_f(Object *data, struct dlist_filter_d *aux){
	if(aux->func(data, aux->aux)){
		aux->list = dlist_append(aux->list, data, aux->deep);
	}
	return TRUE;
}

dlist*
dlist_filter(dlist *head, void* aux, lMapFunc func, BOOLEAN deep){
	struct dlist_filter_d dd = {
		.aux = aux,
		.deep = deep,
		.func = func,
		.list = NULL
	};
	dlist_map(head, FALSE, &dd, (lMapFunc)dlist_filter_f);
	return dd.list;
}

dlist*
dlist_filter_i(dlist *head, void* aux, lMapFunc func, BOOLEAN free_data){
	if(!func || !head) return head;
	dlist *run = head;
	do{
		if(!func(run->data, aux)){
			if(run == head){
				BOOLEAN first = TRUE;
				while(head && run == head && (first || !func(run->data, aux))){
					head = run = dlist_dequeue(run, NULL, free_data);
					first = FALSE;
				}
				if(!head) break;
				run = run->next;
			} else {
				run = dlist_dequeue(run, NULL, free_data);
			}
		} else {
			run = run->next;
		}
	} while(run != head);
	return head;
}

struct dlist_transform_d {
	void* aux;
	lTransFunc func;
};

static BOOLEAN
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
dlist_merge(dlist* dst, dlist* src, void *comp, const Comparator_vtable* comp_method){//IF YOUR OBJECT IS NOT COMPARABLE DO NOT USE THIS!!!
	if(comp_method->compare(comp, src->data, dst->data) < 0){ //always merge into list with smaller head
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
			} while(rundst != dst && comp_method->compare(comp, rundst->data, runsrc->data) < 0); //find start
			if(rundst == dst && comp_method->compare(comp, rundst->data, runsrc->data) != 0) l1_done = TRUE;
		}
		//ldp("---------------\nInterstate 1", dst, rundst);
		if(!l2_done){
			do {
				runsrc_end = runsrc_end->next;
			} while(runsrc_end != runsrc && comp_method->compare(comp, rundst->data, runsrc_end->data) >= 0);
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
dlist_sort(dlist* head, void* key, const Comparator_vtable* key_method){//XXX this can be faster, using "natural" mergesort
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
		left = dlist_sort(left, key, key_method);
		right = dlist_sort(right, key, key_method);
		return dlist_merge(left, right, key, key_method);
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
dlist_find(dlist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered){
	if(!head) return NULL;
	dlist* p = head;
	do {
		if(ordered?(key_method->compare(key,p->data) <= 0):(key_method->compare(key,p->data) == 0)) break;
		p = p->next;
	} while(p != head);
	if(key_method->compare(key, p->data) == 0) return p;
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
