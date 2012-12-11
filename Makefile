SRC=linked_dlist.c linked_slist.c
OBJ=$(addsuffix .o, $(basename $(SRC)))
LIB=liblinked.a
CC=gcc

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJ)
	ar rvs $(LIB) $^

clean:
	rm -rf $(OBJ) $(LIB)
