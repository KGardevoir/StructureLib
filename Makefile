SRC=dlist.c slist.c splaytree.c btree.c graph.c htable.c transform.c
CFLAGS:=-g -Wall -Wno-pointer-sign -O2 $(CFLAGS) -Wmissing-prototypes -Wmissing-declarations
OBJ=$(addsuffix .o, $(basename $(SRC)))
LIB=liblinked.a
CC=gcc
PMCCABE=pmccabe

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@
	-$(PMCCABE) $< | sort -nr | awk '$$1 > 8 { print $$0 }'

.PHONY: all
all: $(OBJ)
	ar rvs $(LIB) $^

test: main.o all
	$(CC) main.o -L. -llinked -lm $(CFLAGS) -g -o $@

clean:
	rm -rf $(OBJ) $(LIB) main.o
	rm -f test
