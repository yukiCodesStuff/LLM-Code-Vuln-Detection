/*
 * Copyright (C) 2012 Calxeda, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This driver provides the clk notifier callbacks that are used when
 * the cpufreq-cpu0 driver changes to frequency to alert the highbank
 * EnergyCore Management Engine (ECME) about the need to change
 * voltage. The ECME interfaces with the actual voltage regulators.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/mailbox.h>
#include <linux/platform_device.h>

#define HB_CPUFREQ_CHANGE_NOTE	0x80000001
#define HB_CPUFREQ_IPC_LEN	7
#define HB_CPUFREQ_VOLT_RETRIES	15

static int hb_voltage_change(unsigned int freq)
{
	int i;
	u32 msg[HB_CPUFREQ_IPC_LEN];

	msg[0] = HB_CPUFREQ_CHANGE_NOTE;
	msg[1] = freq / 1000000;
	for (i = 2; i < HB_CPUFREQ_IPC_LEN; i++)
		msg[i] = 0;

	return pl320_ipc_transmit(msg);
}