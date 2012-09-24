#include <stdio.h>
#include "linked_structures.h"
#define MAX(a,b) ({ typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ typeof(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
long compare_ints(void *a, void *b){ return (long)((long)(a) - (long)(b)); }

double_linkedList*
make_dlist(int a[], size_t s, tspec* type){
	long i = 0;
	double_linkedList* newl = NULL;
	for(; i < s; i++){
		newl = double_append(newl, NULL, (void*)a[i], 0, type);
	}
	return newl;
}
double_linkedList*
print_list(double_linkedList* list){
	double_linkedList* run = list;
	do {
		printf("<%p, %p %p>: %d\n", run->prev, run, run->next, (int)run->data);
		run = run->next;
	} while(run != list);
	return list;
}

int
main(int argc, char** argv){
	tspec list_type = {malloc, free, NULL, compare_ints, compare_ints, NULL, NULL};
	int x1[] = { 1, 3, 5, 6, 9, 10, 13 };
	int x2[] = { 7, 6, 5, 4, 3, 2, 1, 8, 11, 14, 9, 10 };
	int x3[] = { 3, 4, 7, 8, 11, 12, 14, 15, 16, 17, 18 };
	printf("List  1---------------------------\n");
	double_linkedList *l1 = print_list(make_dlist(x1, sizeof(x1)/sizeof(int), &list_type));
	printf("List  2---------------------------\n");
	double_linkedList *l2 = print_list(make_dlist(x2, sizeof(x2)/sizeof(int), &list_type));
	printf("List  3---------------------------\n");
	double_linkedList *l3 = print_list(make_dlist(x3, sizeof(x3)/sizeof(int), &list_type));
	//printf("Test  1---------------------------\n");
	//print_list(double_concatLists(l1, l3));
	//print_list(double_concatLists(l1, l2));
	//print_list(double_mergeLists(l1, l3, compare_ints));
	printf("Test  2---------------------------\n");
	print_list(double_sort(l2, &list_type));
	return 0;
}
