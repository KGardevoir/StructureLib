#ifndef _SCAPEGOAT_H_
#define _SCAPEGOAT_H_
#include "btree.h"

typedef struct scapegoat {
	size_t size;
	size_t max_size;
	const double a;
	const double inv_log_a_inv;
	btree *root;
} scapegoat;

scapegoat* scapegoat_new(double a) __attribute__((warn_unused_result));
void scapegoat_destroy(scapegoat*);
void scapegoat_insert(scapegoat*, Object* data, const Comparable_vtable *data_method, BOOLEAN copy);
Object *scapegoat_remove(scapegoat*, Object* key, const Comparable_vtable *key_method, BOOLEAN destroy_data) __attribute__((warn_unused_result));
void scapegoat_clear(scapegoat*, BOOLEAN clear);

btree *scapegoat_find(scapegoat*, Object* data, const Comparable_vtable *data_method);//returns the btree pointer to where Object is located


#endif /* end of include guard: _SCAPEGOAT_H_ */

