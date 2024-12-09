/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_SPECIAL_INSNS_H
#define _ASM_X86_SPECIAL_INSNS_H


#ifdef __KERNEL__

#include <asm/nops.h>
#include <asm/processor-flags.h>
#include <linux/jump_label.h>

/*
 * Volatile isn't enough to prevent the compiler from reordering the
 * read/write functions for the control registers and messing everything up.
 * A memory clobber would solve the problem, but would prevent reordering of
 * all loads stores around it, which can hurt performance. Solution is to
 * use a variable and mimic reads and writes to it to enforce serialization
 */
extern unsigned long __force_order;

/* Starts false and gets enabled once CPU feature detection is done. */
DECLARE_STATIC_KEY_FALSE(cr_pinning);
extern unsigned long cr4_pinned_bits;

static inline unsigned long native_read_cr0(void)
{
	unsigned long val;
	asm volatile("mov %%cr0,%0\n\t" : "=r" (val), "=m" (__force_order));
	return val;
}
/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_SPECIAL_INSNS_H
#define _ASM_X86_SPECIAL_INSNS_H


#ifdef __KERNEL__

#include <asm/nops.h>
#include <asm/processor-flags.h>
#include <linux/jump_label.h>

/*
 * Volatile isn't enough to prevent the compiler from reordering the
 * read/write functions for the control registers and messing everything up.
 * A memory clobber would solve the problem, but would prevent reordering of
 * all loads stores around it, which can hurt performance. Solution is to
 * use a variable and mimic reads and writes to it to enforce serialization
 */
extern unsigned long __force_order;

/* Starts false and gets enabled once CPU feature detection is done. */
DECLARE_STATIC_KEY_FALSE(cr_pinning);
extern unsigned long cr4_pinned_bits;

static inline unsigned long native_read_cr0(void)
{
	unsigned long val;
	asm volatile("mov %%cr0,%0\n\t" : "=r" (val), "=m" (__force_order));
	return val;
}
		     : "=r" (val), "=m" (__force_order) : "0" (0));
#else
	/* CR4 always exists on x86_64. */
	asm volatile("mov %%cr4,%0\n\t" : "=r" (val), "=m" (__force_order));
#endif
	return val;
}

static inline void native_write_cr4(unsigned long val)
{
	unsigned long bits_missing = 0;

set_register:
	asm volatile("mov %0,%%cr4": "+r" (val), "+m" (cr4_pinned_bits));

	if (static_branch_likely(&cr_pinning)) {
		if (unlikely((val & cr4_pinned_bits) != cr4_pinned_bits)) {
			bits_missing = ~val & cr4_pinned_bits;
			val |= bits_missing;
			goto set_register;
		}
		/* Warn after we've set the missing bits. */
		WARN_ONCE(bits_missing, "CR4 bits went missing: %lx!?\n",
			  bits_missing);
	}
}