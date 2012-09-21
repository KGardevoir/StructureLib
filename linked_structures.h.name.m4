divert(-1)dnl
include(c_defs.m4)
# Setup Section
ifdef(-{BASE_NAME}-,,-{define(-{BASE_NAME}-, -{linkedList}-)}-)
ifdef(-{H_NAME}-,,dnl ifndef
	-{ifdef(-{SLL}-,
		-{ifdef(-{DCLL}-, define(-{H_NAME}-,-{linked_structures.h}-), define(-{H_NAME}-,-{linked_list_sll.h}-))}-,
		-{ifdef(-{DCLL}-, define(-{H_NAME}-,-{linked_list_dcll.h}-))}-
	)}-
)

ifdef(-{TSPEC_NAME}-,,-{define(-{TSPEC_NAME}-, -{tspec}-)}-)

ifdef(-{SLL}-,-{
	ifdef(-{SLL_PREFIX}-,,-{
		ifdef(-{DCLL}-,
			-{define(-{SLL_PREFIX}-, -{single_}-)}-,
			-{define(-{SLL_PREFIX}-, -{}-)}-
		)}-
	)
	define(-{SLL_NAME}-, SLL_PREFIX-{}-BASE_NAME)
}-)
ifdef(-{DCLL}-,-{
	ifdef(-{DCLL_PREFIX}-,,-{
		ifdef(-{SLL}-,
			-{define(-{DCLL_PREFIX}-, -{double_}-)}-,
			-{define(-{DCLL_PREFIX}-, -{}-)}-
		)}-
	)
	define(-{DCLL_NAME}-, DCLL_PREFIX-{}-BASE_NAME)
}-)
divert(0)dnl
