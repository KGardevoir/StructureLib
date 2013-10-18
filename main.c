#include "main.h"

int
main(int argc, char** argv){
	TEST("Singly Linked Lists", test_slist);
	TEST("Doubly Linked Circular Lists", test_dlist);
	TEST("Binary Trees", test_btree);
	TEST("Graphs", test_graph);

	TEST("Splay Trees", test_splay);
	TEST("Scapegoat Trees", test_scapegoat);
	TEST("Tree-like graphs", test_graph_tree);
	TEST("Unrolled dlists (Array Lists)", test_udlist);

	TEST("Hash Tables", test_htable);
	return 0;
}
