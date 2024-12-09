static struct pci_driver snbep_uncore_pci_driver = {
	.name		= "snbep_uncore",
	.id_table	= snbep_uncore_pci_ids,
};

/*
 * build pci bus to socket mapping
 */
static int snbep_pci2phy_map_init(int devid, int nodeid_loc, int idmap_loc, bool reverse)
{
	struct pci_dev *ubox_dev = NULL;
	int i, bus, nodeid, segment;
	struct pci2phy_map *map;
	int err = 0;
	u32 config = 0;

	while (1) {
		/* find the UBOX device */
		ubox_dev = pci_get_device(PCI_VENDOR_ID_INTEL, devid, ubox_dev);
		if (!ubox_dev)
			break;
		bus = ubox_dev->bus->number;
		/* get the Node ID of the local register */
		err = pci_read_config_dword(ubox_dev, nodeid_loc, &config);
		if (err)
			break;
		nodeid = config;
		/* get the Node ID mapping */
		err = pci_read_config_dword(ubox_dev, idmap_loc, &config);
		if (err)
			break;

		segment = pci_domain_nr(ubox_dev->bus);
		raw_spin_lock(&pci2phy_map_lock);
		map = __find_pci2phy_map(segment);
		if (!map) {
			raw_spin_unlock(&pci2phy_map_lock);
			err = -ENOMEM;
			break;
		}

		/*
		 * every three bits in the Node ID mapping register maps
		 * to a particular node.
		 */
		for (i = 0; i < 8; i++) {
			if (nodeid == ((config >> (3 * i)) & 0x7)) {
				map->pbus_to_physid[bus] = i;
				break;
			}
		}
		raw_spin_unlock(&pci2phy_map_lock);
	}

	if (!err) {
		/*
		 * For PCI bus with no UBOX device, find the next bus
		 * that has UBOX device and use its mapping.
		 */
		raw_spin_lock(&pci2phy_map_lock);
		list_for_each_entry(map, &pci2phy_map_head, list) {
			i = -1;
			if (reverse) {
				for (bus = 255; bus >= 0; bus--) {
					if (map->pbus_to_physid[bus] >= 0)
						i = map->pbus_to_physid[bus];
					else
						map->pbus_to_physid[bus] = i;
				}
			} else {
				for (bus = 0; bus <= 255; bus++) {
					if (map->pbus_to_physid[bus] >= 0)
						i = map->pbus_to_physid[bus];
					else
						map->pbus_to_physid[bus] = i;
				}
			}
		}
		raw_spin_unlock(&pci2phy_map_lock);
	}

	pci_dev_put(ubox_dev);

	return err ? pcibios_err_to_errno(err) : 0;
}

int snbep_uncore_pci_init(void)
{
	int ret = snbep_pci2phy_map_init(0x3ce0, SNBEP_CPUNODEID, SNBEP_GIDNIDMAP, true);
	if (ret)
		return ret;
	uncore_pci_uncores = snbep_pci_uncores;
	uncore_pci_driver = &snbep_uncore_pci_driver;
	return 0;
}
/* end of Sandy Bridge-EP uncore support */

/* IvyTown uncore support */
static void ivbep_uncore_msr_init_box(struct intel_uncore_box *box)
{
	unsigned msr = uncore_msr_box_ctl(box);
	if (msr)
		wrmsrl(msr, IVBEP_PMON_BOX_CTL_INT);
}

static void ivbep_uncore_pci_init_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;

	pci_write_config_dword(pdev, SNBEP_PCI_PMON_BOX_CTL, IVBEP_PMON_BOX_CTL_INT);
}

#define IVBEP_UNCORE_MSR_OPS_COMMON_INIT()			\
	.init_box	= ivbep_uncore_msr_init_box,		\
	.disable_box	= snbep_uncore_msr_disable_box,		\
	.enable_box	= snbep_uncore_msr_enable_box,		\
	.disable_event	= snbep_uncore_msr_disable_event,	\
	.enable_event	= snbep_uncore_msr_enable_event,	\
	.read_counter	= uncore_msr_read_counter

static struct intel_uncore_ops ivbep_uncore_msr_ops = {
	IVBEP_UNCORE_MSR_OPS_COMMON_INIT(),
};

static struct intel_uncore_ops ivbep_uncore_pci_ops = {
	.init_box	= ivbep_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= snbep_uncore_pci_disable_event,
	.enable_event	= snbep_uncore_pci_enable_event,
	.read_counter	= snbep_uncore_pci_read_counter,
};

#define IVBEP_UNCORE_PCI_COMMON_INIT()				\
	.perf_ctr	= SNBEP_PCI_PMON_CTR0,			\
	.event_ctl	= SNBEP_PCI_PMON_CTL0,			\
	.event_mask	= IVBEP_PMON_RAW_EVENT_MASK,		\
	.box_ctl	= SNBEP_PCI_PMON_BOX_CTL,		\
	.ops		= &ivbep_uncore_pci_ops,			\
	.format_group	= &ivbep_uncore_format_group

static struct attribute *ivbep_uncore_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	NULL,
};

static struct attribute *ivbep_uncore_ubox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh5.attr,
	NULL,
};

static struct attribute *ivbep_uncore_cbox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_thresh8.attr,
	&format_attr_filter_tid.attr,
	&format_attr_filter_link.attr,
	&format_attr_filter_state2.attr,
	&format_attr_filter_nid2.attr,
	&format_attr_filter_opc2.attr,
	&format_attr_filter_nc.attr,
	&format_attr_filter_c6.attr,
	&format_attr_filter_isoc.attr,
	NULL,
};

static struct attribute *ivbep_uncore_pcu_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_occ_sel.attr,
	&format_attr_edge.attr,
	&format_attr_thresh5.attr,
	&format_attr_occ_invert.attr,
	&format_attr_occ_edge.attr,
	&format_attr_filter_band0.attr,
	&format_attr_filter_band1.attr,
	&format_attr_filter_band2.attr,
	&format_attr_filter_band3.attr,
	NULL,
};

static struct attribute *ivbep_uncore_qpi_formats_attr[] = {
	&format_attr_event_ext.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_thresh8.attr,
	&format_attr_match_rds.attr,
	&format_attr_match_rnid30.attr,
	&format_attr_match_rnid4.attr,
	&format_attr_match_dnid.attr,
	&format_attr_match_mc.attr,
	&format_attr_match_opc.attr,
	&format_attr_match_vnw.attr,
	&format_attr_match0.attr,
	&format_attr_match1.attr,
	&format_attr_mask_rds.attr,
	&format_attr_mask_rnid30.attr,
	&format_attr_mask_rnid4.attr,
	&format_attr_mask_dnid.attr,
	&format_attr_mask_mc.attr,
	&format_attr_mask_opc.attr,
	&format_attr_mask_vnw.attr,
	&format_attr_mask0.attr,
	&format_attr_mask1.attr,
	NULL,
};

static const struct attribute_group ivbep_uncore_format_group = {
	.name = "format",
	.attrs = ivbep_uncore_formats_attr,
};

static const struct attribute_group ivbep_uncore_ubox_format_group = {
	.name = "format",
	.attrs = ivbep_uncore_ubox_formats_attr,
};

static const struct attribute_group ivbep_uncore_cbox_format_group = {
	.name = "format",
	.attrs = ivbep_uncore_cbox_formats_attr,
};

static const struct attribute_group ivbep_uncore_pcu_format_group = {
	.name = "format",
	.attrs = ivbep_uncore_pcu_formats_attr,
};

static const struct attribute_group ivbep_uncore_qpi_format_group = {
	.name = "format",
	.attrs = ivbep_uncore_qpi_formats_attr,
};

static struct intel_uncore_type ivbep_uncore_ubox = {
	.name		= "ubox",
	.num_counters   = 2,
	.num_boxes	= 1,
	.perf_ctr_bits	= 44,
	.fixed_ctr_bits	= 48,
	.perf_ctr	= SNBEP_U_MSR_PMON_CTR0,
	.event_ctl	= SNBEP_U_MSR_PMON_CTL0,
	.event_mask	= IVBEP_U_MSR_PMON_RAW_EVENT_MASK,
	.fixed_ctr	= SNBEP_U_MSR_PMON_UCLK_FIXED_CTR,
	.fixed_ctl	= SNBEP_U_MSR_PMON_UCLK_FIXED_CTL,
	.ops		= &ivbep_uncore_msr_ops,
	.format_group	= &ivbep_uncore_ubox_format_group,
};

static struct extra_reg ivbep_uncore_cbox_extra_regs[] = {
	SNBEP_CBO_EVENT_EXTRA_REG(SNBEP_CBO_PMON_CTL_TID_EN,
				  SNBEP_CBO_PMON_CTL_TID_EN, 0x1),
	SNBEP_CBO_EVENT_EXTRA_REG(0x1031, 0x10ff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x1134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4134, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5134, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0334, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4334, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0534, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4534, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0934, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4934, 0xffff, 0xc),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4135, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4335, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4435, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4835, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a35, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5035, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4136, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4336, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4436, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4836, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a36, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5036, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4037, 0x40ff, 0x8),
	EVENT_EXTRA_END
};

static u64 ivbep_cbox_filter_mask(int fields)
{
	u64 mask = 0;

	if (fields & 0x1)
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_TID;
	if (fields & 0x2)
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_LINK;
	if (fields & 0x4)
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_STATE;
	if (fields & 0x8)
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_NID;
	if (fields & 0x10) {
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_OPC;
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_NC;
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_C6;
		mask |= IVBEP_CB0_MSR_PMON_BOX_FILTER_ISOC;
	}

	return mask;
}

static struct event_constraint *
ivbep_cbox_get_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	return __snbep_cbox_get_constraint(box, event, ivbep_cbox_filter_mask);
}

static int ivbep_cbox_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct extra_reg *er;
	int idx = 0;

	for (er = ivbep_uncore_cbox_extra_regs; er->msr; er++) {
		if (er->event != (event->hw.config & er->config_mask))
			continue;
		idx |= er->idx;
	}

	if (idx) {
		reg1->reg = SNBEP_C0_MSR_PMON_BOX_FILTER +
			SNBEP_CBO_MSR_OFFSET * box->pmu->pmu_idx;
		reg1->config = event->attr.config1 & ivbep_cbox_filter_mask(idx);
		reg1->idx = idx;
	}
	return 0;
}

static void ivbep_cbox_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;

	if (reg1->idx != EXTRA_REG_NONE) {
		u64 filter = uncore_shared_reg_config(box, 0);
		wrmsrl(reg1->reg, filter & 0xffffffff);
		wrmsrl(reg1->reg + 6, filter >> 32);
	}

	wrmsrl(hwc->config_base, hwc->config | SNBEP_PMON_CTL_EN);
}

static struct intel_uncore_ops ivbep_uncore_cbox_ops = {
	.init_box		= ivbep_uncore_msr_init_box,
	.disable_box		= snbep_uncore_msr_disable_box,
	.enable_box		= snbep_uncore_msr_enable_box,
	.disable_event		= snbep_uncore_msr_disable_event,
	.enable_event		= ivbep_cbox_enable_event,
	.read_counter		= uncore_msr_read_counter,
	.hw_config		= ivbep_cbox_hw_config,
	.get_constraint		= ivbep_cbox_get_constraint,
	.put_constraint		= snbep_cbox_put_constraint,
};

