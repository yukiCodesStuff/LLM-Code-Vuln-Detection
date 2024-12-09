{
	/*
	 * The features below are disabled by default, so we instead look to see
	 * if firmware has *enabled* them, and set them if so.
	 */
	if (result->character & H_CPU_CHAR_SPEC_BAR_ORI31)
		security_ftr_set(SEC_FTR_SPEC_BAR_ORI31);

	if (result->character & H_CPU_CHAR_BCCTRL_SERIALISED)
		security_ftr_set(SEC_FTR_BCCTRL_SERIALISED);

	if (result->character & H_CPU_CHAR_L1D_FLUSH_ORI30)
		security_ftr_set(SEC_FTR_L1D_FLUSH_ORI30);

	if (result->character & H_CPU_CHAR_L1D_FLUSH_TRIG2)
		security_ftr_set(SEC_FTR_L1D_FLUSH_TRIG2);

	if (result->character & H_CPU_CHAR_L1D_THREAD_PRIV)
		security_ftr_set(SEC_FTR_L1D_THREAD_PRIV);

	if (result->character & H_CPU_CHAR_COUNT_CACHE_DISABLED)
		security_ftr_set(SEC_FTR_COUNT_CACHE_DISABLED);

	if (result->character & H_CPU_CHAR_BCCTR_FLUSH_ASSIST)
		security_ftr_set(SEC_FTR_BCCTR_FLUSH_ASSIST);

	if (result->character & H_CPU_CHAR_BCCTR_LINK_FLUSH_ASSIST)
		security_ftr_set(SEC_FTR_BCCTR_LINK_FLUSH_ASSIST);

	if (result->behaviour & H_CPU_BEHAV_FLUSH_COUNT_CACHE)
		security_ftr_set(SEC_FTR_FLUSH_COUNT_CACHE);

	if (result->behaviour & H_CPU_BEHAV_FLUSH_LINK_STACK)
		security_ftr_set(SEC_FTR_FLUSH_LINK_STACK);

	/*
	 * The features below are enabled by default, so we instead look to see
	 * if firmware has *disabled* them, and clear them if so.
	 */
	if (!(result->behaviour & H_CPU_BEHAV_FAVOUR_SECURITY))
		security_ftr_clear(SEC_FTR_FAVOUR_SECURITY);

	if (!(result->behaviour & H_CPU_BEHAV_L1D_FLUSH_PR))
		security_ftr_clear(SEC_FTR_L1D_FLUSH_PR);

	if (!(result->behaviour & H_CPU_BEHAV_BNDS_CHK_SPEC_BAR))
		security_ftr_clear(SEC_FTR_BNDS_CHK_SPEC_BAR);
}

void pseries_setup_security_mitigations(void)
{
	struct h_cpu_char_result result;
	enum l1d_flush_type types;
	bool enable;
	long rc;

	/*
	 * Set features to the defaults assumed by init_cpu_char_feature_flags()
	 * so it can set/clear again any features that might have changed after
	 * migration, and in case the hypercall fails and it is not even called.
	 */
	powerpc_security_features = SEC_FTR_DEFAULT;

	rc = plpar_get_cpu_characteristics(&result);
	if (rc == H_SUCCESS)
		init_cpu_char_feature_flags(&result);

	/*
	 * We're the guest so this doesn't apply to us, clear it to simplify
	 * handling of it elsewhere.
	 */
	security_ftr_clear(SEC_FTR_L1D_FLUSH_HV);

	types = L1D_FLUSH_FALLBACK;

	if (security_ftr_enabled(SEC_FTR_L1D_FLUSH_TRIG2))
		types |= L1D_FLUSH_MTTRIG;

	if (security_ftr_enabled(SEC_FTR_L1D_FLUSH_ORI30))
		types |= L1D_FLUSH_ORI;

	enable = security_ftr_enabled(SEC_FTR_FAVOUR_SECURITY) && \
		 security_ftr_enabled(SEC_FTR_L1D_FLUSH_PR);

	setup_rfi_flush(types, enable);
	setup_count_cache_flush();

	enable = security_ftr_enabled(SEC_FTR_FAVOUR_SECURITY) &&
		 security_ftr_enabled(SEC_FTR_L1D_FLUSH_ENTRY);
	setup_entry_flush(enable);

	enable = security_ftr_enabled(SEC_FTR_FAVOUR_SECURITY) &&
		 security_ftr_enabled(SEC_FTR_L1D_FLUSH_UACCESS);
	setup_uaccess_flush(enable);

	setup_stf_barrier();
}

#ifdef CONFIG_PCI_IOV
enum rtas_iov_fw_value_map {
	NUM_RES_PROPERTY  = 0, /* Number of Resources */
	LOW_INT           = 1, /* Lowest 32 bits of Address */
	START_OF_ENTRIES  = 2, /* Always start of entry */
	APERTURE_PROPERTY = 2, /* Start of entry+ to  Aperture Size */
	WDW_SIZE_PROPERTY = 4, /* Start of entry+ to Window Size */
	NEXT_ENTRY        = 7  /* Go to next entry on array */
};

enum get_iov_fw_value_index {
	BAR_ADDRS     = 1,    /*  Get Bar Address */
	APERTURE_SIZE = 2,    /*  Get Aperture Size */
	WDW_SIZE      = 3     /*  Get Window Size */
};

resource_size_t pseries_get_iov_fw_value(struct pci_dev *dev, int resno,
					 enum get_iov_fw_value_index value)
{
	const int *indexes;
	struct device_node *dn = pci_device_to_OF_node(dev);
	int i, num_res, ret = 0;

