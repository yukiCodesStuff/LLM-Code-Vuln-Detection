/*
 *  linux/arch/m68knommu/kernel/setup.c
 *
 *  Copyright (C) 1999-2007  Greg Ungerer (gerg@snapgear.com)
 *  Copyright (C) 1998,1999  D. Jeff Dionne <jeff@uClinux.org>
 *  Copyleft  ()) 2000       James D. Schettine {james@telos-systems.com}
 *  Copyright (C) 1998       Kenneth Albanowski <kjahds@kjahds.com>
 *  Copyright (C) 1995       Hamish Macdonald
 *  Copyright (C) 2000       Lineo Inc. (www.lineo.com)
 *  Copyright (C) 2001 	     Lineo, Inc. <www.lineo.com>
 *
 *  68VZ328 Fixes/support    Evan Stawnyczy <e@lineo.ca>
 */

/*
 * This file handles the architecture-dependent parts of system setup
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fb.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/console.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/bootmem.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/root_dev.h>
#include <linux/rtc.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/machdep.h>
#include <asm/pgtable.h>
#include <asm/sections.h>

unsigned long memory_start;
unsigned long memory_end;

EXPORT_SYMBOL(memory_start);
EXPORT_SYMBOL(memory_end);

char __initdata command_line[COMMAND_LINE_SIZE];

/* machine dependent timer functions */
void (*mach_sched_init)(irq_handler_t handler) __initdata = NULL;
int (*mach_set_clock_mmss)(unsigned long);
int (*mach_hwclk) (int, struct rtc_time*);

/* machine dependent reboot functions */
void (*mach_reset)(void);
void (*mach_halt)(void);
void (*mach_power_off)(void);

#ifdef CONFIG_M68328
#define CPU_NAME	"MC68328"
#endif
#ifdef CONFIG_M68EZ328
#define CPU_NAME	"MC68EZ328"
#endif
#ifdef CONFIG_M68VZ328
#define CPU_NAME	"MC68VZ328"
#endif
#ifdef CONFIG_M68360
#define CPU_NAME	"MC68360"
#endif
#ifndef CPU_NAME
#define	CPU_NAME	"UNKNOWN"
#endif

/*
 * Different cores have different instruction execution timings.
 * The old/traditional 68000 cores are basically all the same, at 16.
 * The ColdFire cores vary a little, their values are defined in their
 * headers. We default to the standard 68000 value here.
 */
#ifndef CPU_INSTR_PER_JIFFY
#define	CPU_INSTR_PER_JIFFY	16
#endif

#if defined(CONFIG_UBOOT)
/*
 * parse_uboot_commandline
 *
 * Copies u-boot commandline arguments and store them in the proper linux
 * variables.
 *
 * Assumes:
 *	_init_sp global contains the address in the stack pointer when the
 *	kernel starts (see head.S::_start)
 *
 *	U-Boot calling convention:
 *	(*kernel) (kbd, initrd_start, initrd_end, cmd_start, cmd_end);
 *
 *	_init_sp can be parsed as such
 *
 *	_init_sp+00 = u-boot cmd after jsr into kernel (skip)
 *	_init_sp+04 = &kernel board_info (residual data)
 *	_init_sp+08 = &initrd_start
 *	_init_sp+12 = &initrd_end
 *	_init_sp+16 = &cmd_start
 *	_init_sp+20 = &cmd_end
 *
 *	This also assumes that the memory locations pointed to are still
 *	unmodified. U-boot places them near the end of external SDRAM.
 *
 * Argument(s):
 *	commandp = the linux commandline arg container to fill.
 *	size     = the sizeof commandp.
 *
 * Returns:
 */
void parse_uboot_commandline(char *commandp, int size)
{
	extern unsigned long _init_sp;
	unsigned long *sp;
	unsigned long uboot_kbd;
	unsigned long uboot_initrd_start, uboot_initrd_end;
	unsigned long uboot_cmd_start, uboot_cmd_end;


	sp = (unsigned long *)_init_sp;
	uboot_kbd = sp[1];
	uboot_initrd_start = sp[2];
	uboot_initrd_end = sp[3];
	uboot_cmd_start = sp[4];
	uboot_cmd_end = sp[5];

	if (uboot_cmd_start && uboot_cmd_end)
		strncpy(commandp, (const char *)uboot_cmd_start, size);
#if defined(CONFIG_BLK_DEV_INITRD)
	if (uboot_initrd_start && uboot_initrd_end &&
		(uboot_initrd_end > uboot_initrd_start)) {
		initrd_start = uboot_initrd_start;
		initrd_end = uboot_initrd_end;
		ROOT_DEV = Root_RAM0;
		printk(KERN_INFO "initrd at 0x%lx:0x%lx\n",
			initrd_start, initrd_end);
	}
#endif /* if defined(CONFIG_BLK_DEV_INITRD) */
}