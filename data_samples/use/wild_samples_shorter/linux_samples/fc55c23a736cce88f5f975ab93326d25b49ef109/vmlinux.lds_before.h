#define ARM_MMU_DISCARD(x)	x
#endif

#define PROC_INFO							\
		. = ALIGN(4);						\
		__proc_info_begin = .;					\
		*(.proc.info.init)					\
 * only thing that matters is their relative offsets
 */
#define ARM_VECTORS							\
	__vectors_start = .;						\
	.vectors 0xffff0000 : AT(__vectors_start) {			\
		*(.vectors)						\
	}								\
	. = __vectors_start + SIZEOF(.vectors);				\
	__vectors_end = .;						\
									\
	__stubs_start = .;						\
	.stubs ADDR(.vectors) + 0x1000 : AT(__stubs_start) {		\
		*(.stubs)						\
	}								\
	. = __stubs_start + SIZEOF(.stubs);				\
	__stubs_end = .;						\
									\
	PROVIDE(vector_fiq_offset = vector_fiq - ADDR(.vectors));

#define ARM_TCM								\