	indexes = of_get_property(dn, "ibm,open-sriov-vf-bar-info", NULL);
	if (!indexes)
		return  0;

	/*
	 * First element in the array is the number of Bars
	 * returned.  Search through the list to find the matching
	 * bar
	 */
	num_res = of_read_number(&indexes[NUM_RES_PROPERTY], 1);
	if (resno >= num_res)
		return 0; /* or an errror */

	i = START_OF_ENTRIES + NEXT_ENTRY * resno;
	switch (value) {
	case BAR_ADDRS:
		ret = of_read_number(&indexes[i], 2);
		break;
	case APERTURE_SIZE:
		ret = of_read_number(&indexes[i + APERTURE_PROPERTY], 2);
		break;
	case WDW_SIZE:
		ret = of_read_number(&indexes[i + WDW_SIZE_PROPERTY], 2);
		break;
	}

	return ret;
}

void of_pci_set_vf_bar_size(struct pci_dev *dev, const int *indexes)
{
	struct resource *res;
	resource_size_t base, size;
	int i, r, num_res;

	num_res = of_read_number(&indexes[NUM_RES_PROPERTY], 1);
	num_res = min_t(int, num_res, PCI_SRIOV_NUM_BARS);
	for (i = START_OF_ENTRIES, r = 0; r < num_res && r < PCI_SRIOV_NUM_BARS;
	     i += NEXT_ENTRY, r++) {
		res = &dev->resource[r + PCI_IOV_RESOURCES];
		base = of_read_number(&indexes[i], 2);
		size = of_read_number(&indexes[i + APERTURE_PROPERTY], 2);
		res->flags = pci_parse_of_flags(of_read_number
						(&indexes[i + LOW_INT], 1), 0);
		res->flags |= (IORESOURCE_MEM_64 | IORESOURCE_PCI_FIXED);
		res->name = pci_name(dev);
		res->start = base;
		res->end = base + size - 1;
	}
}

void of_pci_parse_iov_addrs(struct pci_dev *dev, const int *indexes)
{
	struct resource *res, *root, *conflict;
	resource_size_t base, size;
	int i, r, num_res;

	/*
	 * First element in the array is the number of Bars
	 * returned.  Search through the list to find the matching
	 * bars assign them from firmware into resources structure.
	 */
	num_res = of_read_number(&indexes[NUM_RES_PROPERTY], 1);
	for (i = START_OF_ENTRIES, r = 0; r < num_res && r < PCI_SRIOV_NUM_BARS;
	     i += NEXT_ENTRY, r++) {
		res = &dev->resource[r + PCI_IOV_RESOURCES];
		base = of_read_number(&indexes[i], 2);
		size = of_read_number(&indexes[i + WDW_SIZE_PROPERTY], 2);
		res->name = pci_name(dev);
		res->start = base;
		res->end = base + size - 1;
		root = &iomem_resource;
		dev_dbg(&dev->dev,
			"pSeries IOV BAR %d: trying firmware assignment %pR\n",
			 r + PCI_IOV_RESOURCES, res);
		conflict = request_resource_conflict(root, res);
		if (conflict) {
			dev_info(&dev->dev,
				 "BAR %d: %pR conflicts with %s %pR\n",
				 r + PCI_IOV_RESOURCES, res,
				 conflict->name, conflict);
			res->flags |= IORESOURCE_UNSET;
		}
	}
}

static void pseries_disable_sriov_resources(struct pci_dev *pdev)
{
	int i;

	pci_warn(pdev, "No hypervisor support for SR-IOV on this device, IOV BARs disabled.\n");
	for (i = 0; i < PCI_SRIOV_NUM_BARS; i++)
		pdev->resource[i + PCI_IOV_RESOURCES].flags = 0;
}

static void pseries_pci_fixup_resources(struct pci_dev *pdev)
{
	const int *indexes;
	struct device_node *dn = pci_device_to_OF_node(pdev);

	/*Firmware must support open sriov otherwise dont configure*/
	indexes = of_get_property(dn, "ibm,open-sriov-vf-bar-info", NULL);
	if (indexes)
		of_pci_set_vf_bar_size(pdev, indexes);
	else
		pseries_disable_sriov_resources(pdev);
}

static void pseries_pci_fixup_iov_resources(struct pci_dev *pdev)
{
	const int *indexes;
	struct device_node *dn = pci_device_to_OF_node(pdev);

	if (!pdev->is_physfn || pci_dev_is_added(pdev))
		return;
	/*Firmware must support open sriov otherwise dont configure*/
	indexes = of_get_property(dn, "ibm,open-sriov-vf-bar-info", NULL);
	if (indexes)
		of_pci_parse_iov_addrs(pdev, indexes);
	else
		pseries_disable_sriov_resources(pdev);
}

static resource_size_t pseries_pci_iov_resource_alignment(struct pci_dev *pdev,
							  int resno)
{
	const __be32 *reg;
	struct device_node *dn = pci_device_to_OF_node(pdev);

	/*Firmware must support open sriov otherwise report regular alignment*/
	reg = of_get_property(dn, "ibm,is-open-sriov-pf", NULL);
	if (!reg)
		return pci_iov_resource_size(pdev, resno);

	if (!pdev->is_physfn)
		return 0;
	return pseries_get_iov_fw_value(pdev,
					resno - PCI_IOV_RESOURCES,
					APERTURE_SIZE);
}
#endif

