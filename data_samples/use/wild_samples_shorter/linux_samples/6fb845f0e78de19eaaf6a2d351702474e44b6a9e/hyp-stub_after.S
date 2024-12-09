#include <asm/virt.h>

	.text
	.pushsection	.hyp.text, "ax"

	.align 11

ENTRY(__hyp_stub_vectors)
	ventry	el2_sync_invalid		// Synchronous EL2t