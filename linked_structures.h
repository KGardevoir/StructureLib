#ifndef _LINKED_STRUCTURES_H_
#define _LINKED_STRUCTURES_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void* (*lMemoryAllocator)(size_t);//how to allocate
typedef void* (*lMemoryFree)(void*);
typedef void* (*lDeepCopy)(const void* src); //return a copy of the information (within a newly allocated buffer)
typedef long  (*lCompare)(const void* key, const void*); //how to compare (NULL if undesired)
typedef long  (*lKeyCompare)(const void *key, const void*);
typedef char* (*lToString)(const void*); //how to convert to a string

typedef struct list_tspec {
	lMemoryAllocator adalloc;
	lMemoryFree adfree;
	lDeepCopy deep_copy;
	lCompare compar;
	lKeyCompare key_compar;
	lToString strfunc;
	char* name; //type name
} list_tspec;

typedef struct slist {
	struct slist *next;
	void* data;
} slist;

typedef struct dlist {
	struct dlist *next;
	void* data;
	struct dlist *prev;
} dlist;

typedef enum BOOLEAN { FALSE=0, TRUE=-1 } BOOLEAN;



//Add
slist* slist_push(slist* he, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
slist* slist_append(slist* he, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
slist* slist_addOrdered(slist* he, void* buf, BOOLEAN copy, BOOLEAN overwrite, list_tspec*) __attribute__((warn_unused_result));
slist* slist_copy(slist* src, BOOLEAN deep_copy, list_tspec*) __attribute__((warn_unused_result));

//Remove
void slist_clear(slist *he, BOOLEAN free_data, list_tspec*);
slist* slist_dequeue(slist *head, void** data, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
slist* slist_pop(slist *head, void** data, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
slist* slist_removeElement(slist *head, slist *rem, BOOLEAN free_data, list_tspec* type) __attribute__((warn_unused_result));
slist* slist_removeViaAllKey(slist *head, void **data, void* key, BOOLEAN ordered, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
slist* slist_removeViaKey(slist *head, void **data, void* key, BOOLEAN ordered, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));

//Other Operations
typedef slist* (*SLMapFunc)(void*,slist*); //a mapping function
slist* slist_map(slist *head, SLMapFunc) __attribute__((warn_unused_result));
//Transform
void** slist_toArray(slist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
void** slist_toArrayReverse(slist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
dlist* slist_to_dlist(slist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));



//Find
void* slist_find(slist *head, void* key, BOOLEAN ordered, list_tspec*) __attribute__((warn_unused_result));
//END SINGLE_LINKED_LISTdnl

//Doubly linked list functions
//Add
dlist* dlist_push(dlist*, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result)) __attribute__((warn_unused_result));
dlist* dlist_append(dlist*, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_addOrdered(dlist*, void* buf, BOOLEAN copy, BOOLEAN overwrite, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_copy(dlist* src, BOOLEAN deep_copy, list_tspec*) __attribute__((warn_unused_result));

//Remove
void dlist_clear(dlist *he, BOOLEAN free_data, list_tspec*);
dlist* dlist_dequeue(dlist* head, void** data, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_pop(dlist* he, void** data, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_removeViaKey(dlist*, void **data, void *key, BOOLEAN ordered, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_removeAllViaKey(dlist*, void **data, void *key, BOOLEAN ordered, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_removeElement(dlist *head, dlist *rem, BOOLEAN free_data, list_tspec* type) __attribute__((warn_unused_result));

//Other Operations
typedef dlist* (*LMapFunc)(void*,dlist*); //a mapping function
dlist* dlist_map(dlist *head, LMapFunc);
//Transform
void** dlist_toArray(dlist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
void** dlist_toArrayReverse(dlist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
slist* dlist_to_slist(dlist *he, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));

//Merging
dlist* dlist_sort(dlist* head, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_merge(dlist* dst, dlist* src, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_concat(dlist* dst, dlist* src) __attribute__((warn_unused_result));
dlist* dlist_split(dlist* h1, dlist* h2) __attribute__((warn_unused_result));

//Find
//Data returned from find is PACKED which means that it returns the node, not the data, where the data resides
dlist* dlist_find(dlist *head, const void* key, BOOLEAN ordered, list_tspec*) __attribute__((warn_unused_result));
//DOUBLE_LINKED_LIST
#endif //_LINKED_STRUCTURES_H_