static void __init pSeries_setup_arch(void)
{
	set_arch_panic_timeout(10, ARCH_PANIC_TIMEOUT);

	/* Discover PIC type and setup ppc_md accordingly */
	smp_init_pseries();


	if (radix_enabled() && !mmu_has_feature(MMU_FTR_GTSE))
		if (!firmware_has_feature(FW_FEATURE_RPT_INVALIDATE))
			panic("BUG: Radix support requires either GTSE or RPT_INVALIDATE\n");


	/* openpic global configuration register (64-bit format). */
	/* openpic Interrupt Source Unit pointer (64-bit format). */
	/* python0 facility area (mmio) (64-bit format) REAL address. */

	/* init to some ~sane value until calibrate_delay() runs */
	loops_per_jiffy = 50000000;

	fwnmi_init();

	pseries_setup_security_mitigations();
	pseries_lpar_read_hblkrm_characteristics();

	/* By default, only probe PCI (can be overridden by rtas_pci) */
	pci_add_flags(PCI_PROBE_ONLY);

	/* Find and initialize PCI host bridges */
	init_pci_config_tokens();
	find_and_init_phbs();
	of_reconfig_notifier_register(&pci_dn_reconfig_nb);

	pSeries_nvram_init();

	if (firmware_has_feature(FW_FEATURE_LPAR)) {
		vpa_init(boot_cpuid);

		if (lppaca_shared_proc(get_lppaca())) {
			static_branch_enable(&shared_processor);
			pv_spinlocks_init();
		}

		ppc_md.power_save = pseries_lpar_idle;
		ppc_md.enable_pmcs = pseries_lpar_enable_pmcs;
#ifdef CONFIG_PCI_IOV
		ppc_md.pcibios_fixup_resources =
			pseries_pci_fixup_resources;
		ppc_md.pcibios_fixup_sriov =
			pseries_pci_fixup_iov_resources;
		ppc_md.pcibios_iov_resource_alignment =
			pseries_pci_iov_resource_alignment;
#endif
	} else {
		/* No special idle routine */
		ppc_md.enable_pmcs = power4_enable_pmcs;
	}

	ppc_md.pcibios_root_bridge_prepare = pseries_root_bridge_prepare;

	if (swiotlb_force == SWIOTLB_FORCE)
		ppc_swiotlb_enable = 1;
}

static void pseries_panic(char *str)
{
	panic_flush_kmsg_end();
	rtas_os_term(str);
}

static int __init pSeries_init_panel(void)
{
	/* Manually leave the kernel version on the panel. */
#ifdef __BIG_ENDIAN__
	ppc_md.progress("Linux ppc64\n", 0);
#else
	ppc_md.progress("Linux ppc64le\n", 0);
#endif
	ppc_md.progress(init_utsname()->version, 0);

	return 0;
}
machine_arch_initcall(pseries, pSeries_init_panel);

static int pseries_set_dabr(unsigned long dabr, unsigned long dabrx)
{
	return plpar_hcall_norets(H_SET_DABR, dabr);
}

static int pseries_set_xdabr(unsigned long dabr, unsigned long dabrx)
{
	/* Have to set at least one bit in the DABRX according to PAPR */
	if (dabrx == 0 && dabr == 0)
		dabrx = DABRX_USER;
	/* PAPR says we can only set kernel and user bits */
	dabrx &= DABRX_KERNEL | DABRX_USER;

	return plpar_hcall_norets(H_SET_XDABR, dabr, dabrx);
}

static int pseries_set_dawr(int nr, unsigned long dawr, unsigned long dawrx)
{
	/* PAPR says we can't set HYP */
	dawrx &= ~DAWRX_HYP;

	if (nr == 0)
		return plpar_set_watchpoint0(dawr, dawrx);
	else
		return plpar_set_watchpoint1(dawr, dawrx);
}

#define CMO_CHARACTERISTICS_TOKEN 44
#define CMO_MAXLENGTH 1026

void pSeries_coalesce_init(void)
{
	struct hvcall_mpp_x_data mpp_x_data;

	if (firmware_has_feature(FW_FEATURE_CMO) && !h_get_mpp_x(&mpp_x_data))
		powerpc_firmware_features |= FW_FEATURE_XCMO;
	else
		powerpc_firmware_features &= ~FW_FEATURE_XCMO;
}

/**
 * fw_cmo_feature_init - FW_FEATURE_CMO is not stored in ibm,hypertas-functions,
 * handle that here. (Stolen from parse_system_parameter_string)
 */
