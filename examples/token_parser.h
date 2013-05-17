enum TOKEN_ID {
	IDENTIFIER, FUNCTION, FLOAT, INTEGER, HEX_INTEGER, OCT_INTEGER, LPAREN, RPAREN, ASSIGN, BOOLNOT, BITNOT, MULT, DIV,
	MOD, PLUS, MINUS, LSHIFT, RSHIFT, GETHAN, LETHAN, LTHAN, GTHAN, EQUAL, NOTEQUAL, BITAND, BITXOR, BITOR, BOOLAND,
	BOOLXOR, BOOLOR, UMINUS, UPLUS, MORE, END, COMMA, UNRECOGNIZED
};

enum TOKEN_ID parse_tokenize(const char *prim, const char **pbegin, const char **pend);

