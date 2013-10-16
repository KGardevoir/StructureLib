#ifndef _LINKED_STRUCTURES_BASE_H_
#define _LINKED_STRUCTURES_BASE_H_
#define LINKED_MALLOC malloc
#define LINKED_FREE free

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>


typedef struct Object_vtable Object_vtable;
typedef struct Comparable_vtable Comparable_vtable;
typedef struct Comparator_vtable Comparator_vtable;
#undef TRUE
#undef FALSE
typedef enum BOOLEAN { FALSE=0, TRUE=-1, MAYBE=-2 } BOOLEAN;

typedef struct Object {
	const Object_vtable *method;//also can be used as a key for the function type
} Object;

struct Object_vtable {//the most basic form of an object
	char*  (*hashable)(const Object *self, size_t *size); //part of object that is hashable
	void    (*destroy)(const Object *self); //how to destroy our data, NOT deallocate!
	Object*    (*copy)(const Object *self, void* buffer); //return a copy of the information
	BOOLEAN  (*equals)(const Object *self, const void* oth);
	size_t size;
};

struct Comparable_vtable {//an interface for comparable objects
	long  (*compare)(const void* self, const void* oth); //how to compare (NULL if undesired)
};

typedef struct Comparator {
	const Comparator_vtable *method;
} Comparator;

struct Comparator_vtable {
	long  (*compare)(const void* self, const void* oth1, const void* oth2);
};

#define CALL_POSSIBLE(OBJ, METHOD) (OBJ && OBJ->method && OBJ->method->METHOD)
#define CALL(OBJ, METHOD, PARMS, ELSE) (CALL_POSSIBLE(OBJ, METHOD)?(OBJ->method->METHOD PARMS):(ELSE))
#define CALL_VOID(OBJ, METHOD, PARMS) do{ if(CALL_POSSIBLE(OBJ, METHOD)) OBJ->method->METHOD PARMS; } while(0)


typedef struct lMapFuncAux {
	BOOLEAN isAux;
	size_t depth;
	size_t position;
	size_t size;
	void* aux;//user data
} lMapFuncAux;//TODO finish implementing all fields for graphs and htable
typedef BOOLEAN (*lMapFunc)(Object *data, const void *aux/*auxilarly data (constant between calls)*/, void* node); //a mapping function
typedef BOOLEAN (*lTransFunc)(Object **data, const void* aux, const void* node);/*in-place data transformation, should always return TRUE as FALSE means stop*/

typedef enum TRAVERSAL_STRATEGY {
	BREADTH_FIRST=0,
	DEPTH_FIRST_PRE=1,
	DEPTH_FIRST=2,
	DEPTH_FIRST_IN=2,
	DEPTH_FIRST_POST=3,
} TRAVERSAL_STRATEGY;

#endif