static void pSeries_cmo_feature_init(void)
{
	char *ptr, *key, *value, *end;
	int call_status;
	int page_order = IOMMU_PAGE_SHIFT_4K;

	pr_debug(" -> fw_cmo_feature_init()\n");
	spin_lock(&rtas_data_buf_lock);
	memset(rtas_data_buf, 0, RTAS_DATA_BUF_SIZE);
	call_status = rtas_call(rtas_token("ibm,get-system-parameter"), 3, 1,
				NULL,
				CMO_CHARACTERISTICS_TOKEN,
				__pa(rtas_data_buf),
				RTAS_DATA_BUF_SIZE);

	if (call_status != 0) {
		spin_unlock(&rtas_data_buf_lock);
		pr_debug("CMO not available\n");
		pr_debug(" <- fw_cmo_feature_init()\n");
		return;
	}

	end = rtas_data_buf + CMO_MAXLENGTH - 2;
	ptr = rtas_data_buf + 2;	/* step over strlen value */
	key = value = ptr;

	while (*ptr && (ptr <= end)) {
		/* Separate the key and value by replacing '=' with '\0' and
		 * point the value at the string after the '='
		 */
		if (ptr[0] == '=') {
			ptr[0] = '\0';
			value = ptr + 1;
		} else if (ptr[0] == '\0' || ptr[0] == ',') {
			/* Terminate the string containing the key/value pair */
			ptr[0] = '\0';

			if (key == value) {
				pr_debug("Malformed key/value pair\n");
				/* Never found a '=', end processing */
				break;
			}

			if (0 == strcmp(key, "CMOPageSize"))
				page_order = simple_strtol(value, NULL, 10);
			else if (0 == strcmp(key, "PrPSP"))
				CMO_PrPSP = simple_strtol(value, NULL, 10);
			else if (0 == strcmp(key, "SecPSP"))
				CMO_SecPSP = simple_strtol(value, NULL, 10);
			value = key = ptr + 1;
		}
		ptr++;
	}

	/* Page size is returned as the power of 2 of the page size,
	 * convert to the page size in bytes before returning
	 */
	CMO_PageSize = 1 << page_order;
	pr_debug("CMO_PageSize = %lu\n", CMO_PageSize);

	if (CMO_PrPSP != -1 || CMO_SecPSP != -1) {
		pr_info("CMO enabled\n");
		pr_debug("CMO enabled, PrPSP=%d, SecPSP=%d\n", CMO_PrPSP,
		         CMO_SecPSP);
		powerpc_firmware_features |= FW_FEATURE_CMO;
		pSeries_coalesce_init();
	} else
		pr_debug("CMO not enabled, PrPSP=%d, SecPSP=%d\n", CMO_PrPSP,
		         CMO_SecPSP);
	spin_unlock(&rtas_data_buf_lock);
	pr_debug(" <- fw_cmo_feature_init()\n");
}

/*
 * Early initialization.  Relocation is on but do not reference unbolted pages
 */
static void __init pseries_init(void)
{
	pr_debug(" -> pseries_init()\n");

#ifdef CONFIG_HVC_CONSOLE
	if (firmware_has_feature(FW_FEATURE_LPAR))
		hvc_vio_init_early();
#endif
	if (firmware_has_feature(FW_FEATURE_XDABR))
		ppc_md.set_dabr = pseries_set_xdabr;
	else if (firmware_has_feature(FW_FEATURE_DABR))
		ppc_md.set_dabr = pseries_set_dabr;

	if (firmware_has_feature(FW_FEATURE_SET_MODE))
		ppc_md.set_dawr = pseries_set_dawr;

	pSeries_cmo_feature_init();
	iommu_init_early_pSeries();

	pr_debug(" <- pseries_init()\n");
}

/**
 * pseries_power_off - tell firmware about how to power off the system.
 *
 * This function calls either the power-off rtas token in normal cases
 * or the ibm,power-off-ups token (if present & requested) in case of
 * a power failure. If power-off token is used, power on will only be
 * possible with power button press. If ibm,power-off-ups token is used
 * it will allow auto poweron after power is restored.
 */
static void pseries_power_off(void)
{
	int rc;
	int rtas_poweroff_ups_token = rtas_token("ibm,power-off-ups");

	if (rtas_flash_term_hook)
		rtas_flash_term_hook(SYS_POWER_OFF);

	if (rtas_poweron_auto == 0 ||
		rtas_poweroff_ups_token == RTAS_UNKNOWN_SERVICE) {
		rc = rtas_call(rtas_token("power-off"), 2, 1, NULL, -1, -1);
		printk(KERN_INFO "RTAS power-off returned %d\n", rc);
	} else {
		rc = rtas_call(rtas_poweroff_ups_token, 0, 1, NULL);
		printk(KERN_INFO "RTAS ibm,power-off-ups returned %d\n", rc);
	}
	for (;;);
}

static int __init pSeries_probe(void)
{
	if (!of_node_is_type(of_root, "chrp"))
		return 0;

	/* Cell blades firmware claims to be chrp while it's not. Until this
	 * is fixed, we need to avoid those here.
	 */
	if (of_machine_is_compatible("IBM,CPBW-1.0") ||
	    of_machine_is_compatible("IBM,CBEA"))
		return 0;

	pm_power_off = pseries_power_off;

	pr_debug("Machine is%s LPAR !\n",
	         (powerpc_firmware_features & FW_FEATURE_LPAR) ? "" : " not");

	pseries_init();

	return 1;
}

static int pSeries_pci_probe_mode(struct pci_bus *bus)
{
	if (firmware_has_feature(FW_FEATURE_LPAR))
		return PCI_PROBE_DEVTREE;
	return PCI_PROBE_NORMAL;
}

struct pci_controller_ops pseries_pci_controller_ops = {
	.probe_mode		= pSeries_pci_probe_mode,
};

