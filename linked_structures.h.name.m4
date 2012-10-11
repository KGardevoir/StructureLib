divert(-1)dnl
include(c_defs.m4)
# Setup Section
ifdef(-{H_NAME}-,-{H_NAME}-,dnl ifndef
	-{ifdef(-{SLL}-,
		ifdef(-{DCLL}-, -{define(-{H_NAME}-,-{linked_structures.h}-)}-, -{define(-{H_NAME}-,-{linked_slist.h}-)}-),
		ifdef(-{DCLL}-, -{define(-{H_NAME}-,-{linked_dlist.h}-)}-)
	)}-
)

ifdef(-{TSPEC_NAME}-,,-{define(-{TSPEC_NAME}-, -{list_tspec}-)}-)

ifdef(-{SLL}-,
	-{ifdef(-{SLL_NAME}-,,
		-{define(-{SLL_NAME}-, -{slist}-)}-
	)}-
)
ifdef(-{DCLL}-,
	-{ifdef(-{DCLL_NAME}-,,
		-{define(-{DCLL_NAME}-, -{dlist}-)}-
	)}-
)
divert(0)dnl
