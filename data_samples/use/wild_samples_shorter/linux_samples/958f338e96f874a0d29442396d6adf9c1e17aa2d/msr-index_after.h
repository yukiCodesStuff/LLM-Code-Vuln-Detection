#define MSR_IA32_ARCH_CAPABILITIES	0x0000010a
#define ARCH_CAP_RDCL_NO		(1 << 0)   /* Not susceptible to Meltdown */
#define ARCH_CAP_IBRS_ALL		(1 << 1)   /* Enhanced IBRS support */
#define ARCH_CAP_SKIP_VMENTRY_L1DFLUSH	(1 << 3)   /* Skip L1D flush on vmentry */
#define ARCH_CAP_SSB_NO			(1 << 4)   /*
						    * Not susceptible to Speculative Store Bypass
						    * attack, so no Speculative Store Bypass
						    * control required.
						    */

#define MSR_IA32_FLUSH_CMD		0x0000010b
#define L1D_FLUSH			(1 << 0)   /*
						    * Writeback and invalidate the
						    * L1 data cache.
						    */

#define MSR_IA32_BBL_CR_CTL		0x00000119
#define MSR_IA32_BBL_CR_CTL3		0x0000011e

#define MSR_IA32_SYSENTER_CS		0x00000174