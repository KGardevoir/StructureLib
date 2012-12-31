SRC=dlist.c slist.c splaytree.c bstree.c graphs.c
CFLAGS:=-Wall -Wno-pointer-sign -O2 $(CFLAGS)
OBJ=$(addsuffix .o, $(basename $(SRC))) #tlsf/tlsf.o
LIB=liblinked.a
CC=gcc

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@

.PHONY: all
all: $(OBJ)
	ar rvs $(LIB) $^

tlsf/tlsf.o:
	+cd tlsf && $(MAKE)

test: main.o all
	$(CC) main.o -L. -llinked $(CFLAGS) -g -o $@

clean:
	+cd tlsf && $(MAKE) clean
	rm -rf $(OBJ) $(LIB) main.o
	rm -f test
