#include "linked_structures.h"
#include "stdio.h"
#include "token_parser.h"

#define MEMCMP(V1, V2) (((V1)>(V2))?1:(((V1)==(V2))?0:-1))
#define ACCESS(TYPE, MEM) ((TYPE)MEM)
#define TL_ACCESS(TYPE1, MEM, TYPE2, MOD) ACCESS(TYPE2, ACCESS(TYPE1, MEM) MOD)
#define GRAPH_LIST_ACCESS(DD) TL_ACCESS(graph*, (DD)->data, Token*, ->data)
#define DESTROY_AND_FREE_GRAPH(DAT) do {\
			DAT->data->method->destroy(DAT->data);\
			DAT->method->parent.destroy((Object*)next);\
			free(DAT->data);\
			free(DAT); } while(0)

#define SLIST_POP(ROOT, TOK, ELSE_CODE) do{\
				if(ROOT){\
					ROOT = slist_pop(ROOT, (Object**)&TOK, FALSE);\
				} else { ELSE_CODE } } while(0)

#if 0
#define DPRINTFERR(...) printf(__VA_ARGS__)
#define DUMP_OPSTACK(STK) do {\
	slist *iter;\
	SLIST_ITERATE(iter, STK,\
		DPRINTFERR("('%s': %d, %lu)", GRAPH_LIST_ACCESS(iter)->token, GRAPH_LIST_ACCESS(iter)->tokenid, GRAPH_LIST_ACCESS(iter)->precedence);\
	);\
	DPRINTFERR("\n");\
} while(0)
#else
#define DPRINTFERR(...)
#define DUMP_OPSTACK(STK)
#endif

static splaytree *METHOD_OVERLOADS = NULL;

enum TYPE_ID {//no derived types for now
	INTEGER_TYPE, BOOLEAN_TYPE, FLOAT_TYPE, STRING_TYPE, POINTER_TYPE, END_IDENTIFIER
};
#define GEN_ENTRY(TYPE) case TYPE: return #TYPE
const char const*
getTypeIDName(enum TYPE_ID id){
	switch(id){
		GEN_ENTRY(INTEGER_TYPE);
		GEN_ENTRY(BOOLEAN_TYPE);
		GEN_ENTRY(FLOAT_TYPE);
		GEN_ENTRY(STRING_TYPE);
		GEN_ENTRY(POINTER_TYPE);
		GEN_ENTRY(END_IDENTIFIER);
	}
	return "UNKNOWN";
}

typedef struct MethodOverload_vtable {
	const Object_vtable parent;
	const Comparable_vtable compare;//the list is strictly unordered, TODO give it order, sort by sig length, then type, then return length then return type
} MethodOverload_vtable;

typedef struct MethodOverload {
	const MethodOverload_vtable const* method;
	const char *final_method;
	intptr_t *signature;
	size_t signature_length;
	intptr_t *returntype;
	size_t returntype_length;
} MethodOverload;

void
MethodOverload_destroy(MethodOverload* self){
	free((char*)self->final_method);
	free(self->signature);
	free(self->returntype);
	free(self);
}

long
MethodOverload_compare(dlist* comp, MethodOverload* self){
	dlist *iterCmp = comp;
	size_t i = 0;
	//printf("%s: ", self->final_method);
	//for(; i < self->signature_length; i++){
	//	printf("%s ", getTypeIDName(self->signature[i]));
	//}
	//printf("\n");
	size_t length = dlist_length(comp);
	if(length != self->signature_length) return MEMCMP(self->signature_length,length);
	i = 0;
	DLIST_ITERATE(iterCmp, comp,
		if(self->signature[i] != (enum TOKEN_ID)iterCmp->data) {
			//printf("Failed on %d because: %d %d\n", i, i >= self->signature_length, self->signature[i] != (enum TOKEN_ID)iterCmp->data);
			return MEMCMP(self->signature[i],(enum TOKEN_ID)iterCmp->data);
		}
		i++;
	);
	return 0;
}

MethodOverload_vtable MethodOverload_type = {
	.parent = {
		.hashable = NULL,
		.destroy = (void(*)(const Object*))MethodOverload_destroy,
		.equals = NULL,
		.copy = NULL
	},
	.compare = {
		.compare = (long(*)(const void*, const void*))MethodOverload_compare
	}
};

