#ifndef _ASM_S390_CPU_MF_H
#define _ASM_S390_CPU_MF_H

#include <asm/facility.h>

#define CPU_MF_INT_SF_IAE	(1 << 31)	/* invalid entry address */
#define CPU_MF_INT_SF_ISE	(1 << 30)	/* incorrect SDBT entry */