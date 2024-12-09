		security_ftr_clear(SEC_FTR_BNDS_CHK_SPEC_BAR);
}

void pseries_setup_rfi_flush(void)
{
	struct h_cpu_char_result result;
	enum l1d_flush_type types;
	bool enable;

	setup_rfi_flush(types, enable);
	setup_count_cache_flush();
}

#ifdef CONFIG_PCI_IOV
enum rtas_iov_fw_value_map {

	fwnmi_init();

	pseries_setup_rfi_flush();
	setup_stf_barrier();
	pseries_lpar_read_hblkrm_characteristics();

	/* By default, only probe PCI (can be overridden by rtas_pci) */
	pci_add_flags(PCI_PROBE_ONLY);