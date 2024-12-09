/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include <asm/tlbflush.h>
#include <asm/cputhreads.h>
#include <asm/irqflags.h>
#include <asm/iommu.h>
#include <asm/switch_to.h>
#include <asm/xive.h>

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
	switch (ext) {
#ifdef CONFIG_BOOKE
	case KVM_CAP_PPC_BOOKE_SREGS:
	case KVM_CAP_PPC_BOOKE_WATCHDOG:
	case KVM_CAP_PPC_EPR:
#else
	case KVM_CAP_PPC_SEGSTATE:
	case KVM_CAP_PPC_HIOR:
	case KVM_CAP_PPC_PAPR:
#endif
	case KVM_CAP_PPC_UNSET_IRQ:
	case KVM_CAP_PPC_IRQ_LEVEL:
	case KVM_CAP_ENABLE_CAP:
	case KVM_CAP_ENABLE_CAP_VM:
	case KVM_CAP_ONE_REG:
	case KVM_CAP_IOEVENTFD:
	case KVM_CAP_DEVICE_CTRL:
	case KVM_CAP_IMMEDIATE_EXIT:
		r = 1;
		break;
	case KVM_CAP_PPC_PAIRED_SINGLES:
	case KVM_CAP_PPC_OSI:
	case KVM_CAP_PPC_GET_PVINFO:
#if defined(CONFIG_KVM_E500V2) || defined(CONFIG_KVM_E500MC)
	case KVM_CAP_SW_TLB:
#endif
		/* We support this only for PR */
		r = !hv_enabled;
		break;
#ifdef CONFIG_KVM_MPIC
	case KVM_CAP_IRQ_MPIC:
		r = 1;
		break;
#endif

#ifdef CONFIG_PPC_BOOK3S_64
	case KVM_CAP_SPAPR_TCE:
	case KVM_CAP_SPAPR_TCE_64:
		/* fallthrough */
	case KVM_CAP_SPAPR_TCE_VFIO:
	case KVM_CAP_PPC_RTAS:
	case KVM_CAP_PPC_FIXUP_HCALL:
	case KVM_CAP_PPC_ENABLE_HCALL:
#ifdef CONFIG_KVM_XICS
	case KVM_CAP_IRQ_XICS:
#endif
		r = 1;
		break;

	case KVM_CAP_PPC_ALLOC_HTAB:
		r = hv_enabled;
		break;
#endif /* CONFIG_PPC_BOOK3S_64 */
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	case KVM_CAP_PPC_SMT:
		r = 0;
		if (kvm) {
			if (kvm->arch.emul_smt_mode > 1)
				r = kvm->arch.emul_smt_mode;
			else
				r = kvm->arch.smt_mode;
		} else if (hv_enabled) {
			if (cpu_has_feature(CPU_FTR_ARCH_300))
				r = 1;
			else
				r = threads_per_subcore;
		}
		break;
	case KVM_CAP_PPC_SMT_POSSIBLE:
		r = 1;
		if (hv_enabled) {
			if (!cpu_has_feature(CPU_FTR_ARCH_300))
				r = ((threads_per_subcore << 1) - 1);
			else
				/* P9 can emulate dbells, so allow any mode */
				r = 8 | 4 | 2 | 1;
		}
		break;
	case KVM_CAP_PPC_RMA:
		r = 0;
		break;
	case KVM_CAP_PPC_HWRNG:
		r = kvmppc_hwrng_present();
		break;
	case KVM_CAP_PPC_MMU_RADIX:
		r = !!(hv_enabled && radix_enabled());
		break;
	case KVM_CAP_PPC_MMU_HASH_V3:
		r = !!(hv_enabled && cpu_has_feature(CPU_FTR_ARCH_300));
		break;
#endif
	case KVM_CAP_SYNC_MMU:
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
		r = hv_enabled;
#elif defined(KVM_ARCH_WANT_MMU_NOTIFIER)
		r = 1;
#else
		r = 0;
#endif
		break;
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	case KVM_CAP_PPC_HTAB_FD:
		r = hv_enabled;
		break;
#endif
	case KVM_CAP_NR_VCPUS:
		/*
		 * Recommending a number of CPUs is somewhat arbitrary; we
		 * return the number of present CPUs for -HV (since a host
		 * will have secondary threads "offline"), and for other KVM
		 * implementations just count online CPUs.
		 */
		if (hv_enabled)
			r = num_present_cpus();
		else
			r = num_online_cpus();
		break;
	case KVM_CAP_NR_MEMSLOTS:
		r = KVM_USER_MEM_SLOTS;
		break;
	case KVM_CAP_MAX_VCPUS:
		r = KVM_MAX_VCPUS;
		break;
#ifdef CONFIG_PPC_BOOK3S_64
	case KVM_CAP_PPC_GET_SMMU_INFO:
		r = 1;
		break;
	case KVM_CAP_SPAPR_MULTITCE:
		r = 1;
		break;
	case KVM_CAP_SPAPR_RESIZE_HPT:
		/* Disable this on POWER9 until code handles new HPTE format */
		r = !!hv_enabled && !cpu_has_feature(CPU_FTR_ARCH_300);
		break;
#endif
#ifdef CONFIG_KVM_BOOK3S_HV_POSSIBLE
	case KVM_CAP_PPC_FWNMI:
		r = hv_enabled;
		break;
#endif
	case KVM_CAP_PPC_HTM:
		r = hv_enabled &&
		    (cur_cpu_spec->cpu_user_features2 & PPC_FEATURE2_HTM_COMP);
		break;
	default:
		r = 0;
		break;
	}
	case KVM_CAP_PPC_SMT: {
		unsigned long mode = cap->args[0];
		unsigned long flags = cap->args[1];

		r = -EINVAL;
		if (kvm->arch.kvm_ops->set_smt_mode)
			r = kvm->arch.kvm_ops->set_smt_mode(kvm, mode, flags);
		break;
	}
