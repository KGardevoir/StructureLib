#ifndef _LINKED_STRUCTURES_TRANSFORM_H_
#define _LINKED_STRUCTURES_TRANSFORM_H_
#include "dlist.h"
#include "slist.h"

//Transform
Object** dlist_toArray(dlist *head, size_t *size, BOOLEAN deep) __attribute__((warn_unused_result));
Object** dlist_toArrayReverse(dlist *head, size_t *size, BOOLEAN deep) __attribute__((warn_unused_result));
slist* dlist_toSlist(dlist *he, BOOLEAN deep) __attribute__((warn_unused_result));
dlist* array_toDlist(Object **array, size_t size, BOOLEAN deep) __attribute__((warn_unused_result));

//Transform
Object** slist_toArray(slist *head, size_t *size, BOOLEAN deep) __attribute__((warn_unused_result));
Object** slist_toArrayReverse(slist *head, size_t *size, BOOLEAN deep) __attribute__((warn_unused_result));
dlist* slist_toDlist(slist *head, BOOLEAN deep) __attribute__((warn_unused_result));
slist* array_toSlist(Object** head, size_t size, BOOLEAN deep) __attribute__((warn_unused_result));


#endif
