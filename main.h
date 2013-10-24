#ifndef _MAIN_H_
#define _MAIN_H_

#include "aLong.h"
#include <sys/time.h>
#include <sys/resource.h>

#define TEST(NAME, FUNC) do { \
	printf("STARTING  ========> %s (%s)\n", NAME, #FUNC);\
	FUNC();\
	printf("FINISHING ========> %s (%s)\n", NAME, #FUNC);\
} while(0)

#define SUBTEST(NAME, FUNC) do { \
	printf("STARTING  ==> %s (%s)\n", NAME, #FUNC);\
	FUNC();\
	printf("FINISHING ==> %s (%s)\n", NAME, #FUNC);\
} while(0)

#ifndef timersub
	#define timersub(a, b, result) \
		if(1) { \
			(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
			(result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
			if ((result)->tv_usec < 0) { \
				--(result)->tv_sec; \
				(result)->tv_usec += 1000000; \
			} \
		}
#endif

#define MIN(a,b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define ARRLENGTH(A) ( sizeof(A)/sizeof(A[0]) )

void test_slist();
void test_dlist();
void test_btree();
void test_graph();

void test_htable();
void test_splay();
void test_scapegoat();
void test_graph_tree();
void test_udlist();

BOOLEAN compare_arrs(const long *expect, const size_t length, const long *got, const size_t got_length);

#endif /* end of include guard: _MAIN_H_ */
