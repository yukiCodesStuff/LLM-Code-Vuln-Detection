#include <stdint.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include "flush_utils.h"


int rfi_flush_test(void)
{
	char *p;
	__u64 l1d_misses_total = 0;
	unsigned long iterations = 100000, zero_size = 24 * 1024;
	unsigned long l1d_misses_expected;
	int rfi_flush_orig, rfi_flush;
	int have_entry_flush, entry_flush_orig;

	SKIP_IF(geteuid() != 0);

	// The PMU event we use only works on Power7 or later
	SKIP_IF(!have_hwcap(PPC_FEATURE_ARCH_2_06));

	if (read_debugfs_file("powerpc/rfi_flush", &rfi_flush_orig) < 0) {
		perror("Unable to read powerpc/rfi_flush debugfs file");
		SKIP_IF(1);
	}

	if (read_debugfs_file("powerpc/entry_flush", &entry_flush_orig) < 0) {
		have_entry_flush = 0;
	} else {
		have_entry_flush = 1;

		if (entry_flush_orig != 0) {
			if (write_debugfs_file("powerpc/entry_flush", 0) < 0) {
				perror("error writing to powerpc/entry_flush debugfs file");
				return 1;
			}
		}
	}

	rfi_flush = rfi_flush_orig;

	fd = perf_event_open_counter(PERF_TYPE_RAW, /* L1d miss */ 0x400f0, -1);
	FAIL_IF(fd < 0);


	FAIL_IF(perf_event_enable(fd));

	// disable L1 prefetching
	set_dscr(1);

	iter = repetitions;

		       repetitions * l1d_misses_expected / 2,
		       passes, repetitions);

	if (rfi_flush == rfi_flush_orig) {
		rfi_flush = !rfi_flush_orig;
		if (write_debugfs_file("powerpc/rfi_flush", rfi_flush) < 0) {
			perror("error writing to powerpc/rfi_flush debugfs file");
			return 1;
		}

	set_dscr(0);

	if (write_debugfs_file("powerpc/rfi_flush", rfi_flush_orig) < 0) {
		perror("unable to restore original value of powerpc/rfi_flush debugfs file");
		return 1;
	}

	if (have_entry_flush) {
		if (write_debugfs_file("powerpc/entry_flush", entry_flush_orig) < 0) {
			perror("unable to restore original value of powerpc/entry_flush "
			       "debugfs file");
			return 1;
		}
	}

	return rc;
}

int main(int argc, char *argv[])