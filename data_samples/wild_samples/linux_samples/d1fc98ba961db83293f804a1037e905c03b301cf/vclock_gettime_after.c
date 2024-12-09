/*
 * Copyright 2006 Andi Kleen, SUSE Labs.
 * Subject to the GNU Public License, v.2
 *
 * Fast user context implementation of clock_gettime, gettimeofday, and time.
 *
 * 32 Bit compat layer by Stefani Seibold <stefani@seibold.net>
 *  sponsored by Rohde & Schwarz GmbH & Co. KG Munich/Germany
 *
 * The code should have no internal unresolved relocations.
 * Check with readelf after changing.
 */

#include <uapi/linux/time.h>
#include <asm/vgtod.h>
#include <asm/hpet.h>
#include <asm/vvar.h>
#include <asm/unistd.h>
#include <asm/msr.h>
#include <linux/math64.h>
#include <linux/time.h>

#define gtod (&VVAR(vsyscall_gtod_data))

extern int __vdso_clock_gettime(clockid_t clock, struct timespec *ts);
extern int __vdso_gettimeofday(struct timeval *tv, struct timezone *tz);
extern time_t __vdso_time(time_t *t);

#ifdef CONFIG_HPET_TIMER
extern u8 hpet_page
	__attribute__((visibility("hidden")));

static notrace cycle_t vread_hpet(void)
{
	return *(const volatile u32 *)(&hpet_page + HPET_COUNTER);
}