static struct intel_uncore_type ivbep_uncore_cbox = {
	.name			= "cbox",
	.num_counters		= 4,
	.num_boxes		= 15,
	.perf_ctr_bits		= 44,
	.event_ctl		= SNBEP_C0_MSR_PMON_CTL0,
	.perf_ctr		= SNBEP_C0_MSR_PMON_CTR0,
	.event_mask		= IVBEP_CBO_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_C0_MSR_PMON_BOX_CTL,
	.msr_offset		= SNBEP_CBO_MSR_OFFSET,
	.num_shared_regs	= 1,
	.constraints		= snbep_uncore_cbox_constraints,
	.ops			= &ivbep_uncore_cbox_ops,
	.format_group		= &ivbep_uncore_cbox_format_group,
};

static struct intel_uncore_ops ivbep_uncore_pcu_ops = {
	IVBEP_UNCORE_MSR_OPS_COMMON_INIT(),
	.hw_config		= snbep_pcu_hw_config,
	.get_constraint		= snbep_pcu_get_constraint,
	.put_constraint		= snbep_pcu_put_constraint,
};

static struct intel_uncore_type ivbep_uncore_pcu = {
	.name			= "pcu",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCU_MSR_PMON_CTR0,
	.event_ctl		= SNBEP_PCU_MSR_PMON_CTL0,
	.event_mask		= IVBEP_PCU_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCU_MSR_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &ivbep_uncore_pcu_ops,
	.format_group		= &ivbep_uncore_pcu_format_group,
};

static struct intel_uncore_type *ivbep_msr_uncores[] = {
	&ivbep_uncore_ubox,
	&ivbep_uncore_cbox,
	&ivbep_uncore_pcu,
	NULL,
};

void ivbep_uncore_cpu_init(void)
{
	if (ivbep_uncore_cbox.num_boxes > boot_cpu_data.x86_max_cores)
		ivbep_uncore_cbox.num_boxes = boot_cpu_data.x86_max_cores;
	uncore_msr_uncores = ivbep_msr_uncores;
}

static struct intel_uncore_type ivbep_uncore_ha = {
	.name		= "ha",
	.num_counters   = 4,
	.num_boxes	= 2,
	.perf_ctr_bits	= 48,
	IVBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type ivbep_uncore_imc = {
	.name		= "imc",
	.num_counters   = 4,
	.num_boxes	= 8,
	.perf_ctr_bits	= 48,
	.fixed_ctr_bits	= 48,
	.fixed_ctr	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTR,
	.fixed_ctl	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTL,
	.event_descs	= snbep_uncore_imc_events,
	IVBEP_UNCORE_PCI_COMMON_INIT(),
};

/* registers in IRP boxes are not properly aligned */
static unsigned ivbep_uncore_irp_ctls[] = {0xd8, 0xdc, 0xe0, 0xe4};
static unsigned ivbep_uncore_irp_ctrs[] = {0xa0, 0xb0, 0xb8, 0xc0};

static void ivbep_uncore_irp_enable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;

	pci_write_config_dword(pdev, ivbep_uncore_irp_ctls[hwc->idx],
			       hwc->config | SNBEP_PMON_CTL_EN);
}

static void ivbep_uncore_irp_disable_event(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;

	pci_write_config_dword(pdev, ivbep_uncore_irp_ctls[hwc->idx], hwc->config);
}

static u64 ivbep_uncore_irp_read_counter(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;
	u64 count = 0;

	pci_read_config_dword(pdev, ivbep_uncore_irp_ctrs[hwc->idx], (u32 *)&count);
	pci_read_config_dword(pdev, ivbep_uncore_irp_ctrs[hwc->idx] + 4, (u32 *)&count + 1);

	return count;
}

static struct intel_uncore_ops ivbep_uncore_irp_ops = {
	.init_box	= ivbep_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= ivbep_uncore_irp_disable_event,
	.enable_event	= ivbep_uncore_irp_enable_event,
	.read_counter	= ivbep_uncore_irp_read_counter,
};

static struct intel_uncore_type ivbep_uncore_irp = {
	.name			= "irp",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.event_mask		= IVBEP_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.ops			= &ivbep_uncore_irp_ops,
	.format_group		= &ivbep_uncore_format_group,
};

static struct intel_uncore_ops ivbep_uncore_qpi_ops = {
	.init_box	= ivbep_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= snbep_uncore_pci_disable_event,
	.enable_event	= snbep_qpi_enable_event,
	.read_counter	= snbep_uncore_pci_read_counter,
	.hw_config	= snbep_qpi_hw_config,
	.get_constraint	= uncore_get_constraint,
	.put_constraint	= uncore_put_constraint,
};

static struct intel_uncore_type ivbep_uncore_qpi = {
	.name			= "qpi",
	.num_counters		= 4,
	.num_boxes		= 3,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCI_PMON_CTR0,
	.event_ctl		= SNBEP_PCI_PMON_CTL0,
	.event_mask		= IVBEP_QPI_PCI_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &ivbep_uncore_qpi_ops,
	.format_group		= &ivbep_uncore_qpi_format_group,
};

static struct intel_uncore_type ivbep_uncore_r2pcie = {
	.name		= "r2pcie",
	.num_counters   = 4,
	.num_boxes	= 1,
	.perf_ctr_bits	= 44,
	.constraints	= snbep_uncore_r2pcie_constraints,
	IVBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type ivbep_uncore_r3qpi = {
	.name		= "r3qpi",
	.num_counters   = 3,
	.num_boxes	= 2,
	.perf_ctr_bits	= 44,
	.constraints	= snbep_uncore_r3qpi_constraints,
	IVBEP_UNCORE_PCI_COMMON_INIT(),
};

enum {
	IVBEP_PCI_UNCORE_HA,
	IVBEP_PCI_UNCORE_IMC,
	IVBEP_PCI_UNCORE_IRP,
	IVBEP_PCI_UNCORE_QPI,
	IVBEP_PCI_UNCORE_R2PCIE,
	IVBEP_PCI_UNCORE_R3QPI,
};

static struct intel_uncore_type *ivbep_pci_uncores[] = {
	[IVBEP_PCI_UNCORE_HA]	= &ivbep_uncore_ha,
	[IVBEP_PCI_UNCORE_IMC]	= &ivbep_uncore_imc,
	[IVBEP_PCI_UNCORE_IRP]	= &ivbep_uncore_irp,
	[IVBEP_PCI_UNCORE_QPI]	= &ivbep_uncore_qpi,
	[IVBEP_PCI_UNCORE_R2PCIE]	= &ivbep_uncore_r2pcie,
	[IVBEP_PCI_UNCORE_R3QPI]	= &ivbep_uncore_r3qpi,
	NULL,
};

static const struct pci_device_id ivbep_uncore_pci_ids[] = {
	{ /* Home Agent 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe30),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_HA, 0),
	},
	{ /* Home Agent 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe38),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_HA, 1),
	},
	{ /* MC0 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb4),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 0),
	},
	{ /* MC0 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb5),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 1),
	},
	{ /* MC0 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb0),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 2),
	},
	{ /* MC0 Channel 4 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xeb1),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 3),
	},
	{ /* MC1 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef4),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 4),
	},
	{ /* MC1 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef5),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 5),
	},
	{ /* MC1 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef0),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 6),
	},
	{ /* MC1 Channel 4 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xef1),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IMC, 7),
	},
	{ /* IRP */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe39),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_IRP, 0),
	},
	{ /* QPI0 Port 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe32),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_QPI, 0),
	},
	{ /* QPI0 Port 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe33),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_QPI, 1),
	},
	{ /* QPI1 Port 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe3a),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_QPI, 2),
	},
	{ /* R2PCIe */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe34),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_R2PCIE, 0),
	},
	{ /* R3QPI0 Link 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe36),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_R3QPI, 0),
	},
	{ /* R3QPI0 Link 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe37),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_R3QPI, 1),
	},
	{ /* R3QPI1 Link 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe3e),
		.driver_data = UNCORE_PCI_DEV_DATA(IVBEP_PCI_UNCORE_R3QPI, 2),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe86),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT0_FILTER),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xe96),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT1_FILTER),
	},
	{ /* end: all zeroes */ }
};

static struct pci_driver ivbep_uncore_pci_driver = {
	.name		= "ivbep_uncore",
	.id_table	= ivbep_uncore_pci_ids,
};

int ivbep_uncore_pci_init(void)
{
	int ret = snbep_pci2phy_map_init(0x0e1e, SNBEP_CPUNODEID, SNBEP_GIDNIDMAP, true);
	if (ret)
		return ret;
	uncore_pci_uncores = ivbep_pci_uncores;
	uncore_pci_driver = &ivbep_uncore_pci_driver;
	return 0;
}
/* end of IvyTown uncore support */

/* KNL uncore support */
static struct attribute *knl_uncore_ubox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_inv.attr,
	&format_attr_thresh5.attr,
	NULL,
};

static const struct attribute_group knl_uncore_ubox_format_group = {
	.name = "format",
	.attrs = knl_uncore_ubox_formats_attr,
};

static struct intel_uncore_type knl_uncore_ubox = {
	.name			= "ubox",
	.num_counters		= 2,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= HSWEP_U_MSR_PMON_CTR0,
	.event_ctl		= HSWEP_U_MSR_PMON_CTL0,
	.event_mask		= KNL_U_MSR_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTR,
	.fixed_ctl		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTL,
	.ops			= &snbep_uncore_msr_ops,
	.format_group		= &knl_uncore_ubox_format_group,
};

static struct attribute *knl_uncore_cha_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_qor.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	&format_attr_filter_tid4.attr,
	&format_attr_filter_link3.attr,
	&format_attr_filter_state4.attr,
	&format_attr_filter_local.attr,
	&format_attr_filter_all_op.attr,
	&format_attr_filter_nnm.attr,
	&format_attr_filter_opc3.attr,
	&format_attr_filter_nc.attr,
	&format_attr_filter_isoc.attr,
	NULL,
};

static const struct attribute_group knl_uncore_cha_format_group = {
	.name = "format",
	.attrs = knl_uncore_cha_formats_attr,
};

static struct event_constraint knl_uncore_cha_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x11, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x1f, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x1),
	EVENT_CONSTRAINT_END
};

static struct extra_reg knl_uncore_cha_extra_regs[] = {
	SNBEP_CBO_EVENT_EXTRA_REG(SNBEP_CBO_PMON_CTL_TID_EN,
				  SNBEP_CBO_PMON_CTL_TID_EN, 0x1),
	SNBEP_CBO_EVENT_EXTRA_REG(0x3d, 0xff, 0x2),
	SNBEP_CBO_EVENT_EXTRA_REG(0x35, 0xff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x36, 0xff, 0x4),
	EVENT_EXTRA_END
};

static u64 knl_cha_filter_mask(int fields)
{
	u64 mask = 0;

	if (fields & 0x1)
		mask |= KNL_CHA_MSR_PMON_BOX_FILTER_TID;
	if (fields & 0x2)
		mask |= KNL_CHA_MSR_PMON_BOX_FILTER_STATE;
	if (fields & 0x4)
		mask |= KNL_CHA_MSR_PMON_BOX_FILTER_OP;
	return mask;
}

