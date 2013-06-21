#include "shunting_yard.h"
#include "stdio.h"
//TODO Implement Identifiers (Requires deeper support, some sort of external system)
//TODO Implement Strings, Booleans (Partial done, comparisons return BOOLEAN, but no strings)
//TODO Implement Type Definition System
//TODO Implement typecasting
//TODO Implement ref-counting
//TODO Implement member access operator actor.method(...) (flips "actor" such that method(actor, ...))
//TODO Ternary operator (?:)                (it is left associative, precedence 2)
//TODO Implement subscription, e.g. expr[i] (it is right associative, precedence 15)
//TODO Implement varargs
//TODO Create a stdlib

#define MEMCMP(V1, V2) (((V1)>(V2))?1:(((V1)==(V2))?0:-1))
#define ACCESS(TYPE, MEM) ((TYPE)MEM)
#define TL_ACCESS(TYPE1, MEM, TYPE2, MOD) ACCESS(TYPE2, ACCESS(TYPE1, MEM) MOD)
#define GRAPH_LIST_ACCESS(DD) TL_ACCESS(graph*, (DD)->data, Token*, ->data)
#define DESTROY_AND_FREE_GRAPH(DAT) do {\
			DAT->data->method->destroy(DAT->data);\
			DAT->method->parent.destroy((Object*)DAT);\
			LINKED_FREE(DAT); } while(0)

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

const char*
getTypeIDName(enum TYPE_ID id){
#define GEN_ENTRY(TYPE) case TYPE: return #TYPE
	switch(id){
		GEN_ENTRY(TYPE_INTEGER);
		GEN_ENTRY(TYPE_BOOLEAN);
		GEN_ENTRY(TYPE_FLOAT);
		GEN_ENTRY(TYPE_STRING);
		GEN_ENTRY(TYPE_POINTER);
	}
#undef GEN_ENTRY
	return "UNKNOWN/DERIVED";
}


static void
MethodOverload_destroy(MethodOverload* self){
	LINKED_FREE((char*)self->final_method);
	LINKED_FREE(self->signature);
	LINKED_FREE(self->returntype);
	LINKED_FREE(self);
}

static long
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
	DLIST_ITERATE(iterCmp, comp,
		if(self->signature[i] != (enum TOKEN_ID)iterCmp->data) {
			//printf("Failed on %d because: %d %d\n", i, i >= self->signature_length, self->signature[i] != (enum TOKEN_ID)iterCmp->data);
			return MEMCMP(self->signature[i],(enum TOKEN_ID)iterCmp->data);
		}
		i++;
	);
	return 0;
}

static long
MethodOverload_inserter(MethodOverload* self, MethodOverload* new){
	size_t i = 0;
	if(new->signature_length != self->signature_length) return MEMCMP(new->signature_length, self->signature_length);
	for(; i < new->signature_length; i++){
		if(new->signature[i] != self->signature[i]) return MEMCMP(new->signature[i], self->signature[i]);
	}
	return 0;
}

MethodOverload_vtable MethodOverload_type = {
	.parent = {
		.hashable = NULL,
		.destroy = (void(*)(const Object*))MethodOverload_destroy,
		.equals = NULL,
		.copy = NULL
	},
	.privates= {
		.compare = {
			.compare = (long(*)(const void*, const void*))MethodOverload_compare
		},
		.inserter = {
			.compare = (long(*)(const void*, const void*))MethodOverload_inserter
		}
	}
};

static MethodOverload*
MethodOverload_new(const char* final_method, dlist* args, dlist *rets, uint8_t flags){
	MethodOverload init = {
		.method = &MethodOverload_type,
		.final_method = strcpy(LINKED_MALLOC((strlen(final_method)+1)*sizeof(char)),final_method),
		.flags = flags,
	};
	init.signature = (intptr_t*)dlist_toArray(args, &init.signature_length, FALSE);
	init.returntype = (intptr_t*)dlist_toArray(rets, &init.returntype_length, FALSE);
	return memcpy(LINKED_MALLOC(sizeof(MethodOverload)), &init, sizeof(init));
}

static void
MethodOverloadRoot_destroy(MethodOverloadRoot* self){
	LINKED_FREE((char*)self->key);
	btree_clear(self->overloads, TRUE);
	LINKED_FREE(self);
}

