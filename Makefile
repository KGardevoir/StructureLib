SRC=dlist.c udlist.c slist.c btree.c splaytree.c scapegoat.c graph.c graph_tree.c htable.c transform.c city.c
CFLAGS:=-g -Wall -Wno-pointer-sign -std=c99 $(CFLAGS) -Wmissing-prototypes -Wmissing-declarations
OBJ=$(addsuffix .o, $(basename $(SRC)))
SRC_TESTS:=$(wildcard main_*.c) aLong.c main.c
OBJ_TESTS=$(addsuffix .o, $(basename $(SRC_TESTS)))
LIB=liblinked.a
CC=gcc
PMCCABE=pmccabe
DEPENDS=.depend

%.o: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@
	-$(PMCCABE) $< | sort -nr | awk '$$1 > 8 { print $$0 }'
	@-awk -v LENGTH=$(shell cat $< | wc -l | wc -m) '\
		BEGIN { len = LENGTH-1; fmt = "%s:%" len "d - %s: %s\n"; } \
		/\/\/\s*TODO/  { arg = "TODO ";  printf fmt, FILENAME, FNR, arg, substr($$0,index($$0,arg)+length(arg),length()) } \
		/\/\/\s*FIXME/ { arg = "FIXME "; printf fmt, FILENAME, FNR, arg, substr($$0,index($$0,arg)+length(arg),length()) } \
	' $<

.PHONY: all
all: $(OBJ)
	ar rvs $(LIB) $^

test: $(OBJ_TESTS) all
	$(CC) $(OBJ_TESTS) -L. -llinked -lm $(CFLAGS) -g -o $@

clean:
	rm -rf $(OBJ) $(LIB) $(OBJ_TESTS)
	rm -f test
	rm -f .depend

.PHONY: depend
depend:
	@rm -f $(DEPENDS)
	@echo "# MAKEDEPENDS" > $(DEPENDS)
	@for file in $(SRC) $(SRC_TESTS); do \
		$(CC) -MM -MG $(CFLAGS) $$file >> $(DEPENDS) ; \
	done

sinclude $(DEPENDS)
