// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright IBM Corp. 2007
 *
 * Authors: Hollis Blanchard <hollisb@us.ibm.com>
 *          Christian Ehrhardt <ehrhardt@linux.vnet.ibm.com>
 */

#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kvm_host.h>
#include <linux/vmalloc.h>
#include <linux/hrtimer.h>
#include <linux/sched/signal.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/irqbypass.h>
#include <linux/kvm_irqfd.h>
#include <asm/cputable.h>
#include <linux/uaccess.h>
#include <asm/kvm_ppc.h>
#include <asm/cputhreads.h>
#include <asm/irqflags.h>
#include <asm/iommu.h>
#include <asm/switch_to.h>
#include <asm/xive.h>
#ifdef CONFIG_PPC_PSERIES
#include <asm/hvcall.h>
#include <asm/plpar_wrappers.h>
#endif
#include <asm/ultravisor.h>
#include <asm/kvm_host.h>

#include "timing.h"
#include "irq.h"
#include "../mm/mmu_decl.h"

#define CREATE_TRACE_POINTS
#include "trace.h"

struct kvmppc_ops *kvmppc_hv_ops;
EXPORT_SYMBOL_GPL(kvmppc_hv_ops);
struct kvmppc_ops *kvmppc_pr_ops;
EXPORT_SYMBOL_GPL(kvmppc_pr_ops);


int kvm_arch_vcpu_runnable(struct kvm_vcpu *v)
{
	return !!(v->arch.pending_exceptions) || kvm_request_pending(v);
}
	case KVM_PPC_GET_CPU_CHAR: {
		struct kvm_ppc_cpu_char cpuchar;

		r = kvmppc_get_cpu_char(&cpuchar);
		if (r >= 0 && copy_to_user(argp, &cpuchar, sizeof(cpuchar)))
			r = -EFAULT;
		break;
	}
	case KVM_PPC_SVM_OFF: {
		struct kvm *kvm = filp->private_data;

		r = 0;
		if (!kvm->arch.kvm_ops->svm_off)
			goto out;

		r = kvm->arch.kvm_ops->svm_off(kvm);
		break;
	}
	default: {
		struct kvm *kvm = filp->private_data;
		r = kvm->arch.kvm_ops->arch_vm_ioctl(filp, ioctl, arg);
	}
#else /* CONFIG_PPC_BOOK3S_64 */
	default:
		r = -ENOTTY;
#endif
	}
out:
	return r;
}

static unsigned long lpid_inuse[BITS_TO_LONGS(KVMPPC_NR_LPIDS)];
static unsigned long nr_lpids;

long kvmppc_alloc_lpid(void)
{
	long lpid;

	do {
		lpid = find_first_zero_bit(lpid_inuse, KVMPPC_NR_LPIDS);
		if (lpid >= nr_lpids) {
			pr_err("%s: No LPIDs free\n", __func__);
			return -ENOMEM;
		}
	} while (test_and_set_bit(lpid, lpid_inuse));

	return lpid;
}
EXPORT_SYMBOL_GPL(kvmppc_alloc_lpid);

void kvmppc_claim_lpid(long lpid)
{
	set_bit(lpid, lpid_inuse);
}
EXPORT_SYMBOL_GPL(kvmppc_claim_lpid);

void kvmppc_free_lpid(long lpid)
{
	clear_bit(lpid, lpid_inuse);
}
EXPORT_SYMBOL_GPL(kvmppc_free_lpid);

void kvmppc_init_lpid(unsigned long nr_lpids_param)
{
	nr_lpids = min_t(unsigned long, KVMPPC_NR_LPIDS, nr_lpids_param);
	memset(lpid_inuse, 0, sizeof(lpid_inuse));
}
EXPORT_SYMBOL_GPL(kvmppc_init_lpid);

int kvm_arch_init(void *opaque)
{
	return 0;
}

EXPORT_TRACEPOINT_SYMBOL_GPL(kvm_ppc_instr);