static BOOLEAN
MethodOverloadRoot_add(MethodOverloadRoot* self, const char* rv, dlist *args, dlist *returntype, uint8_t flags){
	self->overloads = splay_insert(self->overloads, (Object*)MethodOverload_new(rv, args, returntype, flags), &MethodOverload_type.privates.inserter, FALSE);
	return TRUE;
}

static long
MethodOverloadRoot_compare(const MethodOverloadRoot* self, const MethodOverloadRoot* oth){
	return strcmp(self->key, oth->key);
}

static long
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
	.privates = {
		.compare = {
			.compare = (long(*)(const void*, const void*))MethodOverloadRoot_compare
		},
		.locate = {
			.compare = (long(*)(const void*, const void*))MethodOverloadRoot_locate
		}
	}
};

static MethodOverloadRoot*
MethodOverloadRoot_new(const char *key){
	MethodOverloadRoot init = {
		.method = &MethodOverloadRoot_type,
		.key = strcpy(LINKED_MALLOC((strlen(key)+1)*sizeof(char)),key),
		.overloads = NULL
	};
	return memcpy(LINKED_MALLOC(sizeof(MethodOverloadRoot)), &init, sizeof(init));
}

BOOLEAN
METHOD_OVERLOADS_add(const char *prim_key, const char* real_key, dlist *args, dlist *rets, uint8_t flags){
	METHOD_OVERLOADS = splay_find(METHOD_OVERLOADS, (Object*)prim_key, &MethodOverloadRoot_type.privates.locate);
	MethodOverloadRoot *tree;
	if(METHOD_OVERLOADS == NULL || MethodOverloadRoot_type.privates.locate.compare(prim_key, METHOD_OVERLOADS->data) != 0) {
		tree = MethodOverloadRoot_new(prim_key);
		METHOD_OVERLOADS = splay_insert(METHOD_OVERLOADS, (Object*)tree, &MethodOverloadRoot_type.privates.compare, FALSE);
	} else {
		tree = (MethodOverloadRoot*)METHOD_OVERLOADS->data;
	}
	//printf("a tree: '%s'\n", tree->key);

	if( tree->overloads == NULL ||
			MethodOverload_type.privates.compare.compare(args,
				(MethodOverload*)(tree->overloads = splay_find(tree->overloads, (Object*)args, &MethodOverload_type.privates.compare))->data) != 0){
		BOOLEAN b = MethodOverloadRoot_add(tree, real_key, args, rets, flags);
		//printf("aa tree: '%s'\n", ((MethodOverload*)tree->overloads->data)->final_method);
		return b;
	} else {
		return FALSE;//won't insert, duplicate
	}
}

MethodOverload*
METHOD_OVERLOADS_find(const char *key, dlist *sig){
	METHOD_OVERLOADS = splay_find(METHOD_OVERLOADS, (Object*)key, &MethodOverloadRoot_type.privates.locate);
	if(METHOD_OVERLOADS == NULL || MethodOverloadRoot_type.privates.locate.compare(key, METHOD_OVERLOADS->data) != 0) return NULL;
	MethodOverloadRoot *tree = (MethodOverloadRoot*)METHOD_OVERLOADS->data;
	//printf("f tree: '%s'\n", tree->key);
	if(tree->overloads == NULL) return NULL;
	tree->overloads = splay_find(tree->overloads, (Object*)sig, &MethodOverload_type.privates.compare);
	//printf("ff tree: '%s'\n", ((MethodOverload*)tree->overloads->data)->final_method);
	if(MethodOverload_type.privates.compare.compare(sig, tree->overloads->data) != 0) return NULL;
	return (MethodOverload*)tree->overloads->data;
}

static void
Token_destroy(const Token* self){
	LINKED_FREE((void*)self->token);
	dlist_clear(self->type_list, FALSE);
	if(self->pre_comp){
		dlist *tmp;
		DLIST_ITERATE(tmp, self->pre_comp,
			LINKED_FREE(tmp->data);
		);
	}
	dlist_clear(self->pre_comp, FALSE);
	LINKED_FREE((Token*)self);
}

