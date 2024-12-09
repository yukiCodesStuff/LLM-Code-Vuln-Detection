#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/pte-walk.h>
#include <asm/ultravisor.h>
#include <asm/kvm_book3s_uvmem.h>

/*
 * Supported radix tree geometry.
 * Like p9, we support either 5 or 9 bits at the first (lowest) level,
	if (!(dsisr & DSISR_PRTABLE_FAULT))
		gpa |= ea & 0xfff;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return kvmppc_send_page_to_uv(kvm, gfn);

	/* Get the corresponding memslot */
	memslot = gfn_to_memslot(kvm, gfn);

	/* No memslot means it's an emulated MMIO region */
	unsigned long gpa = gfn << PAGE_SHIFT;
	unsigned int shift;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE) {
		uv_page_inval(kvm->arch.lpid, gpa, PAGE_SHIFT);
		return 0;
	}

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep))
		kvmppc_unmap_pte(kvm, ptep, gpa, shift, memslot,
				 kvm->arch.lpid);
	int ref = 0;
	unsigned long old, *rmapp;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return ref;

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep) && pte_young(*ptep)) {
		old = kvmppc_radix_update_pte(kvm, ptep, _PAGE_ACCESSED, 0,
					      gpa, shift);
	unsigned int shift;
	int ref = 0;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return ref;

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep) && pte_young(*ptep))
		ref = 1;
	return ref;
	int ret = 0;
	unsigned long old, *rmapp;

	if (kvm->arch.secure_guest & KVMPPC_SECURE_INIT_DONE)
		return ret;

	ptep = __find_linux_pte(kvm->arch.pgtable, gpa, NULL, &shift);
	if (ptep && pte_present(*ptep) && pte_dirty(*ptep)) {
		ret = 1;
		if (shift)
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