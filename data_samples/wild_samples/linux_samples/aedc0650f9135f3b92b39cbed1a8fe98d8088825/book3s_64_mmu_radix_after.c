// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright 2016 Paul Mackerras, IBM Corp. <paulus@au1.ibm.com>
 */

#include <linux/types.h>
#include <linux/string.h>
#include <linux/kvm.h>
#include <linux/kvm_host.h>
#include <linux/anon_inodes.h>
#include <linux/file.h>
#include <linux/debugfs.h>

#include <asm/kvm_ppc.h>
#include <asm/kvm_book3s.h>
#include <asm/page.h>
#include <asm/mmu.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/pte-walk.h>
#include <asm/ultravisor.h>
#include <asm/kvm_book3s_uvmem.h>

/*
 * Supported radix tree geometry.
 * Like p9, we support either 5 or 9 bits at the first (lowest) level,
 * for a page size of 64k or 4k.
 */
static int p9_supported_radix_bits[4] = { 5, 9, 9, 13 };

unsigned long __kvmhv_copy_tofrom_guest_radix(int lpid, int pid,
					      gva_t eaddr, void *to, void *from,
					      unsigned long n)
{
	int uninitialized_var(old_pid), old_lpid;
	unsigned long quadrant, ret = n;
	bool is_load = !!to;

	/* Can't access quadrants 1 or 2 in non-HV mode, call the HV to do it */
	if (kvmhv_on_pseries())
		return plpar_hcall_norets(H_COPY_TOFROM_GUEST, lpid, pid, eaddr,
					  __pa(to), __pa(from), n);

	quadrant = 1;
	if (!pid)
		quadrant = 2;
	if (is_load)
		from = (void *) (eaddr | (quadrant << 62));
	else
		to = (void *) (eaddr | (quadrant << 62));

	preempt_disable();

	/* switch the lpid first to avoid running host with unallocated pid */
	old_lpid = mfspr(SPRN_LPID);
	if (old_lpid != lpid)
		mtspr(SPRN_LPID, lpid);
	if (quadrant == 1) {
		old_pid = mfspr(SPRN_PID);
		if (old_pid != pid)
			mtspr(SPRN_PID, pid);
	}
	isync();

	pagefault_disable();
	if (is_load)
		ret = raw_copy_from_user(to, from, n);
	else
		ret = raw_copy_to_user(to, from, n);
	pagefault_enable();

	/* switch the pid first to avoid running host with unallocated pid */
	if (quadrant == 1 && pid != old_pid)
		mtspr(SPRN_PID, old_pid);
	if (lpid != old_lpid)
		mtspr(SPRN_LPID, old_lpid);
	isync();

	preempt_enable();

	return ret;
}
	if (dsisr & DSISR_BADACCESS) {
		/* Reflect to the guest as DSI */
		pr_err("KVM: Got radix HV page fault with DSISR=%lx\n", dsisr);
		kvmppc_core_queue_data_storage(vcpu, ea, dsisr);
		return RESUME_GUEST;
	}

	/* Translate the logical address */
	gpa = vcpu->arch.fault_gpa & ~0xfffUL;
	gpa &= ~0xF000000000000000ul;
	gfn = gpa >> PAGE_SHIFT;
	if (!(dsisr & DSISR_PRTABLE_FAULT))
		gpa |= ea & 0xfff;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return kvmppc_send_page_to_uv(kvm, gfn);

	/* Get the corresponding memslot */
	memslot = gfn_to_memslot(kvm, gfn);

	/* No memslot means it's an emulated MMIO region */
	if (!memslot || (memslot->flags & KVM_MEMSLOT_INVALID)) {
		if (dsisr & (DSISR_PRTABLE_FAULT | DSISR_BADACCESS |
			     DSISR_SET_RC)) {
			/*
			 * Bad address in guest page table tree, or other
			 * unusual error - reflect it to the guest as DSI.
			 */
			kvmppc_core_queue_data_storage(vcpu, ea, dsisr);
			return RESUME_GUEST;
		}
		return kvmppc_hv_emulate_mmio(run, vcpu, gpa, ea, writing);
	}
{
	pte_t *ptep;
	unsigned long gpa = gfn << PAGE_SHIFT;
	unsigned int shift;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE) {
		uv_page_inval(kvm->arch.lpid, gpa, PAGE_SHIFT);
		return 0;
	}
{
	pte_t *ptep;
	unsigned long gpa = gfn << PAGE_SHIFT;
	unsigned int shift;
	int ref = 0;
	unsigned long old, *rmapp;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return ref;

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep) && pte_young(*ptep)) {
		old = kvmppc_radix_update_pte(kvm, ptep, _PAGE_ACCESSED, 0,
					      gpa, shift);
		/* XXX need to flush tlb here? */
		/* Also clear bit in ptes in shadow pgtable for nested guests */
		rmapp = &memslot->arch.rmap[gfn - memslot->base_gfn];
		kvmhv_update_nest_rmap_rc_list(kvm, rmapp, _PAGE_ACCESSED, 0,
					       old & PTE_RPN_MASK,
					       1UL << shift);
		ref = 1;
	}
{
	pte_t *ptep;
	unsigned long gpa = gfn << PAGE_SHIFT;
	unsigned int shift;
	int ref = 0;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return ref;

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep) && pte_young(*ptep))
		ref = 1;
	return ref;
}