MethodOverload*
MethodOverload_new(const char* final_method, dlist* args, dlist *rets){
	MethodOverload init = {
		.method = &MethodOverload_type,
		.final_method = final_method,//TODO copy?
	};
	init.signature = (intptr_t*)dlist_toArray(args, &init.signature_length, FALSE);
	init.returntype = (intptr_t*)dlist_toArray(rets, &init.returntype_length, FALSE);
	return memcpy(malloc(sizeof(MethodOverload)), &init, sizeof(init));
}

typedef struct MethodOverloadRoot_vtable{
	const Object_vtable parent;
	const Comparable_vtable compare;//how to compare nodes
	const Comparable_vtable locate; //how to locate based on a key
} MethodOverloadRoot_vtable;

typedef struct MethodOverloadRoot {
	const MethodOverloadRoot_vtable const *method;
	const char *key;
	splaytree *overloads;
} MethodOverloadRoot;

void
MethodOverloadRoot_destroy(MethodOverloadRoot* self){
	free((char*)self->key);
	bstree_clear(self->overloads, TRUE);
	free(self);
}

BOOLEAN
MethodOverloadRoot_add(MethodOverloadRoot* self, const char* rv, dlist *args, dlist *returntype){
	self->overloads = splay_insert(self->overloads, (Object*)MethodOverload_new(rv, args, returntype), &MethodOverload_type.compare, FALSE);
	return TRUE;
}

long
MethodOverloadRoot_compare(const MethodOverloadRoot* self, const MethodOverloadRoot* oth){
	return strcmp(self->key, oth->key);
}

long
MethodOverloadRoot_locate(const char* key, const MethodOverloadRoot *oth){
	return strcmp(key, oth->key);
}

MethodOverloadRoot_vtable MethodOverloadRoot_type = {
	.parent = {
		.hashable = NULL,
		.destroy = (void(*)(const Object* self))MethodOverloadRoot_destroy,
		.equals = NULL,
		.copy = NULL
	},
	.compare = {
		.compare = (long(*)(const void*, const void*))MethodOverloadRoot_compare
	},
	.locate = {
		.compare = (long(*)(const void*, const void*))MethodOverloadRoot_locate
	}
};

MethodOverloadRoot*
MethodOverloadRoot_new(const char *key){
	MethodOverloadRoot init = {
		.method = &MethodOverloadRoot_type,
		.key = key,//TODO Copy maybe?
		.overloads = NULL
	};
	return memcpy(malloc(sizeof(MethodOverloadRoot)), &init, sizeof(init));
}

BOOLEAN
METHOD_OVERLOADS_add(const char *prim_key, const char* real_key, dlist *args, dlist *rets){
	METHOD_OVERLOADS = splay_find(METHOD_OVERLOADS, (Object*)prim_key, &MethodOverloadRoot_type.locate);
	MethodOverloadRoot *tree;
	if(METHOD_OVERLOADS == NULL || MethodOverloadRoot_type.locate.compare(prim_key, METHOD_OVERLOADS->data) != 0) {
		tree = MethodOverloadRoot_new(prim_key);
		METHOD_OVERLOADS = splay_insert(METHOD_OVERLOADS, (Object*)tree, &MethodOverloadRoot_type.compare, FALSE);
	} else {
		tree = (MethodOverloadRoot*)METHOD_OVERLOADS->data;
	}

	if( tree->overloads == NULL ||
			MethodOverload_type.compare.compare(args,
				(MethodOverload*)(tree->overloads = splay_find(tree->overloads, (Object*)args, &MethodOverload_type.compare))->data) != 0){
		return MethodOverloadRoot_add(tree, real_key, args, rets);
	} else {
		return FALSE;//won't insert, duplicate
	}
}
MethodOverload*
METHOD_OVERLOADS_find(const char *key, dlist *sig){
	METHOD_OVERLOADS = splay_find(METHOD_OVERLOADS, (Object*)key, &MethodOverloadRoot_type.locate);
	if(METHOD_OVERLOADS == NULL || MethodOverloadRoot_type.locate.compare(key, METHOD_OVERLOADS->data) != 0) return NULL;
	MethodOverloadRoot *tree = (MethodOverloadRoot*)METHOD_OVERLOADS->data;
	//printf("tree: '%s'\n", tree->key);
	if(tree->overloads == NULL) return NULL;
	tree->overloads = splay_find(tree->overloads, (Object*)sig, &MethodOverload_type.compare);
	if(MethodOverload_type.compare.compare(sig, tree->overloads->data) != 0) return NULL;
	return (MethodOverload*)tree->overloads->data;
}

