						 * attack, so no Speculative Store Bypass
						 * control required.
						 */

#define MSR_IA32_FLUSH_CMD		0x0000010b
#define L1D_FLUSH			BIT(0)	/*
						 * Writeback and invalidate the