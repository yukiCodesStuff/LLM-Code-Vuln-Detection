#include <linux/bpf.h>
#include "bpf.h"
#include "libbpf.h"
#include <errno.h>

/*
 * When building perf, unistd.h is overridden. __NR_bpf is
 * required to be defined explicitly.
 */
	return sys_bpf(BPF_RAW_TRACEPOINT_OPEN, &attr, sizeof(attr));
}

int bpf_load_btf(void *btf, __u32 btf_size, char *log_buf, __u32 log_buf_size,
		 bool do_log)
{
	union bpf_attr attr = {};