typedef struct Token_vtable {
	Object_vtable parent;
} Token_vtable;

typedef struct Token {
	Token_vtable *method;
	const char *token;
	enum TOKEN_ID tokenid;
	size_t precedence;
	const size_t line, col;
	BOOLEAN isOp, isBinary;
	BOOLEAN leftAssociative;
	dlist *type_list; //using TYPE_ID
} Token;

void
Token_destroy(const Token* self){
	free((void*)self->token);
	dlist_clear(self->type_list, FALSE);
}

void
Token_setupPrecedence(Token *self){
	switch(self->tokenid){
		case IDENTIFIER:   self->precedence = 12; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case FUNCTION:     self->precedence = 12; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case FLOAT:        self->precedence =  0; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case INTEGER:      self->precedence =  0; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case HEX_INTEGER:  self->precedence =  0; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case OCT_INTEGER:  self->precedence =  0; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case LPAREN:       self->precedence = 12; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case RPAREN:       self->precedence = 12; self->isOp = FALSE; self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case ASSIGN:       self->precedence = 12; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case BOOLNOT:      self->precedence = 11; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = FALSE; break;
		case BITNOT:       self->precedence = 11; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = FALSE; break;
		case MULT:         self->precedence = 10; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case DIV:          self->precedence = 10; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case MOD:          self->precedence = 10; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case MINUS:
			if(self->isBinary){
				self->precedence =  9; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = MAYBE; break;
			} else {
				self->precedence = 11; self->isOp = TRUE;  self->leftAssociative = FALSE; self->isBinary = FALSE;  break;
			}
		case PLUS:
			if(self->isBinary){
				self->precedence =  9; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = MAYBE; break;
			} else {
				self->precedence = 11; self->isOp = TRUE;  self->leftAssociative = FALSE; self->isBinary = FALSE;  break;
			}
		case LSHIFT:       self->precedence =  8; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case RSHIFT:       self->precedence =  8; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case GETHAN:       self->precedence =  7; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case LETHAN:       self->precedence =  7; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case LTHAN:        self->precedence =  7; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case GTHAN:        self->precedence =  7; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case EQUAL:        self->precedence =  6; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case NOTEQUAL:     self->precedence =  6; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case BITAND:       self->precedence =  5; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case BITXOR:       self->precedence =  4; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case BITOR:        self->precedence =  3; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case BOOLAND:      self->precedence =  2; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case BOOLXOR:      self->precedence =  1; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case BOOLOR:       self->precedence =  0; self->isOp = TRUE;  self->leftAssociative = TRUE;  self->isBinary = TRUE;  break;
		case UMINUS:       self->precedence = 11; self->isOp = TRUE;  self->leftAssociative = FALSE; self->isBinary = FALSE; break;
		case UPLUS:        self->precedence = 11; self->isOp = TRUE;  self->leftAssociative = FALSE; self->isBinary = FALSE; break;
		case COMMA:
		case BEGIN:
		case END:
		case UNRECOGNIZED: self->precedence = 12; self->isOp = FALSE; self->leftAssociative = TRUE; self->isBinary = TRUE; break;
	}
}

Token_vtable Token_type = {
	.parent = {
		.hashable = NULL,
		.destroy = (void(*)(const Object* self))Token_destroy,
		.copy = NULL,
		.equals = NULL
	}
};

Token*
Token_new(Token* self, const char *pstok, const char* pftok, enum TOKEN_ID id, size_t line, size_t col){
	Token init = {
		.method = &Token_type,
		.token = memcpy(memset(malloc(pftok-pstok+1), 0, pftok-pstok+1), pstok, pftok-pstok), //this feels all sorts of unsafe...
		.tokenid = id,
		.line = line,
		.col = col,
		.type_list = NULL,
		.isBinary = TRUE,
	};
	Token_setupPrecedence(&init);
	memcpy(self, &init, sizeof(init));
	//DPRINTFERR("New Token: '%s', %d, %lu, %d, %d, %d\n", self->token, self->tokenid, self->precedence, self->isOp, self->leftAssociative, self->tokenid == UNRECOGNIZED);
	return self;
}

