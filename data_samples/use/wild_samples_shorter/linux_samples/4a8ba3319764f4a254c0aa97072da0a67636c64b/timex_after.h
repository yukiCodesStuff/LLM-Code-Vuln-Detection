	set_clock_comparator(S390_lowcore.clock_comparator);
}

#define CLOCK_TICK_RATE		1193180 /* Underlying HZ */
#define STORE_CLOCK_EXT_SIZE	16	/* stcke writes 16 bytes */

typedef unsigned long long cycles_t;

static inline void get_tod_clock_ext(char *clk)
{
	typedef struct { char _[STORE_CLOCK_EXT_SIZE]; } addrtype;

	asm volatile("stcke %0" : "=Q" (*(addrtype *) clk) : : "cc");
}

static inline unsigned long long get_tod_clock(void)
{
	unsigned char clk[STORE_CLOCK_EXT_SIZE];

	get_tod_clock_ext(clk);
	return *((unsigned long long *)&clk[1]);
}
