SRC=dlist.c slist.c splaytree.c bstree.c graphs.c htable.c
CFLAGS:=-g -Wall -Wno-pointer-sign -O2 $(CFLAGS) -Wmissing-prototypes -Wmissing-declarations
OBJ=$(addsuffix .o, $(basename $(SRC))) #tlsf/tlsf.o
LIB=liblinked.a
CC=gcc
PMCCABE=pmccabe

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@
	-$(PMCCABE) $< | sort -nr | awk '$$1 > 8 { print $$0 }'

.PHONY: all
all: $(OBJ)
	ar rvs $(LIB) $^

tlsf/tlsf.o:
	+cd tlsf && $(MAKE)

test: main.o all
	$(CC) main.o -L. -llinked -lm $(CFLAGS) -g -o $@

clean:
	+cd tlsf && $(MAKE) clean
	rm -rf $(OBJ) $(LIB) main.o
	rm -f test