static struct event_constraint *
knl_cha_get_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	return __snbep_cbox_get_constraint(box, event, knl_cha_filter_mask);
}

static int knl_cha_hw_config(struct intel_uncore_box *box,
			     struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct extra_reg *er;
	int idx = 0;

	for (er = knl_uncore_cha_extra_regs; er->msr; er++) {
		if (er->event != (event->hw.config & er->config_mask))
			continue;
		idx |= er->idx;
	}

	if (idx) {
		reg1->reg = HSWEP_C0_MSR_PMON_BOX_FILTER0 +
			    KNL_CHA_MSR_OFFSET * box->pmu->pmu_idx;
		reg1->config = event->attr.config1 & knl_cha_filter_mask(idx);

		reg1->config |= KNL_CHA_MSR_PMON_BOX_FILTER_REMOTE_NODE;
		reg1->config |= KNL_CHA_MSR_PMON_BOX_FILTER_LOCAL_NODE;
		reg1->config |= KNL_CHA_MSR_PMON_BOX_FILTER_NNC;
		reg1->idx = idx;
	}
	return 0;
}

static void hswep_cbox_enable_event(struct intel_uncore_box *box,
				    struct perf_event *event);

static struct intel_uncore_ops knl_uncore_cha_ops = {
	.init_box		= snbep_uncore_msr_init_box,
	.disable_box		= snbep_uncore_msr_disable_box,
	.enable_box		= snbep_uncore_msr_enable_box,
	.disable_event		= snbep_uncore_msr_disable_event,
	.enable_event		= hswep_cbox_enable_event,
	.read_counter		= uncore_msr_read_counter,
	.hw_config		= knl_cha_hw_config,
	.get_constraint		= knl_cha_get_constraint,
	.put_constraint		= snbep_cbox_put_constraint,
};

static struct intel_uncore_type knl_uncore_cha = {
	.name			= "cha",
	.num_counters		= 4,
	.num_boxes		= 38,
	.perf_ctr_bits		= 48,
	.event_ctl		= HSWEP_C0_MSR_PMON_CTL0,
	.perf_ctr		= HSWEP_C0_MSR_PMON_CTR0,
	.event_mask		= KNL_CHA_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_C0_MSR_PMON_BOX_CTL,
	.msr_offset		= KNL_CHA_MSR_OFFSET,
	.num_shared_regs	= 1,
	.constraints		= knl_uncore_cha_constraints,
	.ops			= &knl_uncore_cha_ops,
	.format_group		= &knl_uncore_cha_format_group,
};

static struct attribute *knl_uncore_pcu_formats_attr[] = {
	&format_attr_event2.attr,
	&format_attr_use_occ_ctr.attr,
	&format_attr_occ_sel.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_inv.attr,
	&format_attr_thresh6.attr,
	&format_attr_occ_invert.attr,
	&format_attr_occ_edge_det.attr,
	NULL,
};

static const struct attribute_group knl_uncore_pcu_format_group = {
	.name = "format",
	.attrs = knl_uncore_pcu_formats_attr,
};

static struct intel_uncore_type knl_uncore_pcu = {
	.name			= "pcu",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.perf_ctr		= HSWEP_PCU_MSR_PMON_CTR0,
	.event_ctl		= HSWEP_PCU_MSR_PMON_CTL0,
	.event_mask		= KNL_PCU_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_PCU_MSR_PMON_BOX_CTL,
	.ops			= &snbep_uncore_msr_ops,
	.format_group		= &knl_uncore_pcu_format_group,
};

static struct intel_uncore_type *knl_msr_uncores[] = {
	&knl_uncore_ubox,
	&knl_uncore_cha,
	&knl_uncore_pcu,
	NULL,
};

void knl_uncore_cpu_init(void)
{
	uncore_msr_uncores = knl_msr_uncores;
}

static void knl_uncore_imc_enable_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;
	int box_ctl = uncore_pci_box_ctl(box);

	pci_write_config_dword(pdev, box_ctl, 0);
}

static void knl_uncore_imc_enable_event(struct intel_uncore_box *box,
					struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;

	if ((event->attr.config & SNBEP_PMON_CTL_EV_SEL_MASK)
							== UNCORE_FIXED_EVENT)
		pci_write_config_dword(pdev, hwc->config_base,
				       hwc->config | KNL_PMON_FIXED_CTL_EN);
	else
		pci_write_config_dword(pdev, hwc->config_base,
				       hwc->config | SNBEP_PMON_CTL_EN);
}

static struct intel_uncore_ops knl_uncore_imc_ops = {
	.init_box	= snbep_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= knl_uncore_imc_enable_box,
	.read_counter	= snbep_uncore_pci_read_counter,
	.enable_event	= knl_uncore_imc_enable_event,
	.disable_event	= snbep_uncore_pci_disable_event,
};

static struct intel_uncore_type knl_uncore_imc_uclk = {
	.name			= "imc_uclk",
	.num_counters		= 4,
	.num_boxes		= 2,
	.perf_ctr_bits		= 48,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= KNL_UCLK_MSR_PMON_CTR0_LOW,
	.event_ctl		= KNL_UCLK_MSR_PMON_CTL0,
	.event_mask		= SNBEP_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= KNL_UCLK_MSR_PMON_UCLK_FIXED_LOW,
	.fixed_ctl		= KNL_UCLK_MSR_PMON_UCLK_FIXED_CTL,
	.box_ctl		= KNL_UCLK_MSR_PMON_BOX_CTL,
	.ops			= &knl_uncore_imc_ops,
	.format_group		= &snbep_uncore_format_group,
};

static struct intel_uncore_type knl_uncore_imc_dclk = {
	.name			= "imc",
	.num_counters		= 4,
	.num_boxes		= 6,
	.perf_ctr_bits		= 48,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= KNL_MC0_CH0_MSR_PMON_CTR0_LOW,
	.event_ctl		= KNL_MC0_CH0_MSR_PMON_CTL0,
	.event_mask		= SNBEP_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= KNL_MC0_CH0_MSR_PMON_FIXED_LOW,
	.fixed_ctl		= KNL_MC0_CH0_MSR_PMON_FIXED_CTL,
	.box_ctl		= KNL_MC0_CH0_MSR_PMON_BOX_CTL,
	.ops			= &knl_uncore_imc_ops,
	.format_group		= &snbep_uncore_format_group,
};

static struct intel_uncore_type knl_uncore_edc_uclk = {
	.name			= "edc_uclk",
	.num_counters		= 4,
	.num_boxes		= 8,
	.perf_ctr_bits		= 48,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= KNL_UCLK_MSR_PMON_CTR0_LOW,
	.event_ctl		= KNL_UCLK_MSR_PMON_CTL0,
	.event_mask		= SNBEP_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= KNL_UCLK_MSR_PMON_UCLK_FIXED_LOW,
	.fixed_ctl		= KNL_UCLK_MSR_PMON_UCLK_FIXED_CTL,
	.box_ctl		= KNL_UCLK_MSR_PMON_BOX_CTL,
	.ops			= &knl_uncore_imc_ops,
	.format_group		= &snbep_uncore_format_group,
};

static struct intel_uncore_type knl_uncore_edc_eclk = {
	.name			= "edc_eclk",
	.num_counters		= 4,
	.num_boxes		= 8,
	.perf_ctr_bits		= 48,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= KNL_EDC0_ECLK_MSR_PMON_CTR0_LOW,
	.event_ctl		= KNL_EDC0_ECLK_MSR_PMON_CTL0,
	.event_mask		= SNBEP_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= KNL_EDC0_ECLK_MSR_PMON_ECLK_FIXED_LOW,
	.fixed_ctl		= KNL_EDC0_ECLK_MSR_PMON_ECLK_FIXED_CTL,
	.box_ctl		= KNL_EDC0_ECLK_MSR_PMON_BOX_CTL,
	.ops			= &knl_uncore_imc_ops,
	.format_group		= &snbep_uncore_format_group,
};

static struct event_constraint knl_uncore_m2pcie_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x23, 0x3),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type knl_uncore_m2pcie = {
	.name		= "m2pcie",
	.num_counters   = 4,
	.num_boxes	= 1,
	.perf_ctr_bits	= 48,
	.constraints	= knl_uncore_m2pcie_constraints,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct attribute *knl_uncore_irp_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_qor.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	NULL,
};

static const struct attribute_group knl_uncore_irp_format_group = {
	.name = "format",
	.attrs = knl_uncore_irp_formats_attr,
};

static struct intel_uncore_type knl_uncore_irp = {
	.name			= "irp",
	.num_counters		= 2,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCI_PMON_CTR0,
	.event_ctl		= SNBEP_PCI_PMON_CTL0,
	.event_mask		= KNL_IRP_PCI_PMON_RAW_EVENT_MASK,
	.box_ctl		= KNL_IRP_PCI_PMON_BOX_CTL,
	.ops			= &snbep_uncore_pci_ops,
	.format_group		= &knl_uncore_irp_format_group,
};

enum {
	KNL_PCI_UNCORE_MC_UCLK,
	KNL_PCI_UNCORE_MC_DCLK,
	KNL_PCI_UNCORE_EDC_UCLK,
	KNL_PCI_UNCORE_EDC_ECLK,
	KNL_PCI_UNCORE_M2PCIE,
	KNL_PCI_UNCORE_IRP,
};

static struct intel_uncore_type *knl_pci_uncores[] = {
	[KNL_PCI_UNCORE_MC_UCLK]	= &knl_uncore_imc_uclk,
	[KNL_PCI_UNCORE_MC_DCLK]	= &knl_uncore_imc_dclk,
	[KNL_PCI_UNCORE_EDC_UCLK]	= &knl_uncore_edc_uclk,
	[KNL_PCI_UNCORE_EDC_ECLK]	= &knl_uncore_edc_eclk,
	[KNL_PCI_UNCORE_M2PCIE]		= &knl_uncore_m2pcie,
	[KNL_PCI_UNCORE_IRP]		= &knl_uncore_irp,
	NULL,
};

/*
 * KNL uses a common PCI device ID for multiple instances of an Uncore PMU
 * device type. prior to KNL, each instance of a PMU device type had a unique
 * device ID.
 *
 *	PCI Device ID	Uncore PMU Devices
 *	----------------------------------
 *	0x7841		MC0 UClk, MC1 UClk
 *	0x7843		MC0 DClk CH 0, MC0 DClk CH 1, MC0 DClk CH 2,
 *			MC1 DClk CH 0, MC1 DClk CH 1, MC1 DClk CH 2
 *	0x7833		EDC0 UClk, EDC1 UClk, EDC2 UClk, EDC3 UClk,
 *			EDC4 UClk, EDC5 UClk, EDC6 UClk, EDC7 UClk
 *	0x7835		EDC0 EClk, EDC1 EClk, EDC2 EClk, EDC3 EClk,
 *			EDC4 EClk, EDC5 EClk, EDC6 EClk, EDC7 EClk
 *	0x7817		M2PCIe
 *	0x7814		IRP
*/

