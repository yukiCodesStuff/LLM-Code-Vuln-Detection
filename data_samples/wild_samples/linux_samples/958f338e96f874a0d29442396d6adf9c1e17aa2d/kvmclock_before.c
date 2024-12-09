/*  KVM paravirtual clock driver. A clocksource implementation
    Copyright (C) 2008 Glauber de Oliveira Costa, Red Hat Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <linux/clocksource.h>
#include <linux/kvm_para.h>
#include <asm/pvclock.h>
#include <asm/msr.h>
#include <asm/apic.h>
#include <linux/percpu.h>
#include <linux/hardirq.h>
#include <linux/cpuhotplug.h>
#include <linux/sched.h>
#include <linux/sched/clock.h>
#include <linux/mm.h>

#include <asm/hypervisor.h>
#include <asm/mem_encrypt.h>
#include <asm/x86_init.h>
#include <asm/reboot.h>
#include <asm/kvmclock.h>

static int kvmclock __initdata = 1;
static int kvmclock_vsyscall __initdata = 1;
static int msr_kvm_system_time __ro_after_init = MSR_KVM_SYSTEM_TIME;
static int msr_kvm_wall_clock __ro_after_init = MSR_KVM_WALL_CLOCK;
static u64 kvm_sched_clock_offset __ro_after_init;

static int __init parse_no_kvmclock(char *arg)
{
	kvmclock = 0;
	return 0;
}