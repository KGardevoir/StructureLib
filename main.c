#include <stdio.h>
#include "linked_structures.h"
#define MAX(a,b) ({ typeof(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define MIN(a,b) ({ typeof(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
long compare_ints(void *a, void *b){ return (long)((long)(a) - (long)(b)); }

dlist*
make_dlist(long a[], size_t s, list_tspec* type){
	long i = 0;
	dlist* newl = NULL;
	for(; i < s; i++){
		newl = dlist_append(newl, (void*)a[i], 0, type);
	}
	return newl;
}
dlist*
print_list(dlist* list){
	dlist* run = list;
	do {
		printf("<%p, %p %p>: %ld\n", run->prev, run, run->next, (long)run->data);
		run = run->next;
	} while(run != list);
	return list;
}

static long
long long_cmp(long a, long b){ return a - b; }

size_t
print_splay_values(splaytree* root, size_t i){
	if(root == NULL) return i;
	i = print_splay_values(root->left, i);
	//printf("%lu <%p,%p,%p>\n", root->data, root->left, root, root->right);
	i = print_splay_values(root->right, i);
	return i + 1;
}
void
test_splay(){
	splaytree *t = NULL; list_tspec type = {.adalloc = (lMemoryAllocator)malloc, .adfree = (lMemoryFree)free, .compar = (lCompare)long_cmp};
	size_t NUMS = 40000;
	size_t GAP  =   307;

	printf("Checking... (no bad output means success)\n");
	{
		long i;
		for(i = GAP; i != 0; i = (i + GAP) % NUMS){
			t = splay_insert(t, (void*)i, FALSE, &type);
		}
		printf("Inserts complete\n");
		printf("%lu Nodes\n", print_splay_values(t, 0));
		for(i = 1; i < NUMS; i+= 2)
			t = splay_remove(t, NULL, (void*)i, FALSE, &type);
		printf("Removes complete\n");
		printf("%lu Nodes\n", print_splay_values(t, 0));
	}

	{
		long i;
		t = splay_findmin(t, (void**)&i, &type);
		long j;
		t = splay_findmax(t, (void**)&j, &type);
		if(i != 2 || j != NUMS-2)
			printf("FindMin or FindMax error! Got %lu and %lu\n", i, j);
	}
	{
		long i;
		for(i = 2; i < NUMS; i+=2){
			long k;
			t = splay_find(t, (void**)&k, (void*)i, &type);
			if(k != i)
				printf("Error: find fails for %ld\n",i);
		}
		for(i = 1; i < NUMS; i+=2){
			long k;
			t = splay_find(t, (void**)&k, (void*)i, &type);
			if(k != 0)
				printf("Error: Found deleted item %ld\n",i);
		}
	}
}

int
main(int argc, char** argv){
	list_tspec list_type = {(lMemoryAllocator)malloc, (lMemoryFree)free, NULL, (lCompare)compare_ints, (lCompare)compare_ints, NULL};
	long x1[] = { 1, 3, 5, 6, 9, 10, 13 };
	long x2[] = { 7, 6, 5, 4, 3, 2, 1, 8, 11, 14, 9, 10 };
	long x3[] = { 3, 4, 7, 8, 11, 12, 14, 15, 16, 17, 18 };
	printf("List  1---------------------------\n");
	dlist *l1 = print_list(make_dlist(x1, sizeof(x1)/sizeof(long), &list_type));
	printf("List  2---------------------------\n");
	dlist *l2 = print_list(make_dlist(x2, sizeof(x2)/sizeof(long), &list_type));
	printf("List  3---------------------------\n");
	dlist *l3 = print_list(make_dlist(x3, sizeof(x3)/sizeof(long), &list_type));
	//printf("Test  1---------------------------\n");
	//print_list(double_concatLists(l1, l3));
	//print_list(double_concatLists(l1, l2));
	//print_list(double_mergeLists(l1, l3, compare_ints));
	printf("Test  2---------------------------\n");
	print_list(dlist_sort(l2, &list_type));
	test_splay();
	return 0;
}
