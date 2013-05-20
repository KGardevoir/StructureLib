#ifndef _SHUNTING_YARD_H_
#define _SHUNTING_YARD_H_
#include "token_parser.h"
#include "linked_structures.h"

enum TYPE_ID {//no derived types for now
	TYPE_INTEGER = 0, TYPE_BOOLEAN = 1, TYPE_FLOAT = 2, TYPE_STRING = 3, TYPE_POINTER = 4, TYPE_END = 4/*Last builtin type, so derived types can be easily implemented*/
};
enum MethodOverload_flags {
	METHODOVERLOAD_FLAG_FUNCTIONAL=1,METHODOVERLOAD_FLAG_BUILTIN=2
};

typedef struct MethodOverload_vtable {
	const Object_vtable parent;
	struct {
		const Comparable_vtable compare;//ordered, by sig length, then type
		const Comparable_vtable inserter;
	} privates;
} MethodOverload_vtable;

typedef struct MethodOverload {
	const MethodOverload_vtable *const method;
	const char *final_method;
	intptr_t *signature;
	size_t signature_length;
	intptr_t *returntype;
	size_t returntype_length;
	uint8_t flags;
} MethodOverload;


typedef struct MethodOverloadRoot_vtable{
	const Object_vtable parent;
	struct {
		const Comparable_vtable compare;//how to compare nodes
		const Comparable_vtable locate; //how to locate based on a key
	} privates;
} MethodOverloadRoot_vtable;

typedef struct MethodOverloadRoot {
	const MethodOverloadRoot_vtable *const method;
	const char *key;
	splaytree *overloads;
} MethodOverloadRoot;


typedef struct Token_vtable {
	Object_vtable parent;
} Token_vtable;

enum Token_flags {
	TOKEN_FLAG_OP=1,TOKEN_FLAG_BINARY=2,TOKEN_FLAG_BINARY_MAYBE=4,TOKEN_FLAG_LEFTASSOCIATIVE=8,TOKEN_FLAG_CONSTANT=16,TOKEN_FLAG_IGNORE=32
};
typedef struct Token {
	Token_vtable *method;
	const char *token;
	enum TOKEN_ID tokenid;
	size_t precedence;
	const size_t line, col;
	uint8_t flags;
	dlist *type_list; //using TYPE_ID
	const MethodOverload* resolved;
	dlist *pre_comp; //using char*
} Token;

int shunting_yard(const char* begin, size_t begin_size, slist **treestk, size_t line, size_t *col);
MethodOverload* METHOD_OVERLOADS_find(const char *key, dlist *sig);
BOOLEAN METHOD_OVERLOADS_add(const char *prim_key, const char* real_key, dlist *args, dlist *rets, uint8_t flags);
const char* getTypeIDName(enum TYPE_ID id);

#endif