define_machine(pseries) {
	.name			= "pSeries",
	.probe			= pSeries_probe,
	.setup_arch		= pSeries_setup_arch,
	.init_IRQ		= pseries_init_irq,
	.show_cpuinfo		= pSeries_show_cpuinfo,
	.log_error		= pSeries_log_error,
	.pcibios_fixup		= pSeries_final_fixup,
	.restart		= rtas_restart,
	.halt			= rtas_halt,
	.panic			= pseries_panic,
	.get_boot_time		= rtas_get_boot_time,
	.get_rtc_time		= rtas_get_rtc_time,
	.set_rtc_time		= rtas_set_rtc_time,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= rtas_progress,
	.system_reset_exception = pSeries_system_reset_exception,
	.machine_check_early	= pseries_machine_check_realmode,
	.machine_check_exception = pSeries_machine_check_exception,
#ifdef CONFIG_KEXEC_CORE
	.machine_kexec          = pSeries_machine_kexec,
	.kexec_cpu_down         = pseries_kexec_cpu_down,
#endif
#ifdef CONFIG_MEMORY_HOTPLUG_SPARSE
	.memory_block_size	= pseries_memory_block_size,
#endif
};
{
	struct h_cpu_char_result result;
	enum l1d_flush_type types;
	bool enable;
	long rc;

	/*
	 * Set features to the defaults assumed by init_cpu_char_feature_flags()
	 * so it can set/clear again any features that might have changed after
	 * migration, and in case the hypercall fails and it is not even called.
	 */
	powerpc_security_features = SEC_FTR_DEFAULT;

	rc = plpar_get_cpu_characteristics(&result);
	if (rc == H_SUCCESS)
		init_cpu_char_feature_flags(&result);

	/*
	 * We're the guest so this doesn't apply to us, clear it to simplify
	 * handling of it elsewhere.
	 */
	security_ftr_clear(SEC_FTR_L1D_FLUSH_HV);

	types = L1D_FLUSH_FALLBACK;

	if (security_ftr_enabled(SEC_FTR_L1D_FLUSH_TRIG2))
		types |= L1D_FLUSH_MTTRIG;

	if (security_ftr_enabled(SEC_FTR_L1D_FLUSH_ORI30))
		types |= L1D_FLUSH_ORI;

	enable = security_ftr_enabled(SEC_FTR_FAVOUR_SECURITY) && \
		 security_ftr_enabled(SEC_FTR_L1D_FLUSH_PR);

	setup_rfi_flush(types, enable);
	setup_count_cache_flush();

	enable = security_ftr_enabled(SEC_FTR_FAVOUR_SECURITY) &&
		 security_ftr_enabled(SEC_FTR_L1D_FLUSH_ENTRY);
	setup_entry_flush(enable);

	enable = security_ftr_enabled(SEC_FTR_FAVOUR_SECURITY) &&
		 security_ftr_enabled(SEC_FTR_L1D_FLUSH_UACCESS);
	setup_uaccess_flush(enable);

	setup_stf_barrier();
}

#ifdef CONFIG_PCI_IOV
enum rtas_iov_fw_value_map {
	NUM_RES_PROPERTY  = 0, /* Number of Resources */
	LOW_INT           = 1, /* Lowest 32 bits of Address */
	START_OF_ENTRIES  = 2, /* Always start of entry */
	APERTURE_PROPERTY = 2, /* Start of entry+ to  Aperture Size */
	WDW_SIZE_PROPERTY = 4, /* Start of entry+ to Window Size */
	NEXT_ENTRY        = 7  /* Go to next entry on array */
};

enum get_iov_fw_value_index {
	BAR_ADDRS     = 1,    /*  Get Bar Address */
	APERTURE_SIZE = 2,    /*  Get Aperture Size */
	WDW_SIZE      = 3     /*  Get Window Size */
};

resource_size_t pseries_get_iov_fw_value(struct pci_dev *dev, int resno,
					 enum get_iov_fw_value_index value)
{
	const int *indexes;
	struct device_node *dn = pci_device_to_OF_node(dev);
	int i, num_res, ret = 0;

	indexes = of_get_property(dn, "ibm,open-sriov-vf-bar-info", NULL);
	if (!indexes)
		return  0;

	/*
	 * First element in the array is the number of Bars
	 * returned.  Search through the list to find the matching
	 * bar
	 */
	num_res = of_read_number(&indexes[NUM_RES_PROPERTY], 1);
	if (resno >= num_res)
		return 0; /* or an errror */

	i = START_OF_ENTRIES + NEXT_ENTRY * resno;
	switch (value) {
	case BAR_ADDRS:
		ret = of_read_number(&indexes[i], 2);
		break;
	case APERTURE_SIZE:
		ret = of_read_number(&indexes[i + APERTURE_PROPERTY], 2);
		break;
	case WDW_SIZE:
		ret = of_read_number(&indexes[i + WDW_SIZE_PROPERTY], 2);
		break;
	}

	return ret;
}

void of_pci_set_vf_bar_size(struct pci_dev *dev, const int *indexes)
{
	struct resource *res;
	resource_size_t base, size;
	int i, r, num_res;

	num_res = of_read_number(&indexes[NUM_RES_PROPERTY], 1);
	num_res = min_t(int, num_res, PCI_SRIOV_NUM_BARS);
	for (i = START_OF_ENTRIES, r = 0; r < num_res && r < PCI_SRIOV_NUM_BARS;
	     i += NEXT_ENTRY, r++) {
		res = &dev->resource[r + PCI_IOV_RESOURCES];
		base = of_read_number(&indexes[i], 2);
		size = of_read_number(&indexes[i + APERTURE_PROPERTY], 2);
		res->flags = pci_parse_of_flags(of_read_number
						(&indexes[i + LOW_INT], 1), 0);
		res->flags |= (IORESOURCE_MEM_64 | IORESOURCE_PCI_FIXED);
		res->name = pci_name(dev);
		res->start = base;
		res->end = base + size - 1;
	}
}