static const struct pci_device_id knl_uncore_pci_ids[] = {
	{ /* MC0 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7841),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(10, 0, KNL_PCI_UNCORE_MC_UCLK, 0),
	},
	{ /* MC1 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7841),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(11, 0, KNL_PCI_UNCORE_MC_UCLK, 1),
	},
	{ /* MC0 DClk CH 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7843),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(8, 2, KNL_PCI_UNCORE_MC_DCLK, 0),
	},
	{ /* MC0 DClk CH 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7843),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(8, 3, KNL_PCI_UNCORE_MC_DCLK, 1),
	},
	{ /* MC0 DClk CH 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7843),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(8, 4, KNL_PCI_UNCORE_MC_DCLK, 2),
	},
	{ /* MC1 DClk CH 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7843),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(9, 2, KNL_PCI_UNCORE_MC_DCLK, 3),
	},
	{ /* MC1 DClk CH 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7843),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(9, 3, KNL_PCI_UNCORE_MC_DCLK, 4),
	},
	{ /* MC1 DClk CH 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7843),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(9, 4, KNL_PCI_UNCORE_MC_DCLK, 5),
	},
	{ /* EDC0 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(15, 0, KNL_PCI_UNCORE_EDC_UCLK, 0),
	},
	{ /* EDC1 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(16, 0, KNL_PCI_UNCORE_EDC_UCLK, 1),
	},
	{ /* EDC2 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(17, 0, KNL_PCI_UNCORE_EDC_UCLK, 2),
	},
	{ /* EDC3 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(18, 0, KNL_PCI_UNCORE_EDC_UCLK, 3),
	},
	{ /* EDC4 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(19, 0, KNL_PCI_UNCORE_EDC_UCLK, 4),
	},
	{ /* EDC5 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(20, 0, KNL_PCI_UNCORE_EDC_UCLK, 5),
	},
	{ /* EDC6 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(21, 0, KNL_PCI_UNCORE_EDC_UCLK, 6),
	},
	{ /* EDC7 UClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7833),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(22, 0, KNL_PCI_UNCORE_EDC_UCLK, 7),
	},
	{ /* EDC0 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(24, 2, KNL_PCI_UNCORE_EDC_ECLK, 0),
	},
	{ /* EDC1 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(25, 2, KNL_PCI_UNCORE_EDC_ECLK, 1),
	},
	{ /* EDC2 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(26, 2, KNL_PCI_UNCORE_EDC_ECLK, 2),
	},
	{ /* EDC3 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(27, 2, KNL_PCI_UNCORE_EDC_ECLK, 3),
	},
	{ /* EDC4 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(28, 2, KNL_PCI_UNCORE_EDC_ECLK, 4),
	},
	{ /* EDC5 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(29, 2, KNL_PCI_UNCORE_EDC_ECLK, 5),
	},
	{ /* EDC6 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(30, 2, KNL_PCI_UNCORE_EDC_ECLK, 6),
	},
	{ /* EDC7 EClk */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7835),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(31, 2, KNL_PCI_UNCORE_EDC_ECLK, 7),
	},
	{ /* M2PCIe */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7817),
		.driver_data = UNCORE_PCI_DEV_DATA(KNL_PCI_UNCORE_M2PCIE, 0),
	},
	{ /* IRP */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x7814),
		.driver_data = UNCORE_PCI_DEV_DATA(KNL_PCI_UNCORE_IRP, 0),
	},
	{ /* end: all zeroes */ }
};

static struct pci_driver knl_uncore_pci_driver = {
	.name		= "knl_uncore",
	.id_table	= knl_uncore_pci_ids,
};

int knl_uncore_pci_init(void)
{
	int ret;

	/* All KNL PCI based PMON units are on the same PCI bus except IRP */
	ret = snb_pci2phy_map_init(0x7814); /* IRP */
	if (ret)
		return ret;
	ret = snb_pci2phy_map_init(0x7817); /* M2PCIe */
	if (ret)
		return ret;
	uncore_pci_uncores = knl_pci_uncores;
	uncore_pci_driver = &knl_uncore_pci_driver;
	return 0;
}

/* end of KNL uncore support */

/* Haswell-EP uncore support */
static struct attribute *hswep_uncore_ubox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh5.attr,
	&format_attr_filter_tid2.attr,
	&format_attr_filter_cid.attr,
	NULL,
};

static const struct attribute_group hswep_uncore_ubox_format_group = {
	.name = "format",
	.attrs = hswep_uncore_ubox_formats_attr,
};

static int hswep_ubox_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	reg1->reg = HSWEP_U_MSR_PMON_FILTER;
	reg1->config = event->attr.config1 & HSWEP_U_MSR_PMON_BOX_FILTER_MASK;
	reg1->idx = 0;
	return 0;
}

static struct intel_uncore_ops hswep_uncore_ubox_ops = {
	SNBEP_UNCORE_MSR_OPS_COMMON_INIT(),
	.hw_config		= hswep_ubox_hw_config,
	.get_constraint		= uncore_get_constraint,
	.put_constraint		= uncore_put_constraint,
};

static struct intel_uncore_type hswep_uncore_ubox = {
	.name			= "ubox",
	.num_counters		= 2,
	.num_boxes		= 1,
	.perf_ctr_bits		= 44,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= HSWEP_U_MSR_PMON_CTR0,
	.event_ctl		= HSWEP_U_MSR_PMON_CTL0,
	.event_mask		= SNBEP_U_MSR_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTR,
	.fixed_ctl		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTL,
	.num_shared_regs	= 1,
	.ops			= &hswep_uncore_ubox_ops,
	.format_group		= &hswep_uncore_ubox_format_group,
};

static struct attribute *hswep_uncore_cbox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_thresh8.attr,
	&format_attr_filter_tid3.attr,
	&format_attr_filter_link2.attr,
	&format_attr_filter_state3.attr,
	&format_attr_filter_nid2.attr,
	&format_attr_filter_opc2.attr,
	&format_attr_filter_nc.attr,
	&format_attr_filter_c6.attr,
	&format_attr_filter_isoc.attr,
	NULL,
};

static const struct attribute_group hswep_uncore_cbox_format_group = {
	.name = "format",
	.attrs = hswep_uncore_cbox_formats_attr,
};

static struct event_constraint hswep_uncore_cbox_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x01, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x09, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x38, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x3b, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x3e, 0x1),
	EVENT_CONSTRAINT_END
};

static struct extra_reg hswep_uncore_cbox_extra_regs[] = {
	SNBEP_CBO_EVENT_EXTRA_REG(SNBEP_CBO_PMON_CTL_TID_EN,
				  SNBEP_CBO_PMON_CTL_TID_EN, 0x1),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0334, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0534, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0934, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x1134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4037, 0x40ff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4028, 0x40ff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4032, 0x40ff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4029, 0x40ff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4033, 0x40ff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x402A, 0x40ff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0135, 0xffff, 0x12),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4135, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4435, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4835, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5035, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4335, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a35, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8335, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8135, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4136, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4436, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4836, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4336, 0xffff, 0x18),
	SNBEP_CBO_EVENT_EXTRA_REG(0x4a36, 0xffff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8336, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x2136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x8136, 0xffff, 0x10),
	SNBEP_CBO_EVENT_EXTRA_REG(0x5036, 0xffff, 0x8),
	EVENT_EXTRA_END
};

static u64 hswep_cbox_filter_mask(int fields)
{
	u64 mask = 0;
	if (fields & 0x1)
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_TID;
	if (fields & 0x2)
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_LINK;
	if (fields & 0x4)
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_STATE;
	if (fields & 0x8)
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_NID;
	if (fields & 0x10) {
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_OPC;
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_NC;
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_C6;
		mask |= HSWEP_CB0_MSR_PMON_BOX_FILTER_ISOC;
	}
	return mask;
}

static struct event_constraint *
hswep_cbox_get_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	return __snbep_cbox_get_constraint(box, event, hswep_cbox_filter_mask);
}

static int hswep_cbox_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct extra_reg *er;
	int idx = 0;

	for (er = hswep_uncore_cbox_extra_regs; er->msr; er++) {
		if (er->event != (event->hw.config & er->config_mask))
			continue;
		idx |= er->idx;
	}

	if (idx) {
		reg1->reg = HSWEP_C0_MSR_PMON_BOX_FILTER0 +
			    HSWEP_CBO_MSR_OFFSET * box->pmu->pmu_idx;
		reg1->config = event->attr.config1 & hswep_cbox_filter_mask(idx);
		reg1->idx = idx;
	}
	return 0;
}

static void hswep_cbox_enable_event(struct intel_uncore_box *box,
				  struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;

	if (reg1->idx != EXTRA_REG_NONE) {
		u64 filter = uncore_shared_reg_config(box, 0);
		wrmsrl(reg1->reg, filter & 0xffffffff);
		wrmsrl(reg1->reg + 1, filter >> 32);
	}

	wrmsrl(hwc->config_base, hwc->config | SNBEP_PMON_CTL_EN);
}

static struct intel_uncore_ops hswep_uncore_cbox_ops = {
	.init_box		= snbep_uncore_msr_init_box,
	.disable_box		= snbep_uncore_msr_disable_box,
	.enable_box		= snbep_uncore_msr_enable_box,
	.disable_event		= snbep_uncore_msr_disable_event,
	.enable_event		= hswep_cbox_enable_event,
	.read_counter		= uncore_msr_read_counter,
	.hw_config		= hswep_cbox_hw_config,
	.get_constraint		= hswep_cbox_get_constraint,
	.put_constraint		= snbep_cbox_put_constraint,
};

static struct intel_uncore_type hswep_uncore_cbox = {
	.name			= "cbox",
	.num_counters		= 4,
	.num_boxes		= 18,
	.perf_ctr_bits		= 48,
	.event_ctl		= HSWEP_C0_MSR_PMON_CTL0,
	.perf_ctr		= HSWEP_C0_MSR_PMON_CTR0,
	.event_mask		= SNBEP_CBO_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_C0_MSR_PMON_BOX_CTL,
	.msr_offset		= HSWEP_CBO_MSR_OFFSET,
	.num_shared_regs	= 1,
	.constraints		= hswep_uncore_cbox_constraints,
	.ops			= &hswep_uncore_cbox_ops,
	.format_group		= &hswep_uncore_cbox_format_group,
};

/*
 * Write SBOX Initialization register bit by bit to avoid spurious #GPs
 */
static void hswep_uncore_sbox_msr_init_box(struct intel_uncore_box *box)
{
	unsigned msr = uncore_msr_box_ctl(box);

	if (msr) {
		u64 init = SNBEP_PMON_BOX_CTL_INT;
		u64 flags = 0;
		int i;

		for_each_set_bit(i, (unsigned long *)&init, 64) {
			flags |= (1ULL << i);
			wrmsrl(msr, flags);
		}
	}
}

static struct intel_uncore_ops hswep_uncore_sbox_msr_ops = {
	__SNBEP_UNCORE_MSR_OPS_COMMON_INIT(),
	.init_box		= hswep_uncore_sbox_msr_init_box
};

static struct attribute *hswep_uncore_sbox_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	NULL,
};

