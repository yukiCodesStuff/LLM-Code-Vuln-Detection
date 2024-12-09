	set_clock_comparator(S390_lowcore.clock_comparator);
}

#define CLOCK_TICK_RATE	1193180 /* Underlying HZ */

typedef unsigned long long cycles_t;

static inline void get_tod_clock_ext(char clk[16])
{
	typedef struct { char _[sizeof(clk)]; } addrtype;

	asm volatile("stcke %0" : "=Q" (*(addrtype *) clk) : : "cc");
}

static inline unsigned long long get_tod_clock(void)
{
	unsigned char clk[16];
	get_tod_clock_ext(clk);
	return *((unsigned long long *)&clk[1]);
}
