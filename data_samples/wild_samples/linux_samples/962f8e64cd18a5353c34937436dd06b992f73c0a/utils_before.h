/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2013, Michael Ellerman, IBM Corp.
 */

#ifndef _SELFTESTS_POWERPC_UTILS_H
#define _SELFTESTS_POWERPC_UTILS_H

#define __cacheline_aligned __attribute__((aligned(128)))

#include <stdint.h>
#include <stdbool.h>
#include <linux/auxvec.h>
#include <linux/perf_event.h>
#include <asm/cputable.h>
#include "reg.h"

/* Avoid headaches with PRI?64 - just use %ll? always */
typedef unsigned long long u64;
typedef   signed long long s64;

/* Just for familiarity */
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

void test_harness_set_timeout(uint64_t time);
int test_harness(int (test_function)(void), char *name);

int read_auxv(char *buf, ssize_t buf_size);
void *find_auxv_entry(int type, char *auxv);
void *get_auxv_entry(int type);

int pick_online_cpu(void);

int read_debugfs_file(char *debugfs_file, int *result);
int write_debugfs_file(char *debugfs_file, int result);
int read_sysfs_file(char *debugfs_file, char *result, size_t result_size);
int perf_event_open_counter(unsigned int type,
			    unsigned long config, int group_fd);
int perf_event_enable(int fd);
int perf_event_disable(int fd);
int perf_event_reset(int fd);

#if !defined(__GLIBC_PREREQ) || !__GLIBC_PREREQ(2, 30)
#include <unistd.h>
#include <sys/syscall.h>

static inline pid_t gettid(void)
{
	return syscall(SYS_gettid);
}