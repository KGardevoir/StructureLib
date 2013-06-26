SRC=dlist.c slist.c splaytree.c btree.c graph.c htable.c transform.c
CFLAGS:=-g -Wall -Wno-pointer-sign -O2 $(CFLAGS) -Wmissing-prototypes -Wmissing-declarations
OBJ=$(addsuffix .o, $(basename $(SRC)))
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

test: main.o all
	$(CC) main.o -L. -llinked -lm $(CFLAGS) -g -o $@

clean:
	rm -rf $(OBJ) $(LIB) main.o
	rm -f test
	rm -f .depend

.PHONY: depend
depend:
	@rm -f $(DEPENDS)
	@echo "# MAKEDEPENDS" > $(DEPENDS)
	@for file in $(SRC) ; do \
		$(CC) -MM -MG $(CFLAGS) $$file >> $(DEPENDS) ; \
	done

sinclude $(DEPENDS)