/* Returns the number of PAGE_SIZE pages that are dirty */
static int kvm_radix_test_clear_dirty(struct kvm *kvm,
				struct kvm_memory_slot *memslot, int pagenum)
{
	unsigned long gfn = memslot->base_gfn + pagenum;
	unsigned long gpa = gfn << PAGE_SHIFT;
	pte_t *ptep;
	unsigned int shift;
	int ret = 0;
	unsigned long old, *rmapp;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return ret;

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep) && pte_dirty(*ptep)) {
		ret = 1;
		if (shift)
			ret = 1 << (shift - PAGE_SHIFT);
		spin_lock(&kvm->mmu_lock);
		old = kvmppc_radix_update_pte(kvm, ptep, _PAGE_DIRTY, 0,
					      gpa, shift);
		kvmppc_radix_tlbie_page(kvm, gpa, shift, kvm->arch.lpid);
		/* Also clear bit in ptes in shadow pgtable for nested guests */
		rmapp = &memslot->arch.rmap[gfn - memslot->base_gfn];
		kvmhv_update_nest_rmap_rc_list(kvm, rmapp, _PAGE_DIRTY, 0,
					       old & PTE_RPN_MASK,
					       1UL << shift);
		spin_unlock(&kvm->mmu_lock);
	}
{
	unsigned long gfn = memslot->base_gfn + pagenum;
	unsigned long gpa = gfn << PAGE_SHIFT;
	pte_t *ptep;
	unsigned int shift;
	int ret = 0;
	unsigned long old, *rmapp;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return ret;

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep) && pte_dirty(*ptep)) {
		ret = 1;
		if (shift)
			ret = 1 << (shift - PAGE_SHIFT);
		spin_lock(&kvm->mmu_lock);
		old = kvmppc_radix_update_pte(kvm, ptep, _PAGE_DIRTY, 0,
					      gpa, shift);
		kvmppc_radix_tlbie_page(kvm, gpa, shift, kvm->arch.lpid);
		/* Also clear bit in ptes in shadow pgtable for nested guests */
		rmapp = &memslot->arch.rmap[gfn - memslot->base_gfn];
		kvmhv_update_nest_rmap_rc_list(kvm, rmapp, _PAGE_DIRTY, 0,
					       old & PTE_RPN_MASK,
					       1UL << shift);
		spin_unlock(&kvm->mmu_lock);
	}
{
	unsigned long n;
	pte_t *ptep;
	unsigned long gpa;
	unsigned int shift;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_START)
		kvmppc_uvmem_drop_pages(memslot, kvm);

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return;

	gpa = memslot->base_gfn << PAGE_SHIFT;
	spin_lock(&kvm->mmu_lock);
	for (n = memslot->npages; n; --n) {
		ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
		if (ptep && pte_present(*ptep))
			kvmppc_unmap_pte(kvm, ptep, gpa, shift, memslot,
					 kvm->arch.lpid);
		gpa += PAGE_SIZE;
	}