static const struct attribute_group hswep_uncore_sbox_format_group = {
	.name = "format",
	.attrs = hswep_uncore_sbox_formats_attr,
};

static struct intel_uncore_type hswep_uncore_sbox = {
	.name			= "sbox",
	.num_counters		= 4,
	.num_boxes		= 4,
	.perf_ctr_bits		= 44,
	.event_ctl		= HSWEP_S0_MSR_PMON_CTL0,
	.perf_ctr		= HSWEP_S0_MSR_PMON_CTR0,
	.event_mask		= HSWEP_S_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_S0_MSR_PMON_BOX_CTL,
	.msr_offset		= HSWEP_SBOX_MSR_OFFSET,
	.ops			= &hswep_uncore_sbox_msr_ops,
	.format_group		= &hswep_uncore_sbox_format_group,
};

static int hswep_pcu_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	struct hw_perf_event_extra *reg1 = &hwc->extra_reg;
	int ev_sel = hwc->config & SNBEP_PMON_CTL_EV_SEL_MASK;

	if (ev_sel >= 0xb && ev_sel <= 0xe) {
		reg1->reg = HSWEP_PCU_MSR_PMON_BOX_FILTER;
		reg1->idx = ev_sel - 0xb;
		reg1->config = event->attr.config1 & (0xff << reg1->idx);
	}
	return 0;
}

static struct intel_uncore_ops hswep_uncore_pcu_ops = {
	SNBEP_UNCORE_MSR_OPS_COMMON_INIT(),
	.hw_config		= hswep_pcu_hw_config,
	.get_constraint		= snbep_pcu_get_constraint,
	.put_constraint		= snbep_pcu_put_constraint,
};

static struct intel_uncore_type hswep_uncore_pcu = {
	.name			= "pcu",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.perf_ctr		= HSWEP_PCU_MSR_PMON_CTR0,
	.event_ctl		= HSWEP_PCU_MSR_PMON_CTL0,
	.event_mask		= SNBEP_PCU_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_PCU_MSR_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &hswep_uncore_pcu_ops,
	.format_group		= &snbep_uncore_pcu_format_group,
};

static struct intel_uncore_type *hswep_msr_uncores[] = {
	&hswep_uncore_ubox,
	&hswep_uncore_cbox,
	&hswep_uncore_sbox,
	&hswep_uncore_pcu,
	NULL,
};

void hswep_uncore_cpu_init(void)
{
	int pkg = boot_cpu_data.logical_proc_id;

	if (hswep_uncore_cbox.num_boxes > boot_cpu_data.x86_max_cores)
		hswep_uncore_cbox.num_boxes = boot_cpu_data.x86_max_cores;

	/* Detect 6-8 core systems with only two SBOXes */
	if (uncore_extra_pci_dev[pkg].dev[HSWEP_PCI_PCU_3]) {
		u32 capid4;

		pci_read_config_dword(uncore_extra_pci_dev[pkg].dev[HSWEP_PCI_PCU_3],
				      0x94, &capid4);
		if (((capid4 >> 6) & 0x3) == 0)
			hswep_uncore_sbox.num_boxes = 2;
	}

	uncore_msr_uncores = hswep_msr_uncores;
}

static struct intel_uncore_type hswep_uncore_ha = {
	.name		= "ha",
	.num_counters   = 4,
	.num_boxes	= 2,
	.perf_ctr_bits	= 48,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct uncore_event_desc hswep_uncore_imc_events[] = {
	INTEL_UNCORE_EVENT_DESC(clockticks,      "event=0x00,umask=0x00"),
	INTEL_UNCORE_EVENT_DESC(cas_count_read,  "event=0x04,umask=0x03"),
	INTEL_UNCORE_EVENT_DESC(cas_count_read.scale, "6.103515625e-5"),
	INTEL_UNCORE_EVENT_DESC(cas_count_read.unit, "MiB"),
	INTEL_UNCORE_EVENT_DESC(cas_count_write, "event=0x04,umask=0x0c"),
	INTEL_UNCORE_EVENT_DESC(cas_count_write.scale, "6.103515625e-5"),
	INTEL_UNCORE_EVENT_DESC(cas_count_write.unit, "MiB"),
	{ /* end: all zeroes */ },
};

static struct intel_uncore_type hswep_uncore_imc = {
	.name		= "imc",
	.num_counters   = 4,
	.num_boxes	= 8,
	.perf_ctr_bits	= 48,
	.fixed_ctr_bits	= 48,
	.fixed_ctr	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTR,
	.fixed_ctl	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTL,
	.event_descs	= hswep_uncore_imc_events,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static unsigned hswep_uncore_irp_ctrs[] = {0xa0, 0xa8, 0xb0, 0xb8};

static u64 hswep_uncore_irp_read_counter(struct intel_uncore_box *box, struct perf_event *event)
{
	struct pci_dev *pdev = box->pci_dev;
	struct hw_perf_event *hwc = &event->hw;
	u64 count = 0;

	pci_read_config_dword(pdev, hswep_uncore_irp_ctrs[hwc->idx], (u32 *)&count);
	pci_read_config_dword(pdev, hswep_uncore_irp_ctrs[hwc->idx] + 4, (u32 *)&count + 1);

	return count;
}

static struct intel_uncore_ops hswep_uncore_irp_ops = {
	.init_box	= snbep_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= ivbep_uncore_irp_disable_event,
	.enable_event	= ivbep_uncore_irp_enable_event,
	.read_counter	= hswep_uncore_irp_read_counter,
};

static struct intel_uncore_type hswep_uncore_irp = {
	.name			= "irp",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.event_mask		= SNBEP_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.ops			= &hswep_uncore_irp_ops,
	.format_group		= &snbep_uncore_format_group,
};

static struct intel_uncore_type hswep_uncore_qpi = {
	.name			= "qpi",
	.num_counters		= 4,
	.num_boxes		= 3,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCI_PMON_CTR0,
	.event_ctl		= SNBEP_PCI_PMON_CTL0,
	.event_mask		= SNBEP_QPI_PCI_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &snbep_uncore_qpi_ops,
	.format_group		= &snbep_uncore_qpi_format_group,
};

static struct event_constraint hswep_uncore_r2pcie_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x10, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x13, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x23, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x24, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x25, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x26, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x27, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x28, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x29, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2a, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x2b, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2c, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2d, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x32, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x33, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x34, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x35, 0x3),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type hswep_uncore_r2pcie = {
	.name		= "r2pcie",
	.num_counters   = 4,
	.num_boxes	= 1,
	.perf_ctr_bits	= 48,
	.constraints	= hswep_uncore_r2pcie_constraints,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct event_constraint hswep_uncore_r3qpi_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x01, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x07, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x08, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x09, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x0a, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x0e, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x10, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x12, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x13, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x14, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x15, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x1f, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x20, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x21, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x22, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x23, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x25, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x26, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x28, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x29, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2c, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2d, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2e, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2f, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x31, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x32, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x33, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x34, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x37, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x38, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x39, 0x3),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type hswep_uncore_r3qpi = {
	.name		= "r3qpi",
	.num_counters   = 3,
	.num_boxes	= 3,
	.perf_ctr_bits	= 44,
	.constraints	= hswep_uncore_r3qpi_constraints,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

enum {
	HSWEP_PCI_UNCORE_HA,
	HSWEP_PCI_UNCORE_IMC,
	HSWEP_PCI_UNCORE_IRP,
	HSWEP_PCI_UNCORE_QPI,
	HSWEP_PCI_UNCORE_R2PCIE,
	HSWEP_PCI_UNCORE_R3QPI,
};

static struct intel_uncore_type *hswep_pci_uncores[] = {
	[HSWEP_PCI_UNCORE_HA]	= &hswep_uncore_ha,
	[HSWEP_PCI_UNCORE_IMC]	= &hswep_uncore_imc,
	[HSWEP_PCI_UNCORE_IRP]	= &hswep_uncore_irp,
	[HSWEP_PCI_UNCORE_QPI]	= &hswep_uncore_qpi,
	[HSWEP_PCI_UNCORE_R2PCIE]	= &hswep_uncore_r2pcie,
	[HSWEP_PCI_UNCORE_R3QPI]	= &hswep_uncore_r3qpi,
	NULL,
};

static const struct pci_device_id hswep_uncore_pci_ids[] = {
	{ /* Home Agent 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f30),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_HA, 0),
	},
	{ /* Home Agent 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f38),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_HA, 1),
	},
	{ /* MC0 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fb0),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 0),
	},
	{ /* MC0 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fb1),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 1),
	},
	{ /* MC0 Channel 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fb4),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 2),
	},
	{ /* MC0 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fb5),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 3),
	},
	{ /* MC1 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fd0),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 4),
	},
	{ /* MC1 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fd1),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 5),
	},
	{ /* MC1 Channel 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fd4),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 6),
	},
	{ /* MC1 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fd5),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IMC, 7),
	},
	{ /* IRP */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f39),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_IRP, 0),
	},
	{ /* QPI0 Port 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f32),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_QPI, 0),
	},
	{ /* QPI0 Port 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f33),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_QPI, 1),
	},
	{ /* QPI1 Port 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f3a),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_QPI, 2),
	},
	{ /* R2PCIe */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f34),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_R2PCIE, 0),
	},
	{ /* R3QPI0 Link 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f36),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_R3QPI, 0),
	},
	{ /* R3QPI0 Link 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f37),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_R3QPI, 1),
	},
	{ /* R3QPI1 Link 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f3e),
		.driver_data = UNCORE_PCI_DEV_DATA(HSWEP_PCI_UNCORE_R3QPI, 2),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f86),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT0_FILTER),
	},
	{ /* QPI Port 1 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2f96),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT1_FILTER),
	},
	{ /* PCU.3 (for Capability registers) */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fc0),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   HSWEP_PCI_PCU_3),
	},
	{ /* end: all zeroes */ }
};

static struct pci_driver hswep_uncore_pci_driver = {
	.name		= "hswep_uncore",
	.id_table	= hswep_uncore_pci_ids,
};

int hswep_uncore_pci_init(void)
{
	int ret = snbep_pci2phy_map_init(0x2f1e, SNBEP_CPUNODEID, SNBEP_GIDNIDMAP, true);
	if (ret)
		return ret;
	uncore_pci_uncores = hswep_pci_uncores;
	uncore_pci_driver = &hswep_uncore_pci_driver;
	return 0;
}
/* end of Haswell-EP uncore support */

/* BDX uncore support */

static struct intel_uncore_type bdx_uncore_ubox = {
	.name			= "ubox",
	.num_counters		= 2,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= HSWEP_U_MSR_PMON_CTR0,
	.event_ctl		= HSWEP_U_MSR_PMON_CTL0,
	.event_mask		= SNBEP_U_MSR_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTR,
	.fixed_ctl		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTL,
	.num_shared_regs	= 1,
	.ops			= &ivbep_uncore_msr_ops,
	.format_group		= &ivbep_uncore_ubox_format_group,
};

