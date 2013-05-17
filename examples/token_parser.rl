#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "token_parser.h"

%%{
	machine tokenizer;
	write data;
}%%

enum TOKEN_ID
parse_tokenize(const char *prim, const char **pbegin, const char **pend){
	const char *begin = NULL;
	const char* p = prim;
	const char* pe = p + strlen(p);
	const char *eof = pe;
	int cs;
	int act;
	const char *ts, *te;
	%%{
		action START {
			begin = p;
		}
		action DONE {
			*pbegin = begin; *pend = p;
		}
		#ident = ([a-zA-Z_][a-zA-Z0-9_]*) >{ begin = p; } %{  *pbegin = begin; *pend = p; return IDENTIFIER; };
		#number = ((([0-9]*'.'[0-9]*) - '.') | ([1-9][0-9]*) | ("0x"i /[0-9a-f]/i+) | ("0"[0-7]*) )
		#	>{ begin = p; }
		#	%{  *pbegin = begin;
		#		*pend = p;
		#		int i = 0;
		#		for(; begin+i < p; i++) if(begin[i] == '.') return FLOAT;
		#		if((p-begin > 1 && begin[1] == 'X') || begin[1] == 'X') return HEX_INTEGER;
		#		if(p-begin > 1 && begin[0] == '0') return OCT_INTEGER;
		#		return INTEGER;
		#	};
		token := |*
			 (" " | "\t" | "\n");
			 "BEGIN"i >START %DONE => {return BEGIN; };
			 "END"i  >START %DONE => {return END; };
			 ([a-zA-Z_][a-zA-Z0-9_]*) >START %DONE => { return IDENTIFIER; };
			 (([0-9]*'.'[0-9]*) - '.') >START %DONE => { return FLOAT; };
			 (([1-9][0-9]*) | "0") >START %DONE => { return INTEGER; };
			 ("0x"i /[0-9a-f]/i+) >START %DONE => { return HEX_INTEGER; };
			 ("0"[0-7]*) >START %DONE => { return OCT_INTEGER; };
			 ","  >START %DONE => { return COMMA; }   ;
			 "("  >START %DONE => { return LPAREN; }  ;
			 ")"  >START %DONE => { return RPAREN; }  ;
			 "="  >START %DONE => { return ASSIGN; }  ;
			 "!"  >START %DONE => { return BOOLNOT; } ;
			 "~"  >START %DONE => { return BITNOT; }  ;
			 "*"  >START %DONE => { return MULT; }    ;
			 "/"  >START %DONE => { return DIV; }     ;
			 "%"  >START %DONE => { return MOD; }     ;
			 "+"  >START %DONE => { return PLUS; }    ;
			 "-"  >START %DONE => { return MINUS; }   ;
			 "<=" >START %DONE => { return LETHAN; }  ;
			 "<<" >START %DONE => { return LSHIFT; }  ;
			 "<"  >START %DONE => { return LTHAN; }   ;
			 ">=" >START %DONE => { return GETHAN; }  ;
			 ">>" >START %DONE => { return RSHIFT; }  ;
			 ">"  >START %DONE => { return GTHAN; }   ;
			 "==" >START %DONE => { return EQUAL; }   ;
			 "!=" >START %DONE => { return NOTEQUAL; };
			 "&"  >START %DONE => { return BITAND; }  ;
			 "^"  >START %DONE => { return BITXOR; }  ;
			 "|"  >START %DONE => { return BITOR; }   ;
			 "&&" >START %DONE => { return BOOLAND; } ;
			 "^^" >START %DONE => { return BOOLXOR; } ;
			 "||" >START %DONE => { return BOOLOR; }  ;
		*|;
		write init;
		write exec;
	}%%
	return UNRECOGNIZED;
}

#if 0
#define PRINTPREC(STATE) printf(STATE" %lu\n", parse_token_precede(STATE))
#define PRINTTOK(TESTV) do { \
	const char *begin = NULL, *end = NULL; \
	size_t type = parse_tokenize(TESTV, &begin, &end); \
	char *cpy = memcpy(memset(malloc(end-begin), 0, end-begin), begin, end-begin);\
	printf("'"TESTV"' %lu, '%s' %p %p\n", type, cpy, begin, end);\
	free(cpy);\
} while(0)
int
main(int argc, const char** argv){
	PRINTPREC("asdf");
	PRINTPREC("(");  PRINTPREC(")");  PRINTPREC("=");
	PRINTPREC("~");  PRINTPREC("!");
	PRINTPREC("*");  PRINTPREC("/");  PRINTPREC("%");
	PRINTPREC("+");  PRINTPREC("-");
	PRINTPREC("<<"); PRINTPREC(">>");
	PRINTPREC("<");  PRINTPREC(">");  PRINTPREC("<="); PRINTPREC(">=");
	PRINTPREC("=="); PRINTPREC("!=");
	PRINTPREC("&");
	PRINTPREC("^");
	PRINTPREC("|");
	PRINTPREC("&&");
	PRINTPREC("^^");
	PRINTPREC("||");
	PRINTTOK(" atoken "); PRINTTOK("0123a"); PRINTTOK("123");
	PRINTTOK("1."); PRINTTOK("."); PRINTTOK(".1"); PRINTTOK("1.1");
	return 0;
}
#endif
