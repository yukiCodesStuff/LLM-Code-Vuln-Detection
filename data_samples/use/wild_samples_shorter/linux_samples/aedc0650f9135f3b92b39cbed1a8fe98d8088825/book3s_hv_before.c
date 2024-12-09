#include <asm/xics.h>
#include <asm/xive.h>
#include <asm/hw_breakpoint.h>

#include "book3s.h"

#define CREATE_TRACE_POINTS
					 kvmppc_get_gpr(vcpu, 5),
					 kvmppc_get_gpr(vcpu, 6));
		break;
	default:
		return RESUME_HOST;
	}
	kvmppc_set_gpr(vcpu, 3, ret);
	if (change == KVM_MR_FLAGS_ONLY && kvm_is_radix(kvm) &&
	    ((new->flags ^ old->flags) & KVM_MEM_LOG_DIRTY_PAGES))
		kvmppc_radix_flush_memslot(kvm, old);
}

/*
 * Update LPCR values in kvm->arch and in vcores.
	char buf[32];
	int ret;

	mutex_init(&kvm->arch.mmu_setup_lock);

	/* Allocate the guest's logical partition ID */

		if (nesting_enabled(kvm))
			kvmhv_release_all_nested(kvm);
		kvm->arch.process_table = 0;
		kvmhv_set_ptbl_entry(kvm->arch.lpid, 0, 0);
	}
	kvmppc_free_lpid(kvm->arch.lpid);

	kvmppc_free_pimap(kvm);
}
	return rc;
}

static struct kvmppc_ops kvm_ops_hv = {
	.get_sregs = kvm_arch_vcpu_ioctl_get_sregs_hv,
	.set_sregs = kvm_arch_vcpu_ioctl_set_sregs_hv,
	.get_one_reg = kvmppc_get_one_reg_hv,
	.enable_nested = kvmhv_enable_nested,
	.load_from_eaddr = kvmhv_load_from_eaddr,
	.store_to_eaddr = kvmhv_store_to_eaddr,
};

static int kvm_init_subcore_bitmap(void)
{
			no_mixing_hpt_and_radix = true;
	}

	return r;
}

static void kvmppc_book3s_exit_hv(void)
{
	kvmppc_free_host_rm_ops();
	if (kvmppc_radix_possible())
		kvmppc_radix_exit();
	kvmppc_hv_ops = NULL;