static struct event_constraint bdx_uncore_cbox_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x09, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x3e, 0x1),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type bdx_uncore_cbox = {
	.name			= "cbox",
	.num_counters		= 4,
	.num_boxes		= 24,
	.perf_ctr_bits		= 48,
	.event_ctl		= HSWEP_C0_MSR_PMON_CTL0,
	.perf_ctr		= HSWEP_C0_MSR_PMON_CTR0,
	.event_mask		= SNBEP_CBO_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_C0_MSR_PMON_BOX_CTL,
	.msr_offset		= HSWEP_CBO_MSR_OFFSET,
	.num_shared_regs	= 1,
	.constraints		= bdx_uncore_cbox_constraints,
	.ops			= &hswep_uncore_cbox_ops,
	.format_group		= &hswep_uncore_cbox_format_group,
};

static struct intel_uncore_type bdx_uncore_sbox = {
	.name			= "sbox",
	.num_counters		= 4,
	.num_boxes		= 4,
	.perf_ctr_bits		= 48,
	.event_ctl		= HSWEP_S0_MSR_PMON_CTL0,
	.perf_ctr		= HSWEP_S0_MSR_PMON_CTR0,
	.event_mask		= HSWEP_S_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_S0_MSR_PMON_BOX_CTL,
	.msr_offset		= HSWEP_SBOX_MSR_OFFSET,
	.ops			= &hswep_uncore_sbox_msr_ops,
	.format_group		= &hswep_uncore_sbox_format_group,
};

#define BDX_MSR_UNCORE_SBOX	3

static struct intel_uncore_type *bdx_msr_uncores[] = {
	&bdx_uncore_ubox,
	&bdx_uncore_cbox,
	&hswep_uncore_pcu,
	&bdx_uncore_sbox,
	NULL,
};

/* Bit 7 'Use Occupancy' is not available for counter 0 on BDX */
static struct event_constraint bdx_uncore_pcu_constraints[] = {
	EVENT_CONSTRAINT(0x80, 0xe, 0x80),
	EVENT_CONSTRAINT_END
};

void bdx_uncore_cpu_init(void)
{
	int pkg = topology_phys_to_logical_pkg(boot_cpu_data.phys_proc_id);

	if (bdx_uncore_cbox.num_boxes > boot_cpu_data.x86_max_cores)
		bdx_uncore_cbox.num_boxes = boot_cpu_data.x86_max_cores;
	uncore_msr_uncores = bdx_msr_uncores;

	/* BDX-DE doesn't have SBOX */
	if (boot_cpu_data.x86_model == 86) {
		uncore_msr_uncores[BDX_MSR_UNCORE_SBOX] = NULL;
	/* Detect systems with no SBOXes */
	} else if (uncore_extra_pci_dev[pkg].dev[HSWEP_PCI_PCU_3]) {
		struct pci_dev *pdev;
		u32 capid4;

		pdev = uncore_extra_pci_dev[pkg].dev[HSWEP_PCI_PCU_3];
		pci_read_config_dword(pdev, 0x94, &capid4);
		if (((capid4 >> 6) & 0x3) == 0)
			bdx_msr_uncores[BDX_MSR_UNCORE_SBOX] = NULL;
	}
	hswep_uncore_pcu.constraints = bdx_uncore_pcu_constraints;
}

static struct intel_uncore_type bdx_uncore_ha = {
	.name		= "ha",
	.num_counters   = 4,
	.num_boxes	= 2,
	.perf_ctr_bits	= 48,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type bdx_uncore_imc = {
	.name		= "imc",
	.num_counters   = 4,
	.num_boxes	= 8,
	.perf_ctr_bits	= 48,
	.fixed_ctr_bits	= 48,
	.fixed_ctr	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTR,
	.fixed_ctl	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTL,
	.event_descs	= hswep_uncore_imc_events,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct intel_uncore_type bdx_uncore_irp = {
	.name			= "irp",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.event_mask		= SNBEP_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.ops			= &hswep_uncore_irp_ops,
	.format_group		= &snbep_uncore_format_group,
};

static struct intel_uncore_type bdx_uncore_qpi = {
	.name			= "qpi",
	.num_counters		= 4,
	.num_boxes		= 3,
	.perf_ctr_bits		= 48,
	.perf_ctr		= SNBEP_PCI_PMON_CTR0,
	.event_ctl		= SNBEP_PCI_PMON_CTL0,
	.event_mask		= SNBEP_QPI_PCI_PMON_RAW_EVENT_MASK,
	.box_ctl		= SNBEP_PCI_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &snbep_uncore_qpi_ops,
	.format_group		= &snbep_uncore_qpi_format_group,
};

static struct event_constraint bdx_uncore_r2pcie_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x10, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x13, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x23, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x25, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x26, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x28, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2c, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2d, 0x3),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type bdx_uncore_r2pcie = {
	.name		= "r2pcie",
	.num_counters   = 4,
	.num_boxes	= 1,
	.perf_ctr_bits	= 48,
	.constraints	= bdx_uncore_r2pcie_constraints,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

static struct event_constraint bdx_uncore_r3qpi_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x01, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x07, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x08, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x09, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x0a, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x0e, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x10, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x11, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x13, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x14, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x15, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x1f, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x20, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x21, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x22, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x23, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x25, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x26, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x28, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x29, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2c, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2d, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2e, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x2f, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x33, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x34, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x37, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x38, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x39, 0x3),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type bdx_uncore_r3qpi = {
	.name		= "r3qpi",
	.num_counters   = 3,
	.num_boxes	= 3,
	.perf_ctr_bits	= 48,
	.constraints	= bdx_uncore_r3qpi_constraints,
	SNBEP_UNCORE_PCI_COMMON_INIT(),
};

enum {
	BDX_PCI_UNCORE_HA,
	BDX_PCI_UNCORE_IMC,
	BDX_PCI_UNCORE_IRP,
	BDX_PCI_UNCORE_QPI,
	BDX_PCI_UNCORE_R2PCIE,
	BDX_PCI_UNCORE_R3QPI,
};

static struct intel_uncore_type *bdx_pci_uncores[] = {
	[BDX_PCI_UNCORE_HA]	= &bdx_uncore_ha,
	[BDX_PCI_UNCORE_IMC]	= &bdx_uncore_imc,
	[BDX_PCI_UNCORE_IRP]	= &bdx_uncore_irp,
	[BDX_PCI_UNCORE_QPI]	= &bdx_uncore_qpi,
	[BDX_PCI_UNCORE_R2PCIE]	= &bdx_uncore_r2pcie,
	[BDX_PCI_UNCORE_R3QPI]	= &bdx_uncore_r3qpi,
	NULL,
};

static const struct pci_device_id bdx_uncore_pci_ids[] = {
	{ /* Home Agent 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f30),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_HA, 0),
	},
	{ /* Home Agent 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f38),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_HA, 1),
	},
	{ /* MC0 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fb0),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 0),
	},
	{ /* MC0 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fb1),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 1),
	},
	{ /* MC0 Channel 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fb4),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 2),
	},
	{ /* MC0 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fb5),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 3),
	},
	{ /* MC1 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fd0),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 4),
	},
	{ /* MC1 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fd1),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 5),
	},
	{ /* MC1 Channel 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fd4),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 6),
	},
	{ /* MC1 Channel 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fd5),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IMC, 7),
	},
	{ /* IRP */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f39),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_IRP, 0),
	},
	{ /* QPI0 Port 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f32),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_QPI, 0),
	},
	{ /* QPI0 Port 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f33),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_QPI, 1),
	},
	{ /* QPI1 Port 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f3a),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_QPI, 2),
	},
	{ /* R2PCIe */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f34),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_R2PCIE, 0),
	},
	{ /* R3QPI0 Link 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f36),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_R3QPI, 0),
	},
	{ /* R3QPI0 Link 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f37),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_R3QPI, 1),
	},
	{ /* R3QPI1 Link 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f3e),
		.driver_data = UNCORE_PCI_DEV_DATA(BDX_PCI_UNCORE_R3QPI, 2),
	},
	{ /* QPI Port 0 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f86),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT0_FILTER),
	},
	{ /* QPI Port 1 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f96),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT1_FILTER),
	},
	{ /* QPI Port 2 filter  */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6f46),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   BDX_PCI_QPI_PORT2_FILTER),
	},
	{ /* PCU.3 (for Capability registers) */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x6fc0),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   HSWEP_PCI_PCU_3),
	},
	{ /* end: all zeroes */ }
};

static struct pci_driver bdx_uncore_pci_driver = {
	.name		= "bdx_uncore",
	.id_table	= bdx_uncore_pci_ids,
};

int bdx_uncore_pci_init(void)
{
	int ret = snbep_pci2phy_map_init(0x6f1e, SNBEP_CPUNODEID, SNBEP_GIDNIDMAP, true);

	if (ret)
		return ret;
	uncore_pci_uncores = bdx_pci_uncores;
	uncore_pci_driver = &bdx_uncore_pci_driver;
	return 0;
}

/* end of BDX uncore support */

/* SKX uncore support */

static struct intel_uncore_type skx_uncore_ubox = {
	.name			= "ubox",
	.num_counters		= 2,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.fixed_ctr_bits		= 48,
	.perf_ctr		= HSWEP_U_MSR_PMON_CTR0,
	.event_ctl		= HSWEP_U_MSR_PMON_CTL0,
	.event_mask		= SNBEP_U_MSR_PMON_RAW_EVENT_MASK,
	.fixed_ctr		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTR,
	.fixed_ctl		= HSWEP_U_MSR_PMON_UCLK_FIXED_CTL,
	.ops			= &ivbep_uncore_msr_ops,
	.format_group		= &ivbep_uncore_ubox_format_group,
};

static struct attribute *skx_uncore_cha_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_tid_en.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	&format_attr_filter_tid4.attr,
	&format_attr_filter_state5.attr,
	&format_attr_filter_rem.attr,
	&format_attr_filter_loc.attr,
	&format_attr_filter_nm.attr,
	&format_attr_filter_all_op.attr,
	&format_attr_filter_not_nm.attr,
	&format_attr_filter_opc_0.attr,
	&format_attr_filter_opc_1.attr,
	&format_attr_filter_nc.attr,
	&format_attr_filter_isoc.attr,
	NULL,
};

static const struct attribute_group skx_uncore_chabox_format_group = {
	.name = "format",
	.attrs = skx_uncore_cha_formats_attr,
};

static struct event_constraint skx_uncore_chabox_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x11, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x36, 0x1),
	EVENT_CONSTRAINT_END
};

static struct extra_reg skx_uncore_cha_extra_regs[] = {
	SNBEP_CBO_EVENT_EXTRA_REG(0x0334, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0534, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x0934, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x1134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x3134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x9134, 0xffff, 0x4),
	SNBEP_CBO_EVENT_EXTRA_REG(0x35, 0xff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x36, 0xff, 0x8),
	SNBEP_CBO_EVENT_EXTRA_REG(0x38, 0xff, 0x3),
	EVENT_EXTRA_END
};

static u64 skx_cha_filter_mask(int fields)
{
	u64 mask = 0;

	if (fields & 0x1)
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_TID;
	if (fields & 0x2)
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_LINK;
	if (fields & 0x4)
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_STATE;
	if (fields & 0x8) {
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_REM;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_LOC;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_ALL_OPC;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_NM;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_NOT_NM;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_OPC0;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_OPC1;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_NC;
		mask |= SKX_CHA_MSR_PMON_BOX_FILTER_ISOC;
	}
	return mask;
}