void of_pci_parse_iov_addrs(struct pci_dev *dev, const int *indexes)
{
	struct resource *res, *root, *conflict;
	resource_size_t base, size;
	int i, r, num_res;

	/*
	 * First element in the array is the number of Bars
	 * returned.  Search through the list to find the matching
	 * bars assign them from firmware into resources structure.
	 */
	num_res = of_read_number(&indexes[NUM_RES_PROPERTY], 1);
	for (i = START_OF_ENTRIES, r = 0; r < num_res && r < PCI_SRIOV_NUM_BARS;
	     i += NEXT_ENTRY, r++) {
		res = &dev->resource[r + PCI_IOV_RESOURCES];
		base = of_read_number(&indexes[i], 2);
		size = of_read_number(&indexes[i + WDW_SIZE_PROPERTY], 2);
		res->name = pci_name(dev);
		res->start = base;
		res->end = base + size - 1;
		root = &iomem_resource;
		dev_dbg(&dev->dev,
			"pSeries IOV BAR %d: trying firmware assignment %pR\n",
			 r + PCI_IOV_RESOURCES, res);
		conflict = request_resource_conflict(root, res);
		if (conflict) {
			dev_info(&dev->dev,
				 "BAR %d: %pR conflicts with %s %pR\n",
				 r + PCI_IOV_RESOURCES, res,
				 conflict->name, conflict);
			res->flags |= IORESOURCE_UNSET;
		}
	}
}

static void pseries_disable_sriov_resources(struct pci_dev *pdev)
{
	int i;

	pci_warn(pdev, "No hypervisor support for SR-IOV on this device, IOV BARs disabled.\n");
	for (i = 0; i < PCI_SRIOV_NUM_BARS; i++)
		pdev->resource[i + PCI_IOV_RESOURCES].flags = 0;
}

static void pseries_pci_fixup_resources(struct pci_dev *pdev)
{
	const int *indexes;
	struct device_node *dn = pci_device_to_OF_node(pdev);

	/*Firmware must support open sriov otherwise dont configure*/
	indexes = of_get_property(dn, "ibm,open-sriov-vf-bar-info", NULL);
	if (indexes)
		of_pci_set_vf_bar_size(pdev, indexes);
	else
		pseries_disable_sriov_resources(pdev);
}

static void pseries_pci_fixup_iov_resources(struct pci_dev *pdev)
{
	const int *indexes;
	struct device_node *dn = pci_device_to_OF_node(pdev);

	if (!pdev->is_physfn || pci_dev_is_added(pdev))
		return;
	/*Firmware must support open sriov otherwise dont configure*/
	indexes = of_get_property(dn, "ibm,open-sriov-vf-bar-info", NULL);
	if (indexes)
		of_pci_parse_iov_addrs(pdev, indexes);
	else
		pseries_disable_sriov_resources(pdev);
}

static resource_size_t pseries_pci_iov_resource_alignment(struct pci_dev *pdev,
							  int resno)
{
	const __be32 *reg;
	struct device_node *dn = pci_device_to_OF_node(pdev);

	/*Firmware must support open sriov otherwise report regular alignment*/
	reg = of_get_property(dn, "ibm,is-open-sriov-pf", NULL);
	if (!reg)
		return pci_iov_resource_size(pdev, resno);

	if (!pdev->is_physfn)
		return 0;
	return pseries_get_iov_fw_value(pdev,
					resno - PCI_IOV_RESOURCES,
					APERTURE_SIZE);
}
#endif

static void __init pSeries_setup_arch(void)
{
	set_arch_panic_timeout(10, ARCH_PANIC_TIMEOUT);

	/* Discover PIC type and setup ppc_md accordingly */
	smp_init_pseries();


	if (radix_enabled() && !mmu_has_feature(MMU_FTR_GTSE))
		if (!firmware_has_feature(FW_FEATURE_RPT_INVALIDATE))
			panic("BUG: Radix support requires either GTSE or RPT_INVALIDATE\n");


	/* openpic global configuration register (64-bit format). */
	/* openpic Interrupt Source Unit pointer (64-bit format). */
	/* python0 facility area (mmio) (64-bit format) REAL address. */

	/* init to some ~sane value until calibrate_delay() runs */
	loops_per_jiffy = 50000000;

	fwnmi_init();

	pseries_setup_security_mitigations();
	pseries_lpar_read_hblkrm_characteristics();

	/* By default, only probe PCI (can be overridden by rtas_pci) */
	pci_add_flags(PCI_PROBE_ONLY);

	/* Find and initialize PCI host bridges */
	init_pci_config_tokens();
	find_and_init_phbs();
	of_reconfig_notifier_register(&pci_dn_reconfig_nb);

	pSeries_nvram_init();

	if (firmware_has_feature(FW_FEATURE_LPAR)) {
		vpa_init(boot_cpuid);

		if (lppaca_shared_proc(get_lppaca())) {
			static_branch_enable(&shared_processor);
			pv_spinlocks_init();
		}

		ppc_md.power_save = pseries_lpar_idle;
		ppc_md.enable_pmcs = pseries_lpar_enable_pmcs;
#ifdef CONFIG_PCI_IOV
		ppc_md.pcibios_fixup_resources =
			pseries_pci_fixup_resources;
		ppc_md.pcibios_fixup_sriov =
			pseries_pci_fixup_iov_resources;
		ppc_md.pcibios_iov_resource_alignment =
			pseries_pci_iov_resource_alignment;
#endif
	} else {
		/* No special idle routine */
		ppc_md.enable_pmcs = power4_enable_pmcs;
	}

	ppc_md.pcibios_root_bridge_prepare = pseries_root_bridge_prepare;

	if (swiotlb_force == SWIOTLB_FORCE)
		ppc_swiotlb_enable = 1;
}

static void pseries_panic(char *str)
{
	panic_flush_kmsg_end();
	rtas_os_term(str);
}

static int __init pSeries_init_panel(void)
{
	/* Manually leave the kernel version on the panel. */
#ifdef __BIG_ENDIAN__
	ppc_md.progress("Linux ppc64\n", 0);
#else
	ppc_md.progress("Linux ppc64le\n", 0);
#endif
	ppc_md.progress(init_utsname()->version, 0);

	return 0;
}
machine_arch_initcall(pseries, pSeries_init_panel);

