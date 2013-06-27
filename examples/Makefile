SRC=token_parser.c shunting_yard.c
OBJ=$(addsuffix .o, $(basename $(SRC)))
CC=gcc
RL_C=ragel
PMCCABE=pmccabe
CFLAGS=-I.. -g -Wall -O2 -Wmissing-prototypes -Wmissing-declarations
LDFLAGS=-L.. -L. -llinked
DEPENDS=.depend

%.c: %.rl
	$(RL_C) -G2 $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	@-$(PMCCABE) $< | sort -nr | awk '$$1 > 8 { print $$0 }'
	@-awk -v LENGTH=$(shell cat $< | wc -l | wc -m) '\
		BEGIN { len = LENGTH-1; fmt = "%s:%" len "d - %s: %s\n"; } \
		/\/\/\s*TODO/  { arg = "TODO ";  printf fmt, FILENAME, FNR, arg, substr($$0,index($$0,arg)+length(arg),length()) } \
		/\/\/\s*FIXME/ { arg = "FIXME "; printf fmt, FILENAME, FNR, arg, substr($$0,index($$0,arg)+length(arg),length()) } \
	' $<

all: $(OBJ)
	$(CC) $^ -g $(LDFLAGS) -o parser

.PHONY: clean
clean:
	rm -f *.o
	rm -f token_parser parser
	rm -f .depend
	rm -f dlist_maker.h

.PHONY: depend
depend: $(SRC)
	@rm -f $(DEPENDS)
	@echo "# MAKEDEPENDS" > $(DEPENDS)
	@for file in $(SRC) ; do \
		$(CC) -MM -MG $(CFLAGS) $$file >> $(DEPENDS) ; \
	done

dlist_maker.h: dlist_maker.h.py
	python dlist_maker.h.py --max_args 20

sinclude $(DEPENDS)