#endif
	default:
		r = -EINVAL;
		break;
	}

	return r;
}

long kvm_arch_vm_ioctl(struct file *filp,
                       unsigned int ioctl, unsigned long arg)
{
	struct kvm *kvm __maybe_unused = filp->private_data;
	void __user *argp = (void __user *)arg;
	long r;

	switch (ioctl) {
	case KVM_PPC_GET_PVINFO: {
		struct kvm_ppc_pvinfo pvinfo;
		memset(&pvinfo, 0, sizeof(pvinfo));
		r = kvm_vm_ioctl_get_pvinfo(&pvinfo);
		if (copy_to_user(argp, &pvinfo, sizeof(pvinfo))) {
			r = -EFAULT;
			goto out;
		}

		break;
	}
	case KVM_ENABLE_CAP:
	{
		struct kvm_enable_cap cap;
		r = -EFAULT;
		if (copy_from_user(&cap, argp, sizeof(cap)))
			goto out;
		r = kvm_vm_ioctl_enable_cap(kvm, &cap);
		break;
	}
#ifdef CONFIG_SPAPR_TCE_IOMMU
	case KVM_CREATE_SPAPR_TCE_64: {
		struct kvm_create_spapr_tce_64 create_tce_64;

		r = -EFAULT;
		if (copy_from_user(&create_tce_64, argp, sizeof(create_tce_64)))
			goto out;
		if (create_tce_64.flags) {
			r = -EINVAL;
			goto out;
		}
		r = kvm_vm_ioctl_create_spapr_tce(kvm, &create_tce_64);
		goto out;
	}
	case KVM_CREATE_SPAPR_TCE: {
		struct kvm_create_spapr_tce create_tce;
		struct kvm_create_spapr_tce_64 create_tce_64;

		r = -EFAULT;
		if (copy_from_user(&create_tce, argp, sizeof(create_tce)))
			goto out;

		create_tce_64.liobn = create_tce.liobn;
		create_tce_64.page_shift = IOMMU_PAGE_SHIFT_4K;
		create_tce_64.offset = 0;
		create_tce_64.size = create_tce.window_size >>
				IOMMU_PAGE_SHIFT_4K;
		create_tce_64.flags = 0;
		r = kvm_vm_ioctl_create_spapr_tce(kvm, &create_tce_64);
		goto out;
	}
#endif
#ifdef CONFIG_PPC_BOOK3S_64
	case KVM_PPC_GET_SMMU_INFO: {
		struct kvm_ppc_smmu_info info;
		struct kvm *kvm = filp->private_data;

		memset(&info, 0, sizeof(info));
		r = kvm->arch.kvm_ops->get_smmu_info(kvm, &info);
		if (r >= 0 && copy_to_user(argp, &info, sizeof(info)))
			r = -EFAULT;
		break;
	}
	case KVM_PPC_RTAS_DEFINE_TOKEN: {
		struct kvm *kvm = filp->private_data;

		r = kvm_vm_ioctl_rtas_define_token(kvm, argp);
		break;
	}
	case KVM_PPC_CONFIGURE_V3_MMU: {
		struct kvm *kvm = filp->private_data;
		struct kvm_ppc_mmuv3_cfg cfg;

		r = -EINVAL;
		if (!kvm->arch.kvm_ops->configure_mmu)
			goto out;
		r = -EFAULT;
		if (copy_from_user(&cfg, argp, sizeof(cfg)))
			goto out;
		r = kvm->arch.kvm_ops->configure_mmu(kvm, &cfg);
		break;
	}
	case KVM_PPC_GET_RMMU_INFO: {
		struct kvm *kvm = filp->private_data;
		struct kvm_ppc_rmmu_info info;

		r = -EINVAL;
		if (!kvm->arch.kvm_ops->get_rmmu_info)
			goto out;
		r = kvm->arch.kvm_ops->get_rmmu_info(kvm, &info);
		if (r >= 0 && copy_to_user(argp, &info, sizeof(info)))
			r = -EFAULT;
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
	case KVM_PPC_GET_RMMU_INFO: {
		struct kvm *kvm = filp->private_data;
		struct kvm_ppc_rmmu_info info;

		r = -EINVAL;
		if (!kvm->arch.kvm_ops->get_rmmu_info)
			goto out;
		r = kvm->arch.kvm_ops->get_rmmu_info(kvm, &info);
		if (r >= 0 && copy_to_user(argp, &info, sizeof(info)))
			r = -EFAULT;
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