static int pseries_set_dabr(unsigned long dabr, unsigned long dabrx)
{
	return plpar_hcall_norets(H_SET_DABR, dabr);
}

static int pseries_set_xdabr(unsigned long dabr, unsigned long dabrx)
{
	/* Have to set at least one bit in the DABRX according to PAPR */
	if (dabrx == 0 && dabr == 0)
		dabrx = DABRX_USER;
	/* PAPR says we can only set kernel and user bits */
	dabrx &= DABRX_KERNEL | DABRX_USER;

	return plpar_hcall_norets(H_SET_XDABR, dabr, dabrx);
}

static int pseries_set_dawr(int nr, unsigned long dawr, unsigned long dawrx)
{
	/* PAPR says we can't set HYP */
	dawrx &= ~DAWRX_HYP;

	if (nr == 0)
		return plpar_set_watchpoint0(dawr, dawrx);
	else
		return plpar_set_watchpoint1(dawr, dawrx);
}

#define CMO_CHARACTERISTICS_TOKEN 44
#define CMO_MAXLENGTH 1026

void pSeries_coalesce_init(void)
{
	struct hvcall_mpp_x_data mpp_x_data;

	if (firmware_has_feature(FW_FEATURE_CMO) && !h_get_mpp_x(&mpp_x_data))
		powerpc_firmware_features |= FW_FEATURE_XCMO;
	else
		powerpc_firmware_features &= ~FW_FEATURE_XCMO;
}

/**
 * fw_cmo_feature_init - FW_FEATURE_CMO is not stored in ibm,hypertas-functions,
 * handle that here. (Stolen from parse_system_parameter_string)
 */
static void pSeries_cmo_feature_init(void)
{
	char *ptr, *key, *value, *end;
	int call_status;
	int page_order = IOMMU_PAGE_SHIFT_4K;

	pr_debug(" -> fw_cmo_feature_init()\n");
	spin_lock(&rtas_data_buf_lock);
	memset(rtas_data_buf, 0, RTAS_DATA_BUF_SIZE);
	call_status = rtas_call(rtas_token("ibm,get-system-parameter"), 3, 1,
				NULL,
				CMO_CHARACTERISTICS_TOKEN,
				__pa(rtas_data_buf),
				RTAS_DATA_BUF_SIZE);

	if (call_status != 0) {
		spin_unlock(&rtas_data_buf_lock);
		pr_debug("CMO not available\n");
		pr_debug(" <- fw_cmo_feature_init()\n");
		return;
	}

	end = rtas_data_buf + CMO_MAXLENGTH - 2;
	ptr = rtas_data_buf + 2;	/* step over strlen value */
	key = value = ptr;

	while (*ptr && (ptr <= end)) {
		/* Separate the key and value by replacing '=' with '\0' and
		 * point the value at the string after the '='
		 */
		if (ptr[0] == '=') {
			ptr[0] = '\0';
			value = ptr + 1;
		} else if (ptr[0] == '\0' || ptr[0] == ',') {
			/* Terminate the string containing the key/value pair */
			ptr[0] = '\0';

			if (key == value) {
				pr_debug("Malformed key/value pair\n");
				/* Never found a '=', end processing */
				break;
			}

			if (0 == strcmp(key, "CMOPageSize"))
				page_order = simple_strtol(value, NULL, 10);
			else if (0 == strcmp(key, "PrPSP"))
				CMO_PrPSP = simple_strtol(value, NULL, 10);
			else if (0 == strcmp(key, "SecPSP"))
				CMO_SecPSP = simple_strtol(value, NULL, 10);
			value = key = ptr + 1;
		}
		ptr++;
	}

	/* Page size is returned as the power of 2 of the page size,
	 * convert to the page size in bytes before returning
	 */
	CMO_PageSize = 1 << page_order;
	pr_debug("CMO_PageSize = %lu\n", CMO_PageSize);

	if (CMO_PrPSP != -1 || CMO_SecPSP != -1) {
		pr_info("CMO enabled\n");
		pr_debug("CMO enabled, PrPSP=%d, SecPSP=%d\n", CMO_PrPSP,
		         CMO_SecPSP);
		powerpc_firmware_features |= FW_FEATURE_CMO;
		pSeries_coalesce_init();
	} else
		pr_debug("CMO not enabled, PrPSP=%d, SecPSP=%d\n", CMO_PrPSP,
		         CMO_SecPSP);
	spin_unlock(&rtas_data_buf_lock);
	pr_debug(" <- fw_cmo_feature_init()\n");
}

/*
 * Early initialization.  Relocation is on but do not reference unbolted pages
 */
static void __init pseries_init(void)
{
	pr_debug(" -> pseries_init()\n");

#ifdef CONFIG_HVC_CONSOLE
	if (firmware_has_feature(FW_FEATURE_LPAR))
		hvc_vio_init_early();
#endif
	if (firmware_has_feature(FW_FEATURE_XDABR))
		ppc_md.set_dabr = pseries_set_xdabr;
	else if (firmware_has_feature(FW_FEATURE_DABR))
		ppc_md.set_dabr = pseries_set_dabr;

	if (firmware_has_feature(FW_FEATURE_SET_MODE))
		ppc_md.set_dawr = pseries_set_dawr;

	pSeries_cmo_feature_init();
	iommu_init_early_pSeries();

	pr_debug(" <- pseries_init()\n");
}

