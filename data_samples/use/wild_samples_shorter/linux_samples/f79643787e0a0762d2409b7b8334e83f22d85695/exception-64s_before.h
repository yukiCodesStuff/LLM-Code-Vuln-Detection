	nop;								\
	nop

/*
 * r10 must be free to use, r13 must be paca
 */
#define INTERRUPT_TO_KERNEL						\
	STF_ENTRY_BARRIER_SLOT

/*
 * Macros for annotating the expected destination of (h)rfid
 *