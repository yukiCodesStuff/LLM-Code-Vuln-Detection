#define SPRN_HSRR0	0x13A	/* Hypervisor Save/Restore 0 */
#define SPRN_HSRR1	0x13B	/* Hypervisor Save/Restore 1 */
#define SPRN_FSCR	0x099	/* Facility Status & Control Register */
#define FSCR_TAR	(1<<8)	/* Enable Target Adress Register */
#define SPRN_TAR	0x32f	/* Target Address Register */
#define SPRN_LPCR	0x13E	/* LPAR Control Register */
#define   LPCR_VPM0	(1ul << (63-0))
#define   LPCR_VPM1	(1ul << (63-1))