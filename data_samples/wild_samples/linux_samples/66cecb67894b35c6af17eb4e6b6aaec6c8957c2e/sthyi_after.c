{
	int i, r, pages;
	bool this_lpar;
	void *diag204_buf;
	void *diag224_buf = NULL;
	struct diag204_x_info_blk_hdr *ti_hdr;
	struct diag204_x_part_block *part_block;
	struct diag204_x_phys_block *phys_block;
	struct lpar_cpu_inf lpar_inf = {};

	/* Errors are handled through the validity bits in the response. */
	pages = diag204((unsigned long)DIAG204_SUBC_RSI |
			(unsigned long)DIAG204_INFO_EXT, 0, NULL);
	if (pages <= 0)
		return;

	diag204_buf = vmalloc(PAGE_SIZE * pages);
	if (!diag204_buf)
		return;

	r = diag204((unsigned long)DIAG204_SUBC_STIB7 |
		    (unsigned long)DIAG204_INFO_EXT, pages, diag204_buf);
	if (r < 0)
		goto out;

	diag224_buf = (void *)__get_free_page(GFP_KERNEL | GFP_DMA);
	if (!diag224_buf || diag224(diag224_buf))
		goto out;

	ti_hdr = diag204_buf;
	part_block = diag204_buf + sizeof(*ti_hdr);

	for (i = 0; i < ti_hdr->npar; i++) {
		/*
		 * For the calling lpar we also need to get the cpu
		 * caps and weights. The time information block header
		 * specifies the offset to the partition block of the
		 * caller lpar, so we know when we process its data.
		 */
		this_lpar = (void *)part_block - diag204_buf == ti_hdr->this_part;
		part_block = lpar_cpu_inf(&lpar_inf, this_lpar, diag224_buf,
					  part_block);
	}
	if (lpar_inf.ifl.lpar_weight) {
		sctns->par.infpwbif = sctns->mac.infmsifl * 0x10000 *
			lpar_inf.ifl.lpar_weight / lpar_inf.ifl.all_weight;
	}
	sctns->par.infpval1 |= PAR_WGHT_VLD;

out:
	free_page((unsigned long)diag224_buf);
	vfree(diag204_buf);
}

static int sthyi(u64 vaddr)
{
	register u64 code asm("0") = 0;
	register u64 addr asm("2") = vaddr;
	int cc;

	asm volatile(
		".insn   rre,0xB2560000,%[code],%[addr]\n"
		"ipm     %[cc]\n"
		"srl     %[cc],28\n"
		: [cc] "=d" (cc)
		: [code] "d" (code), [addr] "a" (addr)
		: "memory", "cc");
	return cc;
}

int handle_sthyi(struct kvm_vcpu *vcpu)
{
	int reg1, reg2, r = 0;
	u64 code, addr, cc = 0;
	struct sthyi_sctns *sctns = NULL;

	/*
	 * STHYI requires extensive locking in the higher hypervisors
	 * and is very computational/memory expensive. Therefore we
	 * ratelimit the executions per VM.
	 */
	if (!__ratelimit(&vcpu->kvm->arch.sthyi_limit)) {
		kvm_s390_retry_instr(vcpu);
		return 0;
	}