static struct event_constraint *
skx_cha_get_constraint(struct intel_uncore_box *box, struct perf_event *event)
{
	return __snbep_cbox_get_constraint(box, event, skx_cha_filter_mask);
}

static int skx_cha_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
	struct hw_perf_event_extra *reg1 = &event->hw.extra_reg;
	struct extra_reg *er;
	int idx = 0;

	for (er = skx_uncore_cha_extra_regs; er->msr; er++) {
		if (er->event != (event->hw.config & er->config_mask))
			continue;
		idx |= er->idx;
	}

	if (idx) {
		reg1->reg = HSWEP_C0_MSR_PMON_BOX_FILTER0 +
			    HSWEP_CBO_MSR_OFFSET * box->pmu->pmu_idx;
		reg1->config = event->attr.config1 & skx_cha_filter_mask(idx);
		reg1->idx = idx;
	}
	return 0;
}

static struct intel_uncore_ops skx_uncore_chabox_ops = {
	/* There is no frz_en for chabox ctl */
	.init_box		= ivbep_uncore_msr_init_box,
	.disable_box		= snbep_uncore_msr_disable_box,
	.enable_box		= snbep_uncore_msr_enable_box,
	.disable_event		= snbep_uncore_msr_disable_event,
	.enable_event		= hswep_cbox_enable_event,
	.read_counter		= uncore_msr_read_counter,
	.hw_config		= skx_cha_hw_config,
	.get_constraint		= skx_cha_get_constraint,
	.put_constraint		= snbep_cbox_put_constraint,
};

static struct intel_uncore_type skx_uncore_chabox = {
	.name			= "cha",
	.num_counters		= 4,
	.perf_ctr_bits		= 48,
	.event_ctl		= HSWEP_C0_MSR_PMON_CTL0,
	.perf_ctr		= HSWEP_C0_MSR_PMON_CTR0,
	.event_mask		= HSWEP_S_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_C0_MSR_PMON_BOX_CTL,
	.msr_offset		= HSWEP_CBO_MSR_OFFSET,
	.num_shared_regs	= 1,
	.constraints		= skx_uncore_chabox_constraints,
	.ops			= &skx_uncore_chabox_ops,
	.format_group		= &skx_uncore_chabox_format_group,
};

static struct attribute *skx_uncore_iio_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh9.attr,
	&format_attr_ch_mask.attr,
	&format_attr_fc_mask.attr,
	NULL,
};

static const struct attribute_group skx_uncore_iio_format_group = {
	.name = "format",
	.attrs = skx_uncore_iio_formats_attr,
};

static struct event_constraint skx_uncore_iio_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x83, 0x3),
	UNCORE_EVENT_CONSTRAINT(0x88, 0xc),
	UNCORE_EVENT_CONSTRAINT(0x95, 0xc),
	UNCORE_EVENT_CONSTRAINT(0xc0, 0xc),
	UNCORE_EVENT_CONSTRAINT(0xc5, 0xc),
	UNCORE_EVENT_CONSTRAINT(0xd4, 0xc),
	EVENT_CONSTRAINT_END
};

static void skx_iio_enable_event(struct intel_uncore_box *box,
				 struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;

	wrmsrl(hwc->config_base, hwc->config | SNBEP_PMON_CTL_EN);
}

static struct intel_uncore_ops skx_uncore_iio_ops = {
	.init_box		= ivbep_uncore_msr_init_box,
	.disable_box		= snbep_uncore_msr_disable_box,
	.enable_box		= snbep_uncore_msr_enable_box,
	.disable_event		= snbep_uncore_msr_disable_event,
	.enable_event		= skx_iio_enable_event,
	.read_counter		= uncore_msr_read_counter,
};

static struct intel_uncore_type skx_uncore_iio = {
	.name			= "iio",
	.num_counters		= 4,
	.num_boxes		= 6,
	.perf_ctr_bits		= 48,
	.event_ctl		= SKX_IIO0_MSR_PMON_CTL0,
	.perf_ctr		= SKX_IIO0_MSR_PMON_CTR0,
	.event_mask		= SKX_IIO_PMON_RAW_EVENT_MASK,
	.event_mask_ext		= SKX_IIO_PMON_RAW_EVENT_MASK_EXT,
	.box_ctl		= SKX_IIO0_MSR_PMON_BOX_CTL,
	.msr_offset		= SKX_IIO_MSR_OFFSET,
	.constraints		= skx_uncore_iio_constraints,
	.ops			= &skx_uncore_iio_ops,
	.format_group		= &skx_uncore_iio_format_group,
};

enum perf_uncore_iio_freerunning_type_id {
	SKX_IIO_MSR_IOCLK			= 0,
	SKX_IIO_MSR_BW				= 1,
	SKX_IIO_MSR_UTIL			= 2,

	SKX_IIO_FREERUNNING_TYPE_MAX,
};


static struct freerunning_counters skx_iio_freerunning[] = {
	[SKX_IIO_MSR_IOCLK]	= { 0xa45, 0x1, 0x20, 1, 36 },
	[SKX_IIO_MSR_BW]	= { 0xb00, 0x1, 0x10, 8, 36 },
	[SKX_IIO_MSR_UTIL]	= { 0xb08, 0x1, 0x10, 8, 36 },
};

static struct uncore_event_desc skx_uncore_iio_freerunning_events[] = {
	/* Free-Running IO CLOCKS Counter */
	INTEL_UNCORE_EVENT_DESC(ioclk,			"event=0xff,umask=0x10"),
	/* Free-Running IIO BANDWIDTH Counters */
	INTEL_UNCORE_EVENT_DESC(bw_in_port0,		"event=0xff,umask=0x20"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port0.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port0.unit,	"MiB"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port1,		"event=0xff,umask=0x21"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port1.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port1.unit,	"MiB"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port2,		"event=0xff,umask=0x22"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port2.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port2.unit,	"MiB"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port3,		"event=0xff,umask=0x23"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port3.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_in_port3.unit,	"MiB"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port0,		"event=0xff,umask=0x24"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port0.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port0.unit,	"MiB"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port1,		"event=0xff,umask=0x25"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port1.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port1.unit,	"MiB"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port2,		"event=0xff,umask=0x26"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port2.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port2.unit,	"MiB"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port3,		"event=0xff,umask=0x27"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port3.scale,	"3.814697266e-6"),
	INTEL_UNCORE_EVENT_DESC(bw_out_port3.unit,	"MiB"),
	/* Free-running IIO UTILIZATION Counters */
	INTEL_UNCORE_EVENT_DESC(util_in_port0,		"event=0xff,umask=0x30"),
	INTEL_UNCORE_EVENT_DESC(util_out_port0,		"event=0xff,umask=0x31"),
	INTEL_UNCORE_EVENT_DESC(util_in_port1,		"event=0xff,umask=0x32"),
	INTEL_UNCORE_EVENT_DESC(util_out_port1,		"event=0xff,umask=0x33"),
	INTEL_UNCORE_EVENT_DESC(util_in_port2,		"event=0xff,umask=0x34"),
	INTEL_UNCORE_EVENT_DESC(util_out_port2,		"event=0xff,umask=0x35"),
	INTEL_UNCORE_EVENT_DESC(util_in_port3,		"event=0xff,umask=0x36"),
	INTEL_UNCORE_EVENT_DESC(util_out_port3,		"event=0xff,umask=0x37"),
	{ /* end: all zeroes */ },
};

static struct intel_uncore_ops skx_uncore_iio_freerunning_ops = {
	.read_counter		= uncore_msr_read_counter,
};

static struct attribute *skx_uncore_iio_freerunning_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	NULL,
};

static const struct attribute_group skx_uncore_iio_freerunning_format_group = {
	.name = "format",
	.attrs = skx_uncore_iio_freerunning_formats_attr,
};

static struct intel_uncore_type skx_uncore_iio_free_running = {
	.name			= "iio_free_running",
	.num_counters		= 17,
	.num_boxes		= 6,
	.num_freerunning_types	= SKX_IIO_FREERUNNING_TYPE_MAX,
	.freerunning		= skx_iio_freerunning,
	.ops			= &skx_uncore_iio_freerunning_ops,
	.event_descs		= skx_uncore_iio_freerunning_events,
	.format_group		= &skx_uncore_iio_freerunning_format_group,
};

static struct attribute *skx_uncore_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	NULL,
};

static const struct attribute_group skx_uncore_format_group = {
	.name = "format",
	.attrs = skx_uncore_formats_attr,
};

static struct intel_uncore_type skx_uncore_irp = {
	.name			= "irp",
	.num_counters		= 2,
	.num_boxes		= 6,
	.perf_ctr_bits		= 48,
	.event_ctl		= SKX_IRP0_MSR_PMON_CTL0,
	.perf_ctr		= SKX_IRP0_MSR_PMON_CTR0,
	.event_mask		= SNBEP_PMON_RAW_EVENT_MASK,
	.box_ctl		= SKX_IRP0_MSR_PMON_BOX_CTL,
	.msr_offset		= SKX_IRP_MSR_OFFSET,
	.ops			= &skx_uncore_iio_ops,
	.format_group		= &skx_uncore_format_group,
};

static struct attribute *skx_uncore_pcu_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	&format_attr_occ_invert.attr,
	&format_attr_occ_edge_det.attr,
	&format_attr_filter_band0.attr,
	&format_attr_filter_band1.attr,
	&format_attr_filter_band2.attr,
	&format_attr_filter_band3.attr,
	NULL,
};

static struct attribute_group skx_uncore_pcu_format_group = {
	.name = "format",
	.attrs = skx_uncore_pcu_formats_attr,
};

static struct intel_uncore_ops skx_uncore_pcu_ops = {
	IVBEP_UNCORE_MSR_OPS_COMMON_INIT(),
	.hw_config		= hswep_pcu_hw_config,
	.get_constraint		= snbep_pcu_get_constraint,
	.put_constraint		= snbep_pcu_put_constraint,
};

static struct intel_uncore_type skx_uncore_pcu = {
	.name			= "pcu",
	.num_counters		= 4,
	.num_boxes		= 1,
	.perf_ctr_bits		= 48,
	.perf_ctr		= HSWEP_PCU_MSR_PMON_CTR0,
	.event_ctl		= HSWEP_PCU_MSR_PMON_CTL0,
	.event_mask		= SNBEP_PCU_MSR_PMON_RAW_EVENT_MASK,
	.box_ctl		= HSWEP_PCU_MSR_PMON_BOX_CTL,
	.num_shared_regs	= 1,
	.ops			= &skx_uncore_pcu_ops,
	.format_group		= &skx_uncore_pcu_format_group,
};

static struct intel_uncore_type *skx_msr_uncores[] = {
	&skx_uncore_ubox,
	&skx_uncore_chabox,
	&skx_uncore_iio,
	&skx_uncore_iio_free_running,
	&skx_uncore_irp,
	&skx_uncore_pcu,
	NULL,
};

