
#ifndef __ASSEMBLY__

#include <asm/nospec-branch.h>

/* Provide __cpuidle; we can't safely include <linux/cpu.h> */
#define __cpuidle __attribute__((__section__(".cpuidle.text")))

/*

static inline __cpuidle void native_safe_halt(void)
{
	mds_idle_clear_cpu_buffers();
	asm volatile("sti; hlt": : :"memory");
}

static inline __cpuidle void native_halt(void)
{
	mds_idle_clear_cpu_buffers();
	asm volatile("hlt": : :"memory");
}

#endif