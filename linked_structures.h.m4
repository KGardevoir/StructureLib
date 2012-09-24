include(linked_structures.h.name.m4)dnl
pushdef(-{H_NAME}-, translit(translit(H_NAME, -{a-z}-,-{A-Z}-),-{.}-,-{_}-))dnl
-{#ifndef}- -{_}-H_NAME-{_}-
-{#define}- -{_}-H_NAME-{_}-
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TSPEC_NAME {
	void* (*adalloc)(size_t);//how to allocate
	void  (*adfree)(void*); //how to free
	void* (*deep_copy)(void* src); //return a copy of the information (within a newly allocated buffer)
	long  (*compar)(void* key,void*); //how to compare (NULL if undesired)
	long  (*key_compar)(void *key, void*);
	char* (*strfunc)(void*); //how to convert to a string
	char* name; //type name
} TSPEC_NAME;

ifdef(-{SLL}-,-{dnl
typedef struct SLL_NAME {
	struct SLL_NAME *next;
	void* data;
} SLL_NAME-{}-;}-)dnl

ifdef(-{DCLL}-,-{dnl
typedef struct DCLL_NAME {
	struct DCLL_NAME *next;
	void* data;
	struct DCLL_NAME *prev;
} DCLL_NAME-{}-;}-)dnl

typedef enum BOOLEAN { FALSE=0, TRUE=-1 } BOOLEAN;

ifelse(0,1,-{dnl
struct linkedBuffer {
	struct linkedBuffer *prev, *next;
	size_t buf_size;
	char buffer[];
};
#define MAX_BUFFER_SIZE 0x100000
//limit size to 1 MB
struct linkedBuffer* linkedBuffer_add(struct linkedBuffer*);
void linkedBuffer_destruct(struct linkedBuffer*);}-)

ifdef(-{SLL}-,-{dnl
//Add
SLL_NAME* SLL_PREFIX-{}-push(SLL_NAME* he, void* buf, BOOLEAN copy, TSPEC_NAME*);
SLL_NAME* SLL_PREFIX-{}-append(SLL_NAME* he, void* buf, BOOLEAN copy, TSPEC_NAME*);
SLL_NAME* SLL_PREFIX-{}-addOrdered(SLL_NAME* he, void* buf, BOOLEAN copy, BOOLEAN overwrite, TSPEC_NAME*);
SLL_NAME* SLL_PREFIX-{}-copy(SLL_NAME* src, BOOLEAN deep_copy, TSPEC_NAME*);

//Remove
void SLL_PREFIX-{}-clear(SLL_NAME *he, BOOLEAN free_data, TSPEC_NAME*);
SLL_NAME* SLL_PREFIX-{}-dequeue(SLL_NAME *head, void** data, BOOLEAN free_data, TSPEC_NAME*);
SLL_NAME* SLL_PREFIX-{}-pop(SLL_NAME *head, void** data, BOOLEAN free_data, TSPEC_NAME*);
SLL_NAME* SLL_PREFIX-{}-removeViaKey(SLL_NAME *head, void **data, void* key, BOOLEAN ordered, BOOLEAN free_data, TSPEC_NAME*);

//Other Operations
SLL_NAME* SLL_PREFIX-{}-map(SLL_NAME *head, SLL_NAME* (func)(void*, SLL_NAME*));
//Transform
ifdef(-{SLL_TRANSFORMS}-,-{dnl
	void** SLL_PREFIX-{}-toArray(SLL_NAME *head, BOOLEAN deep, TSPEC_NAME*);
	void** SLL_PREFIX-{}-toArrayReverse(SLL_NAME *head, BOOLEAN deep, TSPEC_NAME*);
	ifdef(-{DCLL}-, DCLL_NAME* SLL_NAME-{}-_to_-{}-DCLL_NAME-{}-(SLL_NAME *head, BOOLEAN deep, TSPEC_NAME*);)
}-)

ifdef(-{SLL_MERGE}-,-{dnl
//unimplemented, and kinda useless since these are far more expensive than their cdll counterparts
	SLL_NAME* SLL_PREFIX-{}-sort(SLL_NAME* head, TSPEC_NAME*);
	SLL_NAME* SLL_PREFIX-{}-merge(SLL_NAME* dst, SLL_NAME* src, TSPEC_NAME*);
	SLL_NAME* SLL_PREFIX-{}-concat(SLL_NAME* dst, SLL_NAME* src);
	SLL_NAME* SLL_PREFIX-{}-split(SLL_NAME* h1, SLL_NAME* h2);
//end unimplemented}-)dnl

//Find
void* SLL_PREFIX-{}-find(SLL_NAME *head, void* key, BOOLEAN ordered, TSPEC_NAME*);
//END SINGLE_LINKED_LIST}-)dnl

ifdef(-{DCLL}-,-{dnl
//Doubly linked list functions
//Add
DCLL_NAME* DCLL_PREFIX-{}-push(DCLL_NAME*, DCLL_NAME** nhead, void* buf, BOOLEAN copy, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-append(DCLL_NAME*, DCLL_NAME** nhead, void* buf, BOOLEAN copy, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-addOrdered(DCLL_NAME*, DCLL_NAME** nhead, void* buf, BOOLEAN copy, BOOLEAN overwrite, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-copy(DCLL_NAME* src, BOOLEAN deep_copy, TSPEC_NAME*);

//Remove
void DCLL_PREFIX-{}-clear(DCLL_NAME *he, BOOLEAN free_data, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-dequeue(DCLL_NAME* head, void** data, BOOLEAN free_data, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-pop(DCLL_NAME* he, void** data, BOOLEAN free_data, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-removeViaKey(DCLL_NAME*, void **data, void *key, BOOLEAN ordered, BOOLEAN free_data, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-removeElement(DCLL_NAME *head, DCLL_NAME *rem, BOOLEAN free_data, TSPEC_NAME* type);

//Other Operations
DCLL_NAME* DCLL_PREFIX-{}-map(DCLL_NAME *head, DCLL_NAME* (func)(void*, DCLL_NAME*));
//Transform
ifdef(-{DCLL_TRANSFORMS}-,-{dnl
void** DCLL_PREFIX-{}-toArray(DCLL_NAME *head, BOOLEAN deep, TSPEC_NAME*);
void** DCLL_PREFIX-{}-toArrayReverse(DCLL_NAME *head, BOOLEAN deep, TSPEC_NAME*);
ifdef(-{SLL}-, -{SLL_NAME* DCLL_NAME-{}-_to_-{}-SLL_NAME-{}-(DCLL_NAME *he, BOOLEAN deep, TSPEC_NAME*);}-)
}-)dnl

//Merging
ifdef(-{DCLL_MERGE}-,-{dnl
DCLL_NAME* DCLL_PREFIX-{}-sort(DCLL_NAME* head, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-merge(DCLL_NAME* dst, DCLL_NAME* src, TSPEC_NAME*);
DCLL_NAME* DCLL_PREFIX-{}-concat(DCLL_NAME* dst, DCLL_NAME* src);
DCLL_NAME* DCLL_PREFIX-{}-split(DCLL_NAME* h1, DCLL_NAME* h2);}-)

//Find
DCLL_NAME* DCLL_PREFIX-{}-find(DCLL_NAME *head, void* key, BOOLEAN ordered, TSPEC_NAME*);
//DOUBLE_LINKED_LIST
}-)dnl
-{#endif}- //-{_}-H_NAME-{_}-
popdef(-{H_NAME}-)dnl
