/*
 * sampleip: sample instruction pointer and frequency count in a BPF map.
 *
 * Copyright 2016 Netflix, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <linux/perf_event.h>
#include <linux/ptrace.h>
#include <linux/bpf.h>
#include <sys/ioctl.h>
#include "libbpf.h"
#include "bpf_load.h"
#include "perf-sys.h"
#include "trace_helpers.h"

#define DEFAULT_FREQ	99
#define DEFAULT_SECS	5
#define MAX_IPS		8192
#define PAGE_OFFSET	0xffff880000000000

static int nr_cpus;

static void usage(void)
{
	printf("USAGE: sampleip [-F freq] [duration]\n");
	printf("       -F freq    # sample frequency (Hertz), default 99\n");
	printf("       duration   # sampling duration (seconds), default 5\n");
}