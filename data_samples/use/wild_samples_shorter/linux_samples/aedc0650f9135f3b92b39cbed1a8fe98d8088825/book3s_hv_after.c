#include <asm/xics.h>
#include <asm/xive.h>
#include <asm/hw_breakpoint.h>
#include <asm/kvm_host.h>
#include <asm/kvm_book3s_uvmem.h>
#include <asm/ultravisor.h>

#include "book3s.h"

#define CREATE_TRACE_POINTS
					 kvmppc_get_gpr(vcpu, 5),
					 kvmppc_get_gpr(vcpu, 6));
		break;
	case H_SVM_PAGE_IN:
		ret = kvmppc_h_svm_page_in(vcpu->kvm,
					   kvmppc_get_gpr(vcpu, 4),
					   kvmppc_get_gpr(vcpu, 5),
					   kvmppc_get_gpr(vcpu, 6));
		break;
	case H_SVM_PAGE_OUT:
		ret = kvmppc_h_svm_page_out(vcpu->kvm,
					    kvmppc_get_gpr(vcpu, 4),
					    kvmppc_get_gpr(vcpu, 5),
					    kvmppc_get_gpr(vcpu, 6));
		break;
	case H_SVM_INIT_START:
		ret = kvmppc_h_svm_init_start(vcpu->kvm);
		break;
	case H_SVM_INIT_DONE:
		ret = kvmppc_h_svm_init_done(vcpu->kvm);
		break;

	default:
		return RESUME_HOST;
	}
	kvmppc_set_gpr(vcpu, 3, ret);
	if (change == KVM_MR_FLAGS_ONLY && kvm_is_radix(kvm) &&
	    ((new->flags ^ old->flags) & KVM_MEM_LOG_DIRTY_PAGES))
		kvmppc_radix_flush_memslot(kvm, old);
	/*
	 * If UV hasn't yet called H_SVM_INIT_START, don't register memslots.
	 */
	if (!kvm->arch.secure_guest)
		return;

	switch (change) {
	case KVM_MR_CREATE:
		if (kvmppc_uvmem_slot_init(kvm, new))
			return;
		uv_register_mem_slot(kvm->arch.lpid,
				     new->base_gfn << PAGE_SHIFT,
				     new->npages * PAGE_SIZE,
				     0, new->id);
		break;
	case KVM_MR_DELETE:
		uv_unregister_mem_slot(kvm->arch.lpid, old->id);
		kvmppc_uvmem_slot_free(kvm, old);
		break;
	default:
		/* TODO: Handle KVM_MR_MOVE */
		break;
	}
}

/*
 * Update LPCR values in kvm->arch and in vcores.
	char buf[32];
	int ret;

	mutex_init(&kvm->arch.uvmem_lock);
	INIT_LIST_HEAD(&kvm->arch.uvmem_pfns);
	mutex_init(&kvm->arch.mmu_setup_lock);

	/* Allocate the guest's logical partition ID */

		if (nesting_enabled(kvm))
			kvmhv_release_all_nested(kvm);
		kvm->arch.process_table = 0;
		uv_svm_terminate(kvm->arch.lpid);
		kvmhv_set_ptbl_entry(kvm->arch.lpid, 0, 0);
	}

	kvmppc_free_lpid(kvm->arch.lpid);

	kvmppc_free_pimap(kvm);
}
	return rc;
}

static void unpin_vpa_reset(struct kvm *kvm, struct kvmppc_vpa *vpa)
{
	unpin_vpa(kvm, vpa);
	vpa->gpa = 0;
	vpa->pinned_addr = NULL;
	vpa->dirty = false;
	vpa->update_pending = 0;
}

/*
 *  IOCTL handler to turn off secure mode of guest
 *
 * - Release all device pages
 * - Issue ucall to terminate the guest on the UV side
 * - Unpin the VPA pages.
 * - Reinit the partition scoped page tables
 */
static int kvmhv_svm_off(struct kvm *kvm)
{
	struct kvm_vcpu *vcpu;
	int mmu_was_ready;
	int srcu_idx;
	int ret = 0;
	int i;

	if (!(kvm->arch.secure_guest & KVMPPC_SECURE_INIT_START))
		return ret;

	mutex_lock(&kvm->arch.mmu_setup_lock);
	mmu_was_ready = kvm->arch.mmu_ready;
	if (kvm->arch.mmu_ready) {
		kvm->arch.mmu_ready = 0;
		/* order mmu_ready vs. vcpus_running */
		smp_mb();
		if (atomic_read(&kvm->arch.vcpus_running)) {
			kvm->arch.mmu_ready = 1;
			ret = -EBUSY;
			goto out;
		}
	}

	srcu_idx = srcu_read_lock(&kvm->srcu);
	for (i = 0; i < KVM_ADDRESS_SPACE_NUM; i++) {
		struct kvm_memory_slot *memslot;
		struct kvm_memslots *slots = __kvm_memslots(kvm, i);

		if (!slots)
			continue;

		kvm_for_each_memslot(memslot, slots) {
			kvmppc_uvmem_drop_pages(memslot, kvm);
			uv_unregister_mem_slot(kvm->arch.lpid, memslot->id);
		}
	}
	srcu_read_unlock(&kvm->srcu, srcu_idx);

	ret = uv_svm_terminate(kvm->arch.lpid);
	if (ret != U_SUCCESS) {
		ret = -EINVAL;
		goto out;
	}

	/*
	 * When secure guest is reset, all the guest pages are sent
	 * to UV via UV_PAGE_IN before the non-boot vcpus get a
	 * chance to run and unpin their VPA pages. Unpinning of all
	 * VPA pages is done here explicitly so that VPA pages
	 * can be migrated to the secure side.
	 *
	 * This is required to for the secure SMP guest to reboot
	 * correctly.
	 */
	kvm_for_each_vcpu(i, vcpu, kvm) {
		spin_lock(&vcpu->arch.vpa_update_lock);
		unpin_vpa_reset(kvm, &vcpu->arch.dtl);
		unpin_vpa_reset(kvm, &vcpu->arch.slb_shadow);
		unpin_vpa_reset(kvm, &vcpu->arch.vpa);
		spin_unlock(&vcpu->arch.vpa_update_lock);
	}

	kvmppc_setup_partition_table(kvm);
	kvm->arch.secure_guest = 0;
	kvm->arch.mmu_ready = mmu_was_ready;
out:
	mutex_unlock(&kvm->arch.mmu_setup_lock);
	return ret;
}

static struct kvmppc_ops kvm_ops_hv = {
	.get_sregs = kvm_arch_vcpu_ioctl_get_sregs_hv,
	.set_sregs = kvm_arch_vcpu_ioctl_set_sregs_hv,
	.get_one_reg = kvmppc_get_one_reg_hv,
	.enable_nested = kvmhv_enable_nested,
	.load_from_eaddr = kvmhv_load_from_eaddr,
	.store_to_eaddr = kvmhv_store_to_eaddr,
	.svm_off = kvmhv_svm_off,
};

static int kvm_init_subcore_bitmap(void)
{
			no_mixing_hpt_and_radix = true;
	}

	r = kvmppc_uvmem_init();
	if (r < 0)
		pr_err("KVM-HV: kvmppc_uvmem_init failed %d\n", r);

	return r;
}

static void kvmppc_book3s_exit_hv(void)
{
	kvmppc_uvmem_free();
	kvmppc_free_host_rm_ops();
	if (kvmppc_radix_possible())
		kvmppc_radix_exit();
	kvmppc_hv_ops = NULL;