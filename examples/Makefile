CC=gcc
RL_C=ragel
CFLAGS=-I.. -g -Wall -O2
LDFLAGS=-L.. -L. -llinked

%.c: %.rl
	$(RL_C) -G2 $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


all: token_parser.o shunting_yard.o
	$(CC) $^ -g $(LDFLAGS) -o parser

token_parser: token_parser.o
	$(CC) $^ $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm *.o
	rm -f token_parser parser
