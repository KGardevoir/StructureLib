#include "aLong.h"

static long aLong_compare(const aLong *self, const aLong *b){ return self->data - b->data; }
static void aLong_destroy(aLong *self) { LINKED_FREE(self); }
static char*
aLong_hashes(aLong *self, size_t *len){
	*len = sizeof(aLong);
	return (char*)self;
}

aLong_vtable aLong_type = {
	.parent = {
		.hash = NULL,
		.hashable = aLong_hashes,
		.destroy = aLong_destroy,
		.copy = NULL,
		.equals = NULL,
		.size = sizeof(aLong)
	},
	.compare = {
		.compare = aLong_compare
	}
};

aLong*
aLong_new(long a){
	aLong init = {
		.method = &aLong_type,
		.data = a
	};
	aLong *lnew = (aLong*)LINKED_MALLOC(sizeof(init));
	memcpy(lnew, &init, sizeof(init));
	return lnew;
}

static long aLong_comparator_compare(const aLong_comparator *self, const aLong *one, const aLong* two){
	return aLong_compare(one, two);
}

aLong_comparator_vtable aLong_comparator_type = {
	.compare = {
		.compare = aLong_comparator_compare
	}
};