int
shunting_yard(const char* begin, slist **treestk, size_t line, size_t *col){
	enum TOKEN_ID id;
	const char *end = NULL;
	slist *opstk = NULL;
	const char *rbegin = begin;
	graph *prev = NULL;//we need lookbehind to determine if unary operators are actually unary operators.
	while((id = parse_tokenize(begin, &begin, &end)) != END && *begin != '\0'){
		graph *next = graph_insert(NULL, (Object*)Token_new(malloc(sizeof(Token)), begin, end, id, line, *col + (begin - rbegin)), FALSE);
		if(id == FLOAT || id == INTEGER || id == HEX_INTEGER || id == OCT_INTEGER){
			DPRINTFERR("(LITERAL)");
			*treestk = slist_push(*treestk, (Object*)next, FALSE);
		} else if(id == IDENTIFIER){//might be function, save
			const char *lbegin = end, *lend = NULL;
			int lid = parse_tokenize(lbegin, &lbegin, &lend);
			if(lid == LPAREN){//look ahead, if LPAREN, token is function
				DPRINTFERR("(FUNCTION)");
				ACCESS(Token*,next->data)->tokenid = FUNCTION;
				Token_setupPrecedence(ACCESS(Token*,next->data));
				end = lend;
				opstk = slist_push(opstk, (Object*)next, FALSE);
				*treestk = slist_push(*treestk, (Object*)next, FALSE);
			} else if(lid == UNRECOGNIZED){
				*col = ACCESS(Token*,next->data)->col;
				DESTROY_AND_FREE_GRAPH(next);
				return 2;//need more info
			} else {
				DPRINTFERR("(IDENTIFIER)");
				*treestk = slist_push(*treestk, (Object*)next, FALSE);
			}
		} else if(id == LPAREN){
			DPRINTFERR("(LPAREN)");
			opstk = slist_push(opstk, (Object*)next, FALSE);
		} else if(id == COMMA){
			DPRINTFERR("(COMMA)");
			DUMP_OPSTACK(opstk);
			DESTROY_AND_FREE_GRAPH(next);
			while(opstk && (GRAPH_LIST_ACCESS(opstk)->tokenid != FUNCTION)){
				graph *token;
				opstk = slist_pop(opstk, (Object**)&token, FALSE);
				if(ACCESS(Token*, token->data)->isBinary){
					graph *left, *right;
					SLIST_POP(*treestk, left, return 3;);
					SLIST_POP(*treestk, right, return 3;);
					token = graph_link(token, right);
					token = graph_link(token, left);
				} else {
					graph *arg;
					SLIST_POP(*treestk, arg, return 3;);
					token = graph_link(token, arg);
				}
				*treestk = slist_push(*treestk, (Object*)token, FALSE);
			}
			DUMP_OPSTACK(opstk);
			if(!opstk){
				*col = begin-rbegin;
				return 1;
				//TODO mismatched operators, warn about this
			}
		} else if(ACCESS(Token*, next->data)->isOp){
			DPRINTFERR("(OP: '%s', %d)\n", ACCESS(Token*, next->data)->token, ACCESS(Token*, next->data)->tokenid);
			DUMP_OPSTACK(opstk);
			if(ACCESS(Token*, next->data)->isBinary == MAYBE){//TODO check if previous token actually makes no sense
				BOOLEAN isBinary = TRUE;
				if(!prev || ACCESS(Token*, prev->data)->isOp){
					ACCESS(Token*, next->data)->isBinary = FALSE;
					isBinary = FALSE;
				}
				Token_setupPrecedence(ACCESS(Token*, next->data));
				ACCESS(Token*, next->data)->isBinary = isBinary;
			}
			while(opstk &&
					( GRAPH_LIST_ACCESS(opstk)->isOp &&
					  (((ACCESS(Token*, next->data)->leftAssociative && ACCESS(Token*, next->data)->precedence <= GRAPH_LIST_ACCESS(opstk)->precedence)
					  || ACCESS(Token*, next->data)->precedence < GRAPH_LIST_ACCESS(opstk)->precedence) )) ){
				graph *token;
				opstk = slist_pop(opstk, (Object**)&token, FALSE);
				if(ACCESS(Token*, token->data)->isBinary){
					graph *left, *right;
					SLIST_POP(*treestk, left, return 3;);
					SLIST_POP(*treestk, right, return 3;);
					token = graph_link(token, right);
					token = graph_link(token, left);
				} else {
					graph *arg;
					SLIST_POP(*treestk, arg, return 3;);
					token = graph_link(token, arg);
				}
				*treestk = slist_push(*treestk, (Object*)token, FALSE);
			}
			DUMP_OPSTACK(opstk);
			opstk = slist_push(opstk, (Object*)next, FALSE);
		} else if(id == RPAREN){
			DPRINTFERR("(RPAREN)");
			next->method->parent.destroy((Object*)next);
			free(next);
			DUMP_OPSTACK(opstk);
			while(opstk && GRAPH_LIST_ACCESS(opstk)->tokenid != FUNCTION && GRAPH_LIST_ACCESS(opstk)->tokenid != LPAREN){
				graph *token;
				opstk = slist_pop(opstk, (Object**)&token, FALSE);
				if(ACCESS(Token*, token->data)->isBinary){
					graph *left, *right;
					SLIST_POP(*treestk, left, return 3;);
					SLIST_POP(*treestk, right, return 3;);
					token = graph_link(token, right);
					token = graph_link(token, left);
				} else {
					graph *arg;
					SLIST_POP(*treestk, arg, return 3;);
					token = graph_link(token, arg);
				}
				*treestk = slist_push(*treestk, (Object*)token, FALSE);
			}
			DUMP_OPSTACK(opstk);
			if(opstk){
				graph *token;
				opstk = slist_pop(opstk, (Object**)&token, FALSE);
				if(ACCESS(Token*, token->data)->tokenid == FUNCTION){//find matching one in treestk to
					while(*treestk && GRAPH_LIST_ACCESS(*treestk)->tokenid != FUNCTION){
						graph *nxs;
						*treestk = slist_pop(*treestk, (Object**)&nxs, FALSE);
						token = graph_link(token, nxs);
					}
					if(!*treestk){
						return 4; //FIXME something really bad happened, there should always be two of these
					}
				} else {//is LPAREN
					DESTROY_AND_FREE_GRAPH(token);
				}
			} else {
				//TODO tell user about op mismatch
				DPRINTFERR("(ERROR PAREN MISMATCH: %d)", ACCESS(Token*,next->data)->tokenid);
				DESTROY_AND_FREE_GRAPH(next);
				*col = *col + (begin - rbegin);
				return 1;
			}
		} else {
			break;
		}
		begin = end;
		prev = next;
	}
	if(id == END){
		DPRINTFERR("(DONE)\n");
		while(opstk){//pop the remains only operators (no functions) should remain
			graph *token;
			opstk = slist_pop(opstk, (Object**)&token, FALSE);
			if(ACCESS(Token*,token)->tokenid == FUNCTION || ACCESS(Token*,token)->tokenid == LPAREN){
				//TODO report location (error)
			}
			if(ACCESS(Token*, token->data)->isBinary){
				graph *left, *right;
				SLIST_POP(*treestk, left, return 3;);
				SLIST_POP(*treestk, right, return 3;);
				token = graph_link(token, right);
				token = graph_link(token, left);
			} else {
				graph *arg;
				SLIST_POP(*treestk, arg, return 3;);
				token = graph_link(token, arg);
			}
			*treestk = slist_push(*treestk, (Object*)token, FALSE);
		}
		*col = rbegin-begin;
		return 0;//we are done
	}
	return 5;//return not done
}

