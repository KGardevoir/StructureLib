.PHONY: single double all clean all_vars double_vars single_vars
ifndef OUT_H
OUT_H=linked_structures.h
endif
M4_FLAGS:=-DH_NAME=$(OUT_H) $(M4_FLAGS)
DEFAULT_SRC:=linked_slist.c linked_dlist.c

all: all_vars $(OUT_H) $(DEFAULT_SRC)
single: single_vars $(OUT_H) linked_slist.c
double: double_vars $(OUT_H) linked_dlist.c

all_vars:
	$(eval TMP_M4_FLAGS := -DSLL -DSLL_TRANSFORMS -DDCLL -DDCLL_TRANSFORMS -DDCLL_MERGE $(M4_FLAGS))
single_vars:
	$(eval TMP_M4_FLAGS := -DSLL -DSLL_TRANSFORMS $(M4_FLAGS))
double_vars:
	$(eval TMP_M4_FLAGS := -DDCLL -DDCLL_TRANSFORMS -DDCLL_MERGE $(M4_FLAGS))
%.c: %.c.m4 linked_structures.h.name.m4
	m4 $(TMP_M4_FLAGS) $< > $@
$(OUT_H): linked_structures.h.m4 linked_structures.h.name.m4
	m4 $(TMP_M4_FLAGS) $< > $@
test: all 
	gcc main.c $(DEFAULT_SRC) -o $@

clean:
	rm -f linked_structures.h linked_slist.c linked_dlist.c
	rm -f test
