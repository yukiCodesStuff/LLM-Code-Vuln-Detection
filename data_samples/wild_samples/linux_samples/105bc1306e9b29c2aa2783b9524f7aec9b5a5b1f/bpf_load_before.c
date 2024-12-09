// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <linux/bpf.h>
#include <linux/filter.h>
#include <linux/perf_event.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <poll.h>
#include <ctype.h>
#include <assert.h>
#include <bpf/bpf.h>
#include "bpf_load.h"
#include "perf-sys.h"

#define DEBUGFS "/sys/kernel/debug/tracing/"

static char license[128];
static int kern_version;
static bool processed_sec[128];
char bpf_log_buf[BPF_LOG_BUF_SIZE];
int map_fd[MAX_MAPS];
int prog_fd[MAX_PROGS];
int event_fd[MAX_PROGS];
int prog_cnt;
int prog_array_fd = -1;

struct bpf_map_data map_data[MAX_MAPS];
int map_data_count = 0;

static int populate_prog_array(const char *event, int prog_fd)
{
	int ind = atoi(event), err;

	err = bpf_map_update_elem(prog_array_fd, &ind, &prog_fd, BPF_ANY);
	if (err < 0) {
		printf("failed to store prog_fd in prog_array\n");
		return -1;
	}
	return 0;
}