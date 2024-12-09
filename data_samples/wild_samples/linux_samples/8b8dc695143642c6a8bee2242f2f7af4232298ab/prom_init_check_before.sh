#!/bin/sh
#
# Copyright © 2008 IBM Corporation
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.

# This script checks prom_init.o to see what external symbols it
# is using, if it finds symbols not in the whitelist it returns
# an error. The point of this is to discourage people from
# intentionally or accidentally adding new code to prom_init.c
# which has side effects on other parts of the kernel.

# If you really need to reference something from prom_init.o add
# it to the list below:

grep "^CONFIG_KASAN=y$" .config >/dev/null
if [ $? -eq 0 ]
then
	MEM_FUNCS="__memcpy __memset"
else
	MEM_FUNCS="memcpy memset"
fi

WHITELIST="add_reloc_offset __bss_start __bss_stop copy_and_flush
_end enter_prom $MEM_FUNCS reloc_offset __secondary_hold
__secondary_hold_acknowledge __secondary_hold_spinloop __start
logo_linux_clut224
reloc_got2 kernstart_addr memstart_addr linux_banner _stext
__prom_init_toc_start __prom_init_toc_end btext_setup_display TOC."

NM="$1"
OBJ="$2"

ERROR=0

function check_section()
{
    file=$1
    section=$2
    size=$(objdump -h -j $section $file 2>/dev/null | awk "\$2 == \"$section\" {print \$3}")
    size=${size:-0}