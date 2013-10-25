#include "transform.h"

Object**
dlist_toArray(dlist *head, size_t *size, BOOLEAN deep_copy){
size_t len = 0;
dlist* p = head;
	if(!head) return NULL;
	do{ p = p->next; len++; } while(p != head);
	Object **values = (Object**)LINKED_MALLOC((len+1)*sizeof(Object*));
	len = 0; p = head;
	DLIST_ITERATE(p, head) {
		if(deep_copy){
			values[len] = CALL(p->data,copy,p->data, LINKED_MALLOC(p->data->method->size));
		} else {
			values[len] = p->data;
		}
		len++;
	}
	values[len] = NULL;
	*size = len;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}

Object**
dlist_toArrayReverse(dlist *head, size_t *size, BOOLEAN deep_copy){
size_t i = 0, len = 0;
dlist* p = head;
	if(!head) return NULL;
	do{ p = p->next; i++; } while(p != head);
	Object **values = (Object**)LINKED_MALLOC((len+1)*sizeof(Object*));
	len = 0;
	DLIST_ITERATE_REVERSE(p, head){
		if(deep_copy){
			values[len] = CALL(p->data,copy,p->data, LINKED_MALLOC(p->data->method->size));
		} else {
			values[len] = p->data;
		}
		len++;
	}
	values[len] = NULL;
	*size = len;
	//terminate the array, NOTE: This means none of the data can be null, it will only return up to the first non-null data entry
	return values;
}

slist*
dlist_toSlist(dlist *head, BOOLEAN deep_copy){
	slist* p = NULL;
	dlist* d = head;
	if(!d) return NULL;
	do{ p = slist_pushfront(p, d->data, deep_copy); d = d->prev; } while(d != head);
	return p;
}

dlist*
array_toDlist(Object** array, size_t size, BOOLEAN deep_copy){
	dlist *p = NULL;
	if(!array) return NULL;
	if(size == 0) return NULL;
	size_t i = 0;
	for(; i < size; i++){
		p = dlist_pushback(p, array[i], deep_copy);
	}
	return p;
}

Object**
slist_toArray(slist *head, size_t *size, BOOLEAN deep){
size_t len = 0;
slist* p = head;
	if(!head) return NULL;
	for(; p->next; p = p->next, len++);
	Object **values = (Object**)LINKED_MALLOC((len+1)*sizeof(Object*));
	len = 0;
	SLIST_ITERATE(p, head) {
		if(deep){
			values[len] = CALL(p->data,copy,p->data, LINKED_MALLOC(p->data->method->size));
		} else {
			values[len] = p->data;
		}
		len++;
	}
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
		head = slist_pushfront(head, arr[i], deep);
	}
	return head;
}

dlist*
slist_toDlist(slist *head, BOOLEAN deep){
	slist* p = head;
	dlist* d = NULL;
	for(; p; p = p->next){
		d = dlist_pushback(d, p->data, deep);
	}
	return d;
}
