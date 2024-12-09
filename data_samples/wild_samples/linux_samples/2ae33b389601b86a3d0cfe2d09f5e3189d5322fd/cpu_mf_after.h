 *  Author(s): Hendrik Brueckner <brueckner@linux.vnet.ibm.com>
 *	       Jan Glauber <jang@linux.vnet.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (version 2 only)
 * as published by the Free Software Foundation.
 */
#ifndef _ASM_S390_CPU_MF_H
#define _ASM_S390_CPU_MF_H

#include <linux/errno.h>
#include <asm/facility.h>

#define CPU_MF_INT_SF_IAE	(1 << 31)	/* invalid entry address */
#define CPU_MF_INT_SF_ISE	(1 << 30)	/* incorrect SDBT entry */
#define CPU_MF_INT_SF_PRA	(1 << 29)	/* program request alert */
#define CPU_MF_INT_SF_SACA	(1 << 23)	/* sampler auth. change alert */
#define CPU_MF_INT_SF_LSDA	(1 << 22)	/* loss of sample data alert */
#define CPU_MF_INT_CF_CACA	(1 <<  7)	/* counter auth. change alert */
#define CPU_MF_INT_CF_LCDA	(1 <<  6)	/* loss of counter data alert */
#define CPU_MF_INT_RI_HALTED	(1 <<  5)	/* run-time instr. halted */
#define CPU_MF_INT_RI_BUF_FULL	(1 <<  4)	/* run-time instr. program
						   buffer full */

#define CPU_MF_INT_CF_MASK	(CPU_MF_INT_CF_CACA|CPU_MF_INT_CF_LCDA)
#define CPU_MF_INT_SF_MASK	(CPU_MF_INT_SF_IAE|CPU_MF_INT_SF_ISE|	\
				 CPU_MF_INT_SF_PRA|CPU_MF_INT_SF_SACA|	\
				 CPU_MF_INT_SF_LSDA)
#define CPU_MF_INT_RI_MASK	(CPU_MF_INT_RI_HALTED|CPU_MF_INT_RI_BUF_FULL)

/* CPU measurement facility support */
static inline int cpum_cf_avail(void)
{
	return MACHINE_HAS_LPP && test_facility(67);
}