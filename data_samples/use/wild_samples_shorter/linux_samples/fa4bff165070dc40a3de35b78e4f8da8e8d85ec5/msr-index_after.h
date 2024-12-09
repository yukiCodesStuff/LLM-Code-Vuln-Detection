#ifndef _ASM_X86_MSR_INDEX_H
#define _ASM_X86_MSR_INDEX_H

#include <linux/bits.h>

/*
 * CPU model specific register (MSR) numbers.
 *
 * Do not add new entries to this file unless the definitions are shared
/* Intel MSRs. Some also available on other CPUs */

#define MSR_IA32_SPEC_CTRL		0x00000048 /* Speculation Control */
#define SPEC_CTRL_IBRS			BIT(0)	   /* Indirect Branch Restricted Speculation */
#define SPEC_CTRL_STIBP_SHIFT		1	   /* Single Thread Indirect Branch Predictor (STIBP) bit */
#define SPEC_CTRL_STIBP			BIT(SPEC_CTRL_STIBP_SHIFT)	/* STIBP mask */
#define SPEC_CTRL_SSBD_SHIFT		2	   /* Speculative Store Bypass Disable bit */
#define SPEC_CTRL_SSBD			BIT(SPEC_CTRL_SSBD_SHIFT)	/* Speculative Store Bypass Disable */

#define MSR_IA32_PRED_CMD		0x00000049 /* Prediction Command */
#define PRED_CMD_IBPB			BIT(0)	   /* Indirect Branch Prediction Barrier */

#define MSR_PPIN_CTL			0x0000004e
#define MSR_PPIN			0x0000004f

#define MSR_MTRRcap			0x000000fe

#define MSR_IA32_ARCH_CAPABILITIES	0x0000010a
#define ARCH_CAP_RDCL_NO		BIT(0)	/* Not susceptible to Meltdown */
#define ARCH_CAP_IBRS_ALL		BIT(1)	/* Enhanced IBRS support */
#define ARCH_CAP_SKIP_VMENTRY_L1DFLUSH	BIT(3)	/* Skip L1D flush on vmentry */
#define ARCH_CAP_SSB_NO			BIT(4)	/*
						 * Not susceptible to Speculative Store Bypass
						 * attack, so no Speculative Store Bypass
						 * control required.
						 */
#define ARCH_CAP_MDS_NO			BIT(5)   /*
						  * Not susceptible to
						  * Microarchitectural Data
						  * Sampling (MDS) vulnerabilities.
						  */

#define MSR_IA32_FLUSH_CMD		0x0000010b
#define L1D_FLUSH			BIT(0)	/*
						 * Writeback and invalidate the
						 * L1 data cache.
						 */

#define MSR_IA32_BBL_CR_CTL		0x00000119
#define MSR_IA32_BBL_CR_CTL3		0x0000011e