static BOOLEAN
process_tree(graph *gr){
	//TODO process types, lookup types
	switch(ACCESS(Token*, gr->data)->tokenid){
		case IDENTIFIER:
			//TODO make them be able to assume other types
		case INTEGER:
		case HEX_INTEGER:
		case OCT_INTEGER:
			ACCESS(Token*, gr->data)->type_list = dlist_push(ACCESS(Token*, gr->data)->type_list, (Object*)INTEGER_TYPE, FALSE);
			printf("%s ", ACCESS(Token*, gr->data)->token);
			break;
		case FLOAT:
			ACCESS(Token*, gr->data)->type_list = dlist_push(ACCESS(Token*, gr->data)->type_list, (Object*)FLOAT_TYPE, FALSE);
			printf("%s ", ACCESS(Token*, gr->data)->token);
			break;
		default: {//all others need to be evaluated
			dlist *args = NULL;
			dlist *iter = NULL;
			DLIST_ITERATE(iter, gr->edges,
				args = dlist_concat(args, dlist_copy(ACCESS(Token*,ACCESS(graph*,iter->data)->data)->type_list, FALSE));
			);
			MethodOverload* method = METHOD_OVERLOADS_find(ACCESS(Token*, gr->data)->token, args);//TODO fill in "lookup function"
			if(!method){//FIXME ERROR!!! no known overload
				printf("!Unkown overload '%s'(", ACCESS(Token*, gr->data)->token);
				DLIST_ITERATE(iter, args,
					printf("%s,", getTypeIDName((enum TYPE_ID)iter->data));
				);
				printf(")! ");
			} else {
				ACCESS(Token*,gr->data)->type_list = array_toDlist((Object**)method->returntype, method->returntype_length, FALSE);
				printf("%s ", method->final_method);
			}
			dlist_clear(args, FALSE);
		}
	}
	return TRUE;
}

