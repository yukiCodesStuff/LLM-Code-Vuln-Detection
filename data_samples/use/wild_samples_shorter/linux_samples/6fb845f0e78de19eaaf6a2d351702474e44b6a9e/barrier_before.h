#define __smp_mb__before_atomic()	__smp_mb__before_llsc()
#define __smp_mb__after_atomic()	smp_llsc_mb()

#include <asm-generic/barrier.h>

#endif /* __ASM_BARRIER_H */