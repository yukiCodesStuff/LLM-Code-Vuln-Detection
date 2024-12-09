#define MSR_IA32_ARCH_CAPABILITIES	0x0000010a
#define ARCH_CAP_RDCL_NO		(1 << 0)   /* Not susceptible to Meltdown */
#define ARCH_CAP_IBRS_ALL		(1 << 1)   /* Enhanced IBRS support */
#define ARCH_CAP_SSB_NO			(1 << 4)   /*
						    * Not susceptible to Speculative Store Bypass
						    * attack, so no Speculative Store Bypass
						    * control required.
						    */

#define MSR_IA32_BBL_CR_CTL		0x00000119
#define MSR_IA32_BBL_CR_CTL3		0x0000011e

#define MSR_IA32_SYSENTER_CS		0x00000174