#define smp_mb__before_clear_bit()	smp_mb()
#define smp_mb__after_clear_bit()	smp_mb()

/* Macro for generating the ***_bits() functions */
#define DEFINE_BITOP(fn, op, prefix, postfix)	\
static __inline__ void fn(unsigned long mask,	\
		volatile unsigned long *_p)	\