#include "dlist_maker.h"
void
init_overloads(){
	dlist *l1, *l2;
	METHOD_OVERLOADS_add("*",  "*",    l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("%",  "mod",  l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("/",  "/",    l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("+",  "+",    l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "-",    l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "neg",  l1 = DLIST_MAKE(INTEGER_TYPE),                   l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("|",  "or",   l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("&",  "and",  l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("^",  "xor",  l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("~",  "1com", l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<",  "<",    l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<=", "<=",   l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">=", ">=",   l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">",  ">",    l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("==", "=",    l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("!=", "<>",   l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<<", "shlv", l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">>", "shrv", l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE),      l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("!",  "not",  l1 = DLIST_MAKE(BOOLEAN_TYPE),                   l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("||", "or",   l1 = DLIST_MAKE(BOOLEAN_TYPE,BOOLEAN_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("&&", "and",  l1 = DLIST_MAKE(BOOLEAN_TYPE,BOOLEAN_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("^^", "xor",  l1 = DLIST_MAKE(BOOLEAN_TYPE,BOOLEAN_TYPE),      l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("*",  "f*",   l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(FLOAT_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("%",  "fmod", l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(FLOAT_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("/",  "f/",   l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(FLOAT_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("+",  "f+",   l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(FLOAT_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "f-",   l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(FLOAT_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "fneg", l1 = DLIST_MAKE(FLOAT_TYPE),                     l2 = DLIST_MAKE(FLOAT_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<",  "f<",   l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<=", "f<=",  l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">=", "f>=",  l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">",  "f>",   l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("==", "f=",   l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("!=", "f<>",  l1 = DLIST_MAKE(FLOAT_TYPE,FLOAT_TYPE),          l2 = DLIST_MAKE(BOOLEAN_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("float",   "itof",  l1 = DLIST_MAKE(INTEGER_TYPE), l2 = DLIST_MAKE(FLOAT_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("integer", "ftoi",  l1 = DLIST_MAKE(FLOAT_TYPE),   l2 = DLIST_MAKE(INTEGER_TYPE)); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("multdiv", "i*/",  l1 = DLIST_MAKE(INTEGER_TYPE,INTEGER_TYPE,INTEGER_TYPE), l2 = DLIST_MAKE(INTEGER_TYPE));   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
}

int
main(int argc, const char **argv){
	(void)argc;
	(void)argv;
	init_overloads();
	//read from stdin
	FILE *fin = stdin;
	//TODO setup initial types
	if(fin != NULL){
		char line[128];
		slist *gdata = NULL;
		size_t lineno = 0;
		//BOOLEAN begin = FALSE;//assumed
		while( fgets( &line[0], sizeof(line), fin) != NULL ){//TODO handle tokens broken up by buffer underfill...
			//printf("%s\n", &line[0]);
			size_t colno = 0;
			int rcode = shunting_yard(&line[0], &gdata, lineno, &colno);
			if(rcode != 0){
				printf("Error (%d) occured at %lu\n", rcode, colno);
			//if(rcode == 2){
			//	lineno++;
			//	continue;
			//} else if(rcode == 1){
			//	printf("Operator Mismatch at %lu:%lu\n", lineno, colno);
			//	return 1;
			} else {
				slist *iter;//TODO this leaks memory like crazy
				SLIST_ITERATE(iter, gdata,
					graph_map(ACCESS(graph*, iter->data), DEPTH_FIRST_POST, FALSE, FALSE, NULL, (lMapFunc)process_tree);
					printf(", ");
				);
				slist_clear(gdata, FALSE);
				//TODO process tokens, ensure type congruency
				return 0;//TODO may continue?
			}
		}
	}
	return 0;
}
