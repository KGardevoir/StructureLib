CC=gcc
RL_C=ragel
PMCCABE=pmccabe
CFLAGS=-I.. -g -Wall -O2 -Wmissing-prototypes -Wmissing-declarations
LDFLAGS=-L.. -L. -llinked

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


all: token_parser.o shunting_yard.o
	$(CC) $^ -g $(LDFLAGS) -o parser

token_parser: token_parser.o
	$(CC) $^ $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm *.o
	rm -f token_parser parser
