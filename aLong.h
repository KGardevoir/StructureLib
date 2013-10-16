#ifndef _ALONG_H_
#define _ALONG_H_

#include "linked_structures.h"

typedef struct aLong_vtable {
	Object_vtable parent;
	Comparable_vtable compare;
} aLong_vtable;
typedef struct aLong {
	const aLong_vtable *method;
	long data;
} aLong;

typedef struct aLong_comparator_vtable {
	Comparator_vtable compare;
} aLong_comparator_vtable;
typedef struct aLong_comparator {
	aLong_comparator_vtable *method;
} aLong_comparator;

extern aLong_vtable aLong_type;
extern aLong_comparator_vtable aLong_comparator_type;
aLong* aLong_new(long a);

#endif /* end of include guard: _ALONG_H_ */


