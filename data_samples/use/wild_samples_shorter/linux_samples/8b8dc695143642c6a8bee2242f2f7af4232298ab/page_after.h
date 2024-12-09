#endif /* __ASSEMBLY__ */
#include <asm/slice.h>

/*
 * Allow 30-bit DMA for very limited Broadcom wifi chips on many powerbooks.
 */
#ifdef CONFIG_PPC32
#define ARCH_ZONE_DMA_BITS 30
#else
#define ARCH_ZONE_DMA_BITS 31
#endif

#endif /* _ASM_POWERPC_PAGE_H */