/**
 * Precedence (.. is unactualized)
 * A[i]              ==> 15 .. subscription
 * f(x)              ==> 15 function call
 * '.'               ==> 15 .. member acquisition
 * '~'               ==> 14 unary '~'
 * '!'               ==> 14 unary '!'
 * '+' '-'           ==> 14 unary '+','-' respective
 * '*'               ==> 14 .. unary '*'
 * '*' '/' '%'       ==> 13 binary '*','/','%' respective
 * '+' '-'           ==> 12 binary '+','-' respective
 * '<<' '>>'         ==> 11 binary '<<','>>' respective
 * '<' '<=' '>' '>=' ==> 10 binary '<','<=','>','>=' respective
 * '==' '!='         ==> 9  binary '==','!=' respective
 * '&'               ==> 8  binary '&'
 * '^'               ==> 7  binary '|'
 * '|'               ==> 6  binary '^'
 * '&&'              ==> 5  binary '&&'
 * '^^'              ==> 4  binary '^^'
 * '||'              ==> 3  binary '||'
 * '?:'              ==> 2  ternary '?:'
 * '='               ==> 1  binary '='
 */
static void
Token_setupPrecedence(Token *self){
	switch(self->tokenid){
		case IDENTIFIER:   self->precedence = 15; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case FUNCTION:     self->precedence = 15; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case FLOAT:        self->precedence =  0; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case INTEGER:      self->precedence =  0; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case HEX_INTEGER:  self->precedence =  0; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case OCT_INTEGER:  self->precedence =  0; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case LPAREN:       self->precedence = 15; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case RPAREN:       self->precedence = 15; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case ASSIGN:       self->precedence =  1; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case BOOLNOT:      self->precedence = 14; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE; break;
		case BITNOT:       self->precedence = 14; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case MULT:         self->precedence = 13; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case DIV:          self->precedence = 13; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case MOD:          self->precedence = 13; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case MINUS:
			if(self->flags & TOKEN_FLAG_BINARY_MAYBE){
				self->precedence = 14; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY_MAYBE;
			} else {
				if(self->flags & TOKEN_FLAG_BINARY){
					self->precedence = 14; self->flags = TOKEN_FLAG_OP;
				} else {
					self->precedence = 13; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY;
				}
			}
			break;
		case PLUS:
			if(self->flags & TOKEN_FLAG_BINARY_MAYBE){
				self->precedence = 14; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY_MAYBE;
			} else {
				if(self->flags & TOKEN_FLAG_BINARY){
					self->precedence = 14; self->flags = TOKEN_FLAG_OP;
				} else {
					self->precedence = 13; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY;
				}
			}
		case LSHIFT:       self->precedence = 11; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case RSHIFT:       self->precedence = 11; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case GETHAN:       self->precedence = 10; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case LETHAN:       self->precedence = 10; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case LTHAN:        self->precedence = 10; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case GTHAN:        self->precedence = 10; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case EQUAL:        self->precedence =  9; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case NOTEQUAL:     self->precedence =  9; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case BITAND:       self->precedence =  8; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case BITXOR:       self->precedence =  7; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case BITOR:        self->precedence =  6; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case BOOLAND:      self->precedence =  5; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case BOOLXOR:      self->precedence =  4; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case BOOLOR:       self->precedence =  3; self->flags = TOKEN_FLAG_OP | TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
		case UMINUS:       self->precedence = 14; self->flags = TOKEN_FLAG_OP; break;
		case UPLUS:        self->precedence = 14; self->flags = TOKEN_FLAG_OP; break;
		case COMMA:
		case MORE:
		case END:
		case UNRECOGNIZED: self->precedence =  0; self->flags = TOKEN_FLAG_LEFTASSOCIATIVE | TOKEN_FLAG_BINARY; break;
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

static Token*
Token_new(Token* self, const char *pstok, const char* pftok, enum TOKEN_ID id, size_t line, size_t col){
	Token init = {
		.method = &Token_type,
		.token = memcpy(memset(LINKED_MALLOC(pftok-pstok+1), 0, pftok-pstok+1), pstok, pftok-pstok), //this feels all sorts of unsafe...
		.tokenid = id,
		.line = line,
		.col = col,
		.type_list = NULL,
		.flags = TOKEN_FLAG_BINARY_MAYBE | TOKEN_FLAG_BINARY,
		.resolved = NULL
	};
	Token_setupPrecedence(&init);
	memcpy(self, &init, sizeof(init));
	//DPRINTFERR("New Token: '%s', %d, %lu, %d, %d, %d\n", self->token, self->tokenid, self->precedence, self->isOp, self->leftAssociative, self->tokenid == UNRECOGNIZED);
	return self;
}

static FILE *fin;
static BOOLEAN
buffer_reload(const char* buf, size_t buf_size){
	char* rt = fgets((char*) buf, buf_size-1, fin);
	if(strchr(&buf[0], '\n') == NULL){
		printf("Either reached end of file, or buffer overfilled...\n");
		char* end = strchr(&buf[0], '\0');
		end[0] = '\n';
		end[1] = '\0';
	}
	return rt != NULL;
}

int
shunting_yard(const char* begin, size_t begin_size, slist **treestk, size_t line, size_t *col){
	enum TOKEN_ID id;
	const char *end = NULL;
	slist *opstk = NULL;
	const char *rbegin = begin;
	graph *prev = NULL;//we need lookbehind to determine if unary operators are actually unary operators.
	while((id = parse_tokenize(begin, &begin, &end)) != END && *begin != '\0'){
		if(id == MORE){
			if(!buffer_reload(begin, begin_size)) break;
			(*col)++;
			continue;
		}
		graph *next = graph_insert(NULL, (Object*)Token_new(LINKED_MALLOC(sizeof(Token)), begin, end, id, line, *col + (begin - rbegin)), FALSE);
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
				if(ACCESS(Token*, token->data)->flags & TOKEN_FLAG_BINARY){
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
		} else if(ACCESS(Token*, next->data)->flags & TOKEN_FLAG_OP){
			DPRINTFERR("(OP: '%s', %d)\n", ACCESS(Token*, next->data)->token, ACCESS(Token*, next->data)->tokenid);
			DUMP_OPSTACK(opstk);
			if(ACCESS(Token*, next->data)->flags & TOKEN_FLAG_BINARY_MAYBE){
				ACCESS(Token*, next->data)->flags &= ~TOKEN_FLAG_BINARY_MAYBE;
				if(!prev || (ACCESS(Token*, prev->data)->flags & TOKEN_FLAG_OP)){
					ACCESS(Token*, next->data)->flags &= ~TOKEN_FLAG_BINARY;
				}
				Token_setupPrecedence(ACCESS(Token*, next->data));
			}
			while(opstk &&
					( (GRAPH_LIST_ACCESS(opstk)->flags & TOKEN_FLAG_OP) &&
					  ((((ACCESS(Token*, next->data)->flags & TOKEN_FLAG_LEFTASSOCIATIVE) && ACCESS(Token*, next->data)->precedence <= GRAPH_LIST_ACCESS(opstk)->precedence)
					  || ACCESS(Token*, next->data)->precedence < GRAPH_LIST_ACCESS(opstk)->precedence) )) ){
				graph *token;
				opstk = slist_pop(opstk, (Object**)&token, FALSE);
				if(ACCESS(Token*, token->data)->flags & TOKEN_FLAG_BINARY){
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
			DESTROY_AND_FREE_GRAPH(next);
			DUMP_OPSTACK(opstk);
			while(opstk && GRAPH_LIST_ACCESS(opstk)->tokenid != FUNCTION && GRAPH_LIST_ACCESS(opstk)->tokenid != LPAREN){
				graph *token;
				opstk = slist_pop(opstk, (Object**)&token, FALSE);
				if(ACCESS(Token*, token->data)->flags & TOKEN_FLAG_BINARY){
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
						return 4; //FIXME Something really bad happened, as in we accepted an operator with fewer than an acceptable amount of tokens. (This shouldn't happen ATM, but if this algorithm changes sufficiently, it may not be the case)
					}
				} else {//is LPAREN
					DESTROY_AND_FREE_GRAPH(token);
				}
			} else {
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
			if(ACCESS(Token*, token->data)->flags & TOKEN_FLAG_BINARY){
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

#define COMPILE_TOKEN(TOK) printf("%s ", TOK)
#define COMPILE_ERROR_STREAM(FMT ...) printf(FMT)
#define COMPILE_ERROR(FMT ...) do { printf(FMT); printf("\n"); }while(0)

//Sends token to system for interpretation
#define INTERP_TOKEN(TOK)
//this obtains a token from the system (probably some stack) cooresponding to the given type, as a string
#define OBTAIN_TOKEN(TYPE) NULL

static BOOLEAN
resolve_overloads(graph *gr){
	switch(ACCESS(Token*, gr->data)->tokenid){
		case IDENTIFIER:
			//TODO Add the ability of identifiers to assume other types. Also see if it is constant (which at this moment it won't be)
			ACCESS(Token*, gr->data)->type_list = dlist_push(ACCESS(Token*, gr->data)->type_list, (Object*)TYPE_INTEGER, FALSE);
			break;
		case INTEGER:
		case HEX_INTEGER:
		case OCT_INTEGER:
			ACCESS(Token*, gr->data)->flags |= TOKEN_FLAG_CONSTANT;
			ACCESS(Token*, gr->data)->type_list = dlist_push(ACCESS(Token*, gr->data)->type_list, (Object*)TYPE_INTEGER, FALSE);
			break;
		case FLOAT:
			ACCESS(Token*, gr->data)->type_list = dlist_push(ACCESS(Token*, gr->data)->type_list, (Object*)TYPE_FLOAT, FALSE);
			break;
		default: {//all others need to be evaluated
			dlist *args = NULL;
			dlist *iter = NULL;
			BOOLEAN all_const = TRUE;
			DLIST_ITERATE(iter, gr->edges,
				all_const = all_const && (ACCESS(Token*,ACCESS(graph*,iter->data)->data)->flags & TOKEN_FLAG_CONSTANT);
				args = dlist_concat(args, dlist_copy(ACCESS(Token*,ACCESS(graph*,iter->data)->data)->type_list, FALSE));
			);
			MethodOverload* method = METHOD_OVERLOADS_find(ACCESS(Token*, gr->data)->token, args);
			if(!method){//FIXME ERROR we incurred an unknown overload
				COMPILE_ERROR_STREAM("!Unknown overload '%s'(", ACCESS(Token*, gr->data)->token);
				DLIST_ITERATE(iter, args,
					if(iter == args){
						COMPILE_ERROR_STREAM("%s", getTypeIDName((enum TYPE_ID)iter->data));
					} else {
						COMPILE_ERROR_STREAM(",%s", getTypeIDName((enum TYPE_ID)iter->data));
					}
				);
				COMPILE_ERROR(")! ");
			} else {
				if(all_const && (method->flags & METHODOVERLOAD_FLAG_FUNCTIONAL)){
					ACCESS(Token*,gr->data)->flags |= TOKEN_FLAG_CONSTANT;
				}
				ACCESS(Token*,gr->data)->type_list = array_toDlist((Object**)method->returntype, method->returntype_length, FALSE);
				ACCESS(Token*,gr->data)->resolved = method;
			}
			dlist_clear(args, FALSE);
		}
	}
	return TRUE;
}

static BOOLEAN
sparse_eval(graph *gr, dlist** toks){
	Token *tk = ACCESS(Token*,gr->data);
	if(tk->flags & TOKEN_FLAG_CONSTANT){//our result was constant
		if(tk->resolved == NULL){
			*toks = dlist_append(*toks, (Object*)tk, FALSE);
		} else {
			if(tk->resolved->flags & METHODOVERLOAD_FLAG_FUNCTIONAL){
				//flush tokens from tok to evalutor
				dlist *run1;
				DLIST_ITERATE(run1, *toks,
					Token *mtk = ACCESS(Token*, run1->data);
					mtk->flags |= TOKEN_FLAG_IGNORE;
					if(mtk->pre_comp){
						dlist *run2;
						DLIST_ITERATE(run2, mtk->pre_comp,
							INTERP_TOKEN((char*)run2->data);
							LINKED_FREE(run2->data);
						);
						dlist_clear(mtk->pre_comp, FALSE);
						mtk->pre_comp = NULL;
					}
				);
				INTERP_TOKEN(tk->resolved->final_method);
				size_t i = 0;
				for(; i < tk->resolved->returntype_length; i++){
					tk->pre_comp = dlist_append(tk->pre_comp, OBTAIN_TOKEN(tk->resolved->returntype[i]), FALSE);
				}
				
			}
			dlist_clear(*toks, FALSE);
			*toks = NULL;
		}
	}
	return TRUE;
}

static BOOLEAN
compile_tree(graph *gr){
	Token *tk = ACCESS(Token*, gr->data);
	if(!(tk->flags & TOKEN_FLAG_IGNORE)){
		if(tk->pre_comp){
			dlist* run;
			DLIST_ITERATE(run, tk->pre_comp,
				COMPILE_TOKEN((char*)run->data);
			);
		} else {
			if(tk->resolved != NULL)
				COMPILE_TOKEN(tk->resolved->final_method);
			else
				COMPILE_TOKEN(tk->token);
		}
	}
	return TRUE;
}


#include "dlist_maker.h"
static void
init_overloads(){
	dlist *l1, *l2;
	METHOD_OVERLOADS_add("*",  "*",    l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("%",  "mod",  l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("/",  "/",    l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("+",  "+",    l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "-",    l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "neg",  l1 = DLIST_MAKE(TYPE_INTEGER),              l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("|",  "or",   l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("&",  "and",  l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("^",  "xor",  l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("~",  "1com", l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<",  "<",    l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<=", "<=",   l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">=", ">=",   l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">",  ">",    l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("==", "=",    l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("!=", "<>",   l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<<", "shlv", l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">>", "shrv", l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("!",  "not",  l1 = DLIST_MAKE(TYPE_BOOLEAN),              l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("||", "or",   l1 = DLIST_MAKE(TYPE_BOOLEAN,TYPE_BOOLEAN), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("&&", "and",  l1 = DLIST_MAKE(TYPE_BOOLEAN,TYPE_BOOLEAN), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("^^", "xor",  l1 = DLIST_MAKE(TYPE_BOOLEAN,TYPE_BOOLEAN), l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("*",  "f*",   l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_FLOAT),   METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("%",  "fmod", l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_FLOAT),   METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("/",  "f/",   l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_FLOAT),   METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("+",  "f+",   l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_FLOAT),   METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "f-",   l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_FLOAT),   METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("-",  "fneg", l1 = DLIST_MAKE(TYPE_FLOAT),                l2 = DLIST_MAKE(TYPE_FLOAT),   METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<",  "f<",   l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("<=", "f<=",  l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">=", "f>=",  l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add(">",  "f>",   l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("==", "f=",   l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("!=", "f<>",  l1 = DLIST_MAKE(TYPE_FLOAT,TYPE_FLOAT),     l2 = DLIST_MAKE(TYPE_BOOLEAN), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("float",   "itof",  l1 = DLIST_MAKE(TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_FLOAT),   METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN);   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
	METHOD_OVERLOADS_add("integer", "ftoi",  l1 = DLIST_MAKE(TYPE_FLOAT),   l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN); dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);

	METHOD_OVERLOADS_add("multdiv", "i*/",  l1 = DLIST_MAKE(TYPE_INTEGER,TYPE_INTEGER,TYPE_INTEGER), l2 = DLIST_MAKE(TYPE_INTEGER), METHODOVERLOAD_FLAG_FUNCTIONAL | METHODOVERLOAD_FLAG_BUILTIN);   dlist_clear(l1, FALSE); dlist_clear(l2, FALSE);
}

int
main(int argc, const char **argv){
	(void)argc;
	(void)argv;
	init_overloads();
	//read from stdin
	fin = stdin;
	if(fin != NULL){
		char line[128];
		slist *gdata = NULL;
		size_t lineno = 0;
		//BOOLEAN begin = FALSE;//assumed
		while( buffer_reload(&line[0], sizeof(line) ) != FALSE ){
			//printf("%s\n", &line[0]);
			size_t colno = 0;
			int rcode = shunting_yard(&line[0], sizeof(line), &gdata, lineno, &colno);
			if(rcode != 0){
				printf("Error (%d) occured at %lu\n", rcode, colno);
			//if(rcode == 2){
			//	lineno++;
			//	continue;
			//} else if(rcode == 1){
			//	printf("Operator Mismatch at %lu:%lu\n", lineno, colno);
			//	return 1;
			} else {
				slist *iter;
				SLIST_ITERATE(iter, gdata,
					if(iter != gdata) printf(", ");
					graph_map(ACCESS(graph*, iter->data), DEPTH_FIRST_POST, FALSE, FALSE, NULL, (lMapFunc)resolve_overloads);
					//dlist* toks = NULL;
					//graph_map(ACCESS(graph*, iter->data), DEPTH_FIRST_POST, FALSE, FALSE, &toks, (lMapFunc)sparse_eval);
					graph_map(ACCESS(graph*, iter->data), DEPTH_FIRST_POST, FALSE, FALSE, NULL, (lMapFunc)compile_tree);
				);
				SLIST_ITERATE(iter, gdata,
					graph_clear(ACCESS(graph*, iter->data), TRUE);
				);
				slist_clear(gdata, FALSE);
				btree_clear(METHOD_OVERLOADS, TRUE);
				return 0;
			}
		}
	}
	return 0;
}
