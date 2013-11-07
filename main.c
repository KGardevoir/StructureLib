#include "main.h"

BOOLEAN
compare_arrs(const long *expect, const size_t length, const long *got, const size_t got_length){
	size_t i = 0;
	size_t minlen = MIN(length, got_length);
	BOOLEAN failure = FALSE;
	for(; i < minlen; i++){
		if(expect[i] != got[i]){
			printf("FAILURE, value %lu does not match (Got: (", i);
			for(i=0; i < got_length; i++)
				printf("%ld ", got[i]);
			printf("), Expected: (");
			for(i=0; i < length; i++)
				printf("%ld ", expect[i]);
			printf(")\n");
			failure = TRUE;
		}
	}
	if(length != got_length){
		printf("FAILURE, inconsitent number of nodes (Got: %ld, Expected: %ld)\n", got_length, length);
		failure = TRUE;
	}
	if(!failure) printf("PASS\n");
	return !failure;
}

int
main(int argc, char** argv){
	(void)argc;
	(void)argv;
	TEST("Singly Linked Lists", test_slist);
	TEST("Doubly Linked Circular Lists", test_dlist);
	TEST("Binary Trees", test_btree);
	TEST("Splay Trees", test_splay);
	TEST("Graphs", test_graph);

	TEST("Scapegoat Trees", test_scapegoat);
	TEST("Tree-like graphs", test_graph_tree);
	TEST("Unrolled dlists (Array Lists)", test_udlist);

	//TEST("Hash Tables", test_htable);
	return 0;
}
