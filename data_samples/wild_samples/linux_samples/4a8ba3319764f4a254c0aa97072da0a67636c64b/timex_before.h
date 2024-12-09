{
	S390_lowcore.clock_comparator = comp;
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

static inline unsigned long long get_tod_clock_fast(void)
{
#ifdef CONFIG_HAVE_MARCH_Z9_109_FEATURES
	unsigned long long clk;

	asm volatile("stckf %0" : "=Q" (clk) : : "cc");
	return clk;
#else
	return get_tod_clock();
#endif
}

static inline cycles_t get_cycles(void)
{
	return (cycles_t) get_tod_clock() >> 2;
}

int get_sync_clock(unsigned long long *clock);
void init_cpu_timer(void);
unsigned long long monotonic_clock(void);

void tod_to_timeval(__u64, struct timespec *);

static inline
void stck_to_timespec(unsigned long long stck, struct timespec *ts)
{
	tod_to_timeval(stck - TOD_UNIX_EPOCH, ts);
}

extern u64 sched_clock_base_cc;

/**
 * get_clock_monotonic - returns current time in clock rate units
 *
 * The caller must ensure that preemption is disabled.
 * The clock and sched_clock_base get changed via stop_machine.
 * Therefore preemption must be disabled when calling this
 * function, otherwise the returned value is not guaranteed to
 * be monotonic.
 */
static inline unsigned long long get_tod_clock_monotonic(void)
{
	return get_tod_clock() - sched_clock_base_cc;
}

/**
 * tod_to_ns - convert a TOD format value to nanoseconds
 * @todval: to be converted TOD format value
 * Returns: number of nanoseconds that correspond to the TOD format value
 *
 * Converting a 64 Bit TOD format value to nanoseconds means that the value
 * must be divided by 4.096. In order to achieve that we multiply with 125
 * and divide by 512:
 *
 *    ns = (todval * 125) >> 9;
 *
 * In order to avoid an overflow with the multiplication we can rewrite this.
 * With a split todval == 2^32 * th + tl (th upper 32 bits, tl lower 32 bits)
 * we end up with
 *
 *    ns = ((2^32 * th + tl) * 125 ) >> 9;
 * -> ns = (2^23 * th * 125) + ((tl * 125) >> 9);
 *
 */
static inline unsigned long long tod_to_ns(unsigned long long todval)
{
	unsigned long long ns;

	ns = ((todval >> 32) << 23) * 125;
	ns += ((todval & 0xffffffff) * 125) >> 9;
	return ns;
}

#endif