/*
 * To determine the number of CHAs, it should read bits 27:0 in the CAPID6
 * register which located at Device 30, Function 3, Offset 0x9C. PCI ID 0x2083.
 */
#define SKX_CAPID6		0x9c
#define SKX_CHA_BIT_MASK	GENMASK(27, 0)

static int skx_count_chabox(void)
{
	struct pci_dev *dev = NULL;
	u32 val = 0;

	dev = pci_get_device(PCI_VENDOR_ID_INTEL, 0x2083, dev);
	if (!dev)
		goto out;

	pci_read_config_dword(dev, SKX_CAPID6, &val);
	val &= SKX_CHA_BIT_MASK;
out:
	pci_dev_put(dev);
	return hweight32(val);
}

void skx_uncore_cpu_init(void)
{
	skx_uncore_chabox.num_boxes = skx_count_chabox();
	uncore_msr_uncores = skx_msr_uncores;
}

static struct intel_uncore_type skx_uncore_imc = {
	.name		= "imc",
	.num_counters   = 4,
	.num_boxes	= 6,
	.perf_ctr_bits	= 48,
	.fixed_ctr_bits	= 48,
	.fixed_ctr	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTR,
	.fixed_ctl	= SNBEP_MC_CHy_PCI_PMON_FIXED_CTL,
	.event_descs	= hswep_uncore_imc_events,
	.perf_ctr	= SNBEP_PCI_PMON_CTR0,
	.event_ctl	= SNBEP_PCI_PMON_CTL0,
	.event_mask	= SNBEP_PMON_RAW_EVENT_MASK,
	.box_ctl	= SNBEP_PCI_PMON_BOX_CTL,
	.ops		= &ivbep_uncore_pci_ops,
	.format_group	= &skx_uncore_format_group,
};

static struct attribute *skx_upi_uncore_formats_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask_ext.attr,
	&format_attr_edge.attr,
	&format_attr_inv.attr,
	&format_attr_thresh8.attr,
	NULL,
};

static const struct attribute_group skx_upi_uncore_format_group = {
	.name = "format",
	.attrs = skx_upi_uncore_formats_attr,
};

static void skx_upi_uncore_pci_init_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;

	__set_bit(UNCORE_BOX_FLAG_CTL_OFFS8, &box->flags);
	pci_write_config_dword(pdev, SKX_UPI_PCI_PMON_BOX_CTL, IVBEP_PMON_BOX_CTL_INT);
}

static struct intel_uncore_ops skx_upi_uncore_pci_ops = {
	.init_box	= skx_upi_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= snbep_uncore_pci_disable_event,
	.enable_event	= snbep_uncore_pci_enable_event,
	.read_counter	= snbep_uncore_pci_read_counter,
};

static struct intel_uncore_type skx_uncore_upi = {
	.name		= "upi",
	.num_counters   = 4,
	.num_boxes	= 3,
	.perf_ctr_bits	= 48,
	.perf_ctr	= SKX_UPI_PCI_PMON_CTR0,
	.event_ctl	= SKX_UPI_PCI_PMON_CTL0,
	.event_mask	= SNBEP_PMON_RAW_EVENT_MASK,
	.event_mask_ext = SKX_UPI_CTL_UMASK_EXT,
	.box_ctl	= SKX_UPI_PCI_PMON_BOX_CTL,
	.ops		= &skx_upi_uncore_pci_ops,
	.format_group	= &skx_upi_uncore_format_group,
};

static void skx_m2m_uncore_pci_init_box(struct intel_uncore_box *box)
{
	struct pci_dev *pdev = box->pci_dev;

	__set_bit(UNCORE_BOX_FLAG_CTL_OFFS8, &box->flags);
	pci_write_config_dword(pdev, SKX_M2M_PCI_PMON_BOX_CTL, IVBEP_PMON_BOX_CTL_INT);
}

static struct intel_uncore_ops skx_m2m_uncore_pci_ops = {
	.init_box	= skx_m2m_uncore_pci_init_box,
	.disable_box	= snbep_uncore_pci_disable_box,
	.enable_box	= snbep_uncore_pci_enable_box,
	.disable_event	= snbep_uncore_pci_disable_event,
	.enable_event	= snbep_uncore_pci_enable_event,
	.read_counter	= snbep_uncore_pci_read_counter,
};

static struct intel_uncore_type skx_uncore_m2m = {
	.name		= "m2m",
	.num_counters   = 4,
	.num_boxes	= 2,
	.perf_ctr_bits	= 48,
	.perf_ctr	= SKX_M2M_PCI_PMON_CTR0,
	.event_ctl	= SKX_M2M_PCI_PMON_CTL0,
	.event_mask	= SNBEP_PMON_RAW_EVENT_MASK,
	.box_ctl	= SKX_M2M_PCI_PMON_BOX_CTL,
	.ops		= &skx_m2m_uncore_pci_ops,
	.format_group	= &skx_uncore_format_group,
};

static struct event_constraint skx_uncore_m2pcie_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x23, 0x3),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type skx_uncore_m2pcie = {
	.name		= "m2pcie",
	.num_counters   = 4,
	.num_boxes	= 4,
	.perf_ctr_bits	= 48,
	.constraints	= skx_uncore_m2pcie_constraints,
	.perf_ctr	= SNBEP_PCI_PMON_CTR0,
	.event_ctl	= SNBEP_PCI_PMON_CTL0,
	.event_mask	= SNBEP_PMON_RAW_EVENT_MASK,
	.box_ctl	= SNBEP_PCI_PMON_BOX_CTL,
	.ops		= &ivbep_uncore_pci_ops,
	.format_group	= &skx_uncore_format_group,
};

static struct event_constraint skx_uncore_m3upi_constraints[] = {
	UNCORE_EVENT_CONSTRAINT(0x1d, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x1e, 0x1),
	UNCORE_EVENT_CONSTRAINT(0x40, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x4e, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x4f, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x50, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x51, 0x7),
	UNCORE_EVENT_CONSTRAINT(0x52, 0x7),
	EVENT_CONSTRAINT_END
};

static struct intel_uncore_type skx_uncore_m3upi = {
	.name		= "m3upi",
	.num_counters   = 3,
	.num_boxes	= 3,
	.perf_ctr_bits	= 48,
	.constraints	= skx_uncore_m3upi_constraints,
	.perf_ctr	= SNBEP_PCI_PMON_CTR0,
	.event_ctl	= SNBEP_PCI_PMON_CTL0,
	.event_mask	= SNBEP_PMON_RAW_EVENT_MASK,
	.box_ctl	= SNBEP_PCI_PMON_BOX_CTL,
	.ops		= &ivbep_uncore_pci_ops,
	.format_group	= &skx_uncore_format_group,
};

enum {
	SKX_PCI_UNCORE_IMC,
	SKX_PCI_UNCORE_M2M,
	SKX_PCI_UNCORE_UPI,
	SKX_PCI_UNCORE_M2PCIE,
	SKX_PCI_UNCORE_M3UPI,
};

static struct intel_uncore_type *skx_pci_uncores[] = {
	[SKX_PCI_UNCORE_IMC]	= &skx_uncore_imc,
	[SKX_PCI_UNCORE_M2M]	= &skx_uncore_m2m,
	[SKX_PCI_UNCORE_UPI]	= &skx_uncore_upi,
	[SKX_PCI_UNCORE_M2PCIE]	= &skx_uncore_m2pcie,
	[SKX_PCI_UNCORE_M3UPI]	= &skx_uncore_m3upi,
	NULL,
};

static const struct pci_device_id skx_uncore_pci_ids[] = {
	{ /* MC0 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2042),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(10, 2, SKX_PCI_UNCORE_IMC, 0),
	},
	{ /* MC0 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2046),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(10, 6, SKX_PCI_UNCORE_IMC, 1),
	},
	{ /* MC0 Channel 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x204a),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(11, 2, SKX_PCI_UNCORE_IMC, 2),
	},
	{ /* MC1 Channel 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2042),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(12, 2, SKX_PCI_UNCORE_IMC, 3),
	},
	{ /* MC1 Channel 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2046),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(12, 6, SKX_PCI_UNCORE_IMC, 4),
	},
	{ /* MC1 Channel 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x204a),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(13, 2, SKX_PCI_UNCORE_IMC, 5),
	},
	{ /* M2M0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2066),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(8, 0, SKX_PCI_UNCORE_M2M, 0),
	},
	{ /* M2M1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2066),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(9, 0, SKX_PCI_UNCORE_M2M, 1),
	},
	{ /* UPI0 Link 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2058),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(14, 0, SKX_PCI_UNCORE_UPI, 0),
	},
	{ /* UPI0 Link 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2058),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(15, 0, SKX_PCI_UNCORE_UPI, 1),
	},
	{ /* UPI1 Link 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2058),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(16, 0, SKX_PCI_UNCORE_UPI, 2),
	},
	{ /* M2PCIe 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2088),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(21, 1, SKX_PCI_UNCORE_M2PCIE, 0),
	},
	{ /* M2PCIe 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2088),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(22, 1, SKX_PCI_UNCORE_M2PCIE, 1),
	},
	{ /* M2PCIe 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2088),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(23, 1, SKX_PCI_UNCORE_M2PCIE, 2),
	},
	{ /* M2PCIe 3 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2088),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(21, 5, SKX_PCI_UNCORE_M2PCIE, 3),
	},
	{ /* M3UPI0 Link 0 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x204D),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(18, 1, SKX_PCI_UNCORE_M3UPI, 0),
	},
	{ /* M3UPI0 Link 1 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x204E),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(18, 2, SKX_PCI_UNCORE_M3UPI, 1),
	},
	{ /* M3UPI1 Link 2 */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x204D),
		.driver_data = UNCORE_PCI_DEV_FULL_DATA(18, 5, SKX_PCI_UNCORE_M3UPI, 2),
	},
	{ /* end: all zeroes */ }
};


static struct pci_driver skx_uncore_pci_driver = {
	.name		= "skx_uncore",
	.id_table	= skx_uncore_pci_ids,
};

int skx_uncore_pci_init(void)
{
	/* need to double check pci address */
	int ret = snbep_pci2phy_map_init(0x2014, SKX_CPUNODEID, SKX_GIDNIDMAP, false);

	if (ret)
		return ret;

	uncore_pci_uncores = skx_pci_uncores;
	uncore_pci_driver = &skx_uncore_pci_driver;
	return 0;
}

/* end of SKX uncore support */
	while (1) {
		/* find the UBOX device */
		ubox_dev = pci_get_device(PCI_VENDOR_ID_INTEL, devid, ubox_dev);
		if (!ubox_dev)
			break;
		bus = ubox_dev->bus->number;
		/* get the Node ID of the local register */
		err = pci_read_config_dword(ubox_dev, nodeid_loc, &config);
		if (err)
			break;
		nodeid = config;
		/* get the Node ID mapping */
		err = pci_read_config_dword(ubox_dev, idmap_loc, &config);
		if (err)
			break;

		segment = pci_domain_nr(ubox_dev->bus);
		raw_spin_lock(&pci2phy_map_lock);
		map = __find_pci2phy_map(segment);
		if (!map) {
			raw_spin_unlock(&pci2phy_map_lock);
			err = -ENOMEM;
			break;
		}