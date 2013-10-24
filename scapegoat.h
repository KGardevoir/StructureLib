#ifndef _SCAPEGOAT_H_
#define _SCAPEGOAT_H_
#include "btree.h"

typedef struct scapegoat {
	size_t size;
	size_t max_size;
	const double a;
	const double inv_log_a_inv;
	const Comparable_vtable *data_method;
	btree *root;
} scapegoat;

scapegoat* scapegoat_new(double a, const Comparable_vtable *data_method) __attribute__((warn_unused_result));
void scapegoat_destroy(scapegoat*);
BOOLEAN scapegoat_insert(scapegoat*, Object* data, BOOLEAN copy);
Object *scapegoat_remove(scapegoat*, Object* key, const Comparable_vtable *key_method, BOOLEAN destroy_data);
Object *scapegoat_find(scapegoat*, Object* data, const Comparable_vtable *data_method);

void scapegoat_clear(scapegoat*, BOOLEAN clear);



#endif /* end of include guard: _SCAPEGOAT_H_ */

