extern "C" {
	#include "linked_structures.h"
}

class cpp_dlist <T> { //an ordered dlist
private:
	dlist *_list;
	size_t _size;
public:
	cpp_dlist(){ }
	cpp_dlist(dlist *list, size_t size):_list(list),_size(size){ }
	virtual ~cpp_dlist(){ }
	size_t size(){ return _size; }
}
