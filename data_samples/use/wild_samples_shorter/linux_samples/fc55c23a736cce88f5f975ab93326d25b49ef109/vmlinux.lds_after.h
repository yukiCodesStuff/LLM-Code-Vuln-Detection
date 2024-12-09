#define ARM_MMU_DISCARD(x)	x
#endif

/* Set start/end symbol names to the LMA for the section */
#define ARM_LMA(sym, section)						\
	sym##_start = LOADADDR(section);				\
	sym##_end = LOADADDR(section) + SIZEOF(section)

#define PROC_INFO							\
		. = ALIGN(4);						\
		__proc_info_begin = .;					\
		*(.proc.info.init)					\
 * only thing that matters is their relative offsets
 */
#define ARM_VECTORS							\
	__vectors_lma = .;						\
	OVERLAY 0xffff0000 : NOCROSSREFS AT(__vectors_lma) {		\
		.vectors {						\
			*(.vectors)					\
		}							\
		.vectors.bhb.loop8 {					\
			*(.vectors.bhb.loop8)				\
		}							\
		.vectors.bhb.bpiall {					\
			*(.vectors.bhb.bpiall)				\
		}							\
	}								\
	ARM_LMA(__vectors, .vectors);					\
	ARM_LMA(__vectors_bhb_loop8, .vectors.bhb.loop8);		\
	ARM_LMA(__vectors_bhb_bpiall, .vectors.bhb.bpiall);		\
	. = __vectors_lma + SIZEOF(.vectors) +				\
		SIZEOF(.vectors.bhb.loop8) +				\
		SIZEOF(.vectors.bhb.bpiall);				\
									\
	__stubs_lma = .;						\
	.stubs ADDR(.vectors) + 0x1000 : AT(__stubs_lma) {		\
		*(.stubs)						\
	}								\
	ARM_LMA(__stubs, .stubs);					\
	. = __stubs_lma + SIZEOF(.stubs);				\
									\
	PROVIDE(vector_fiq_offset = vector_fiq - ADDR(.vectors));

#define ARM_TCM								\