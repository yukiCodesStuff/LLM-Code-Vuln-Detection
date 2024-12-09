
#ifndef __ASSEMBLY__

/* Provide __cpuidle; we can't safely include <linux/cpu.h> */
#define __cpuidle __attribute__((__section__(".cpuidle.text")))

/*

static inline __cpuidle void native_safe_halt(void)
{
	asm volatile("sti; hlt": : :"memory");
}

static inline __cpuidle void native_halt(void)
{
	asm volatile("hlt": : :"memory");
}

#endif