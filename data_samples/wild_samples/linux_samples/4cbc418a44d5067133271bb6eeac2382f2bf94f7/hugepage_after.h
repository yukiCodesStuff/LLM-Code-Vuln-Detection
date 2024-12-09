/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2013-15 Synopsys, Inc. (www.synopsys.com)
 */


#ifndef _ASM_ARC_HUGEPAGE_H
#define _ASM_ARC_HUGEPAGE_H

#include <linux/types.h>
#include <asm-generic/pgtable-nopmd.h>

static inline pte_t pmd_pte(pmd_t pmd)
{
	return __pte(pmd_val(pmd));
}