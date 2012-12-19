SRC=dlist.c slist.c splaytree.c bstree.c
CFLAGS:=-Wall -O2 $(CFLAGS)
OBJ=$(addsuffix .o, $(basename $(SRC)))
LIB=liblinked.a
CC=gcc

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@

.PHONY: all
all: $(OBJ)
	ar rvs $(LIB) $^

test: main.o all
	$(CC) main.o -L. -llinked $(CFLAGS) -g -o $@

clean:
	rm -rf $(OBJ) $(LIB)
	rm -f test
