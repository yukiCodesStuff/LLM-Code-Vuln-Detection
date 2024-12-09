#include <stdint.h>
#include <malloc.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"

#define CACHELINE_SIZE 128

struct perf_event_read {
	__u64 nr;
	__u64 l1d_misses;
};

static inline __u64 load(void *addr)
{
	__u64 tmp;

	asm volatile("ld %0,0(%1)" : "=r"(tmp) : "b"(addr));

	return tmp;
}

static void syscall_loop(char *p, unsigned long iterations,
			 unsigned long zero_size)
{
	for (unsigned long i = 0; i < iterations; i++) {
		for (unsigned long j = 0; j < zero_size; j += CACHELINE_SIZE)
			load(p + j);
		getppid();
	}
}

static void sigill_handler(int signr, siginfo_t *info, void *unused)
{
	static int warned = 0;
	ucontext_t *ctx = (ucontext_t *)unused;
	unsigned long *pc = &UCONTEXT_NIA(ctx);

	/* mtspr 3,RS to check for move to DSCR below */
	if ((*((unsigned int *)*pc) & 0xfc1fffff) == 0x7c0303a6) {
		if (!warned++)
			printf("WARNING: Skipping over dscr setup. Consider running 'ppc64_cpu --dscr=1' manually.\n");
		*pc += 4;
	} else {
		printf("SIGILL at %p\n", pc);
		abort();
	}
}

static void set_dscr(unsigned long val)
{
	static int init = 0;
	struct sigaction sa;

	if (!init) {
		memset(&sa, 0, sizeof(sa));
		sa.sa_sigaction = sigill_handler;
		sa.sa_flags = SA_SIGINFO;
		if (sigaction(SIGILL, &sa, NULL))
			perror("sigill_handler");
		init = 1;
	}

	asm volatile("mtspr %1,%0" : : "r" (val), "i" (SPRN_DSCR));
}

int rfi_flush_test(void)
{
	char *p;
	__u64 l1d_misses_total = 0;
	unsigned long iterations = 100000, zero_size = 24 * 1024;
	unsigned long l1d_misses_expected;
	int rfi_flush_org, rfi_flush;

	SKIP_IF(geteuid() != 0);

	// The PMU event we use only works on Power7 or later
	SKIP_IF(!have_hwcap(PPC_FEATURE_ARCH_2_06));

	if (read_debugfs_file("powerpc/rfi_flush", &rfi_flush_org)) {
		perror("Unable to read powerpc/rfi_flush debugfs file");
		SKIP_IF(1);
	}

	rfi_flush = rfi_flush_org;

	fd = perf_event_open_counter(PERF_TYPE_RAW, /* L1d miss */ 0x400f0, -1);
	FAIL_IF(fd < 0);


	FAIL_IF(perf_event_enable(fd));

	set_dscr(1);

	iter = repetitions;

		       repetitions * l1d_misses_expected / 2,
		       passes, repetitions);

	if (rfi_flush == rfi_flush_org) {
		rfi_flush = !rfi_flush_org;
		if (write_debugfs_file("powerpc/rfi_flush", rfi_flush) < 0) {
			perror("error writing to powerpc/rfi_flush debugfs file");
			return 1;
		}

	set_dscr(0);

	if (write_debugfs_file("powerpc/rfi_flush", rfi_flush_org) < 0) {
		perror("unable to restore original value of powerpc/rfi_flush debugfs file");
		return 1;
	}

	return rc;
}

int main(int argc, char *argv[])