/**
 * pseries_power_off - tell firmware about how to power off the system.
 *
 * This function calls either the power-off rtas token in normal cases
 * or the ibm,power-off-ups token (if present & requested) in case of
 * a power failure. If power-off token is used, power on will only be
 * possible with power button press. If ibm,power-off-ups token is used
 * it will allow auto poweron after power is restored.
 */
static void pseries_power_off(void)
{
	int rc;
	int rtas_poweroff_ups_token = rtas_token("ibm,power-off-ups");

	if (rtas_flash_term_hook)
		rtas_flash_term_hook(SYS_POWER_OFF);

	if (rtas_poweron_auto == 0 ||
		rtas_poweroff_ups_token == RTAS_UNKNOWN_SERVICE) {
		rc = rtas_call(rtas_token("power-off"), 2, 1, NULL, -1, -1);
		printk(KERN_INFO "RTAS power-off returned %d\n", rc);
	} else {
		rc = rtas_call(rtas_poweroff_ups_token, 0, 1, NULL);
		printk(KERN_INFO "RTAS ibm,power-off-ups returned %d\n", rc);
	}
	for (;;);
}

static int __init pSeries_probe(void)
{
	if (!of_node_is_type(of_root, "chrp"))
		return 0;

	/* Cell blades firmware claims to be chrp while it's not. Until this
	 * is fixed, we need to avoid those here.
	 */
	if (of_machine_is_compatible("IBM,CPBW-1.0") ||
	    of_machine_is_compatible("IBM,CBEA"))
		return 0;

	pm_power_off = pseries_power_off;

	pr_debug("Machine is%s LPAR !\n",
	         (powerpc_firmware_features & FW_FEATURE_LPAR) ? "" : " not");

	pseries_init();

	return 1;
}

static int pSeries_pci_probe_mode(struct pci_bus *bus)
{
	if (firmware_has_feature(FW_FEATURE_LPAR))
		return PCI_PROBE_DEVTREE;
	return PCI_PROBE_NORMAL;
}

struct pci_controller_ops pseries_pci_controller_ops = {
	.probe_mode		= pSeries_pci_probe_mode,
};

define_machine(pseries) {
	.name			= "pSeries",
	.probe			= pSeries_probe,
	.setup_arch		= pSeries_setup_arch,
	.init_IRQ		= pseries_init_irq,
	.show_cpuinfo		= pSeries_show_cpuinfo,
	.log_error		= pSeries_log_error,
	.pcibios_fixup		= pSeries_final_fixup,
	.restart		= rtas_restart,
	.halt			= rtas_halt,
	.panic			= pseries_panic,
	.get_boot_time		= rtas_get_boot_time,
	.get_rtc_time		= rtas_get_rtc_time,
	.set_rtc_time		= rtas_set_rtc_time,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= rtas_progress,
	.system_reset_exception = pSeries_system_reset_exception,
	.machine_check_early	= pseries_machine_check_realmode,
	.machine_check_exception = pSeries_machine_check_exception,
#ifdef CONFIG_KEXEC_CORE
	.machine_kexec          = pSeries_machine_kexec,
	.kexec_cpu_down         = pseries_kexec_cpu_down,
#endif
#ifdef CONFIG_MEMORY_HOTPLUG_SPARSE
	.memory_block_size	= pseries_memory_block_size,
#endif
};
{
	set_arch_panic_timeout(10, ARCH_PANIC_TIMEOUT);

	/* Discover PIC type and setup ppc_md accordingly */
	smp_init_pseries();


	if (radix_enabled() && !mmu_has_feature(MMU_FTR_GTSE))
		if (!firmware_has_feature(FW_FEATURE_RPT_INVALIDATE))
			panic("BUG: Radix support requires either GTSE or RPT_INVALIDATE\n");


	/* openpic global configuration register (64-bit format). */
	/* openpic Interrupt Source Unit pointer (64-bit format). */
	/* python0 facility area (mmio) (64-bit format) REAL address. */

	/* init to some ~sane value until calibrate_delay() runs */
	loops_per_jiffy = 50000000;

	fwnmi_init();

	pseries_setup_security_mitigations();
	pseries_lpar_read_hblkrm_characteristics();

	/* By default, only probe PCI (can be overridden by rtas_pci) */
	pci_add_flags(PCI_PROBE_ONLY);

	/* Find and initialize PCI host bridges */
	init_pci_config_tokens();
	find_and_init_phbs();
	of_reconfig_notifier_register(&pci_dn_reconfig_nb);

	pSeries_nvram_init();

	if (firmware_has_feature(FW_FEATURE_LPAR)) {
		vpa_init(boot_cpuid);

		if (lppaca_shared_proc(get_lppaca())) {
			static_branch_enable(&shared_processor);
			pv_spinlocks_init();
		}

		ppc_md.power_save = pseries_lpar_idle;
		ppc_md.enable_pmcs = pseries_lpar_enable_pmcs;
#ifdef CONFIG_PCI_IOV
		ppc_md.pcibios_fixup_resources =
			pseries_pci_fixup_resources;
		ppc_md.pcibios_fixup_sriov =
			pseries_pci_fixup_iov_resources;
		ppc_md.pcibios_iov_resource_alignment =
			pseries_pci_iov_resource_alignment;
#endif
	} else {
		/* No special idle routine */
		ppc_md.enable_pmcs = power4_enable_pmcs;
	}

	ppc_md.pcibios_root_bridge_prepare = pseries_root_bridge_prepare;

	if (swiotlb_force == SWIOTLB_FORCE)
		ppc_swiotlb_enable = 1;
}