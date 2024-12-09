{
	*(__le32 *)(&p->virt[off]) = cpu_to_le32(virt);
	*(__le32 *)(&p->write[off]) = cpu_to_le32(write);
}

/*
 * Restore the *real* BARs after we detect a FLR or backdoor reset.
 * (backdoor = some device specific technique that we didn't catch)
 */
static void vfio_bar_restore(struct vfio_pci_device *vdev)
{
	struct pci_dev *pdev = vdev->pdev;
	u32 *rbar = vdev->rbar;
	u16 cmd;
	int i;

	if (pdev->is_virtfn)
		return;

	pci_info(pdev, "%s: reset recovery - restoring BARs\n", __func__);

	for (i = PCI_BASE_ADDRESS_0; i <= PCI_BASE_ADDRESS_5; i += 4, rbar++)
		pci_user_write_config_dword(pdev, i, *rbar);

	pci_user_write_config_dword(pdev, PCI_ROM_ADDRESS, *rbar);

	if (vdev->nointx) {
		pci_user_read_config_word(pdev, PCI_COMMAND, &cmd);
		cmd |= PCI_COMMAND_INTX_DISABLE;
		pci_user_write_config_word(pdev, PCI_COMMAND, cmd);
	}
	if (offset == PCI_COMMAND) {
		bool phys_mem, virt_mem, new_mem, phys_io, virt_io, new_io;
		u16 phys_cmd;

		ret = pci_user_read_config_word(pdev, PCI_COMMAND, &phys_cmd);
		if (ret)
			return ret;

		new_cmd = le32_to_cpu(val);

		phys_mem = !!(phys_cmd & PCI_COMMAND_MEMORY);
		virt_mem = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_MEMORY);
		new_mem = !!(new_cmd & PCI_COMMAND_MEMORY);

		phys_io = !!(phys_cmd & PCI_COMMAND_IO);
		virt_io = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_IO);
		new_io = !!(new_cmd & PCI_COMMAND_IO);

		/*
		 * If the user is writing mem/io enable (new_mem/io) and we
		 * think it's already enabled (virt_mem/io), but the hardware
		 * shows it disabled (phys_mem/io, then the device has
		 * undergone some kind of backdoor reset and needs to be
		 * restored before we allow it to enable the bars.
		 * SR-IOV devices will trigger this, but we catch them later
		 */
		if ((new_mem && virt_mem && !phys_mem) ||
		    (new_io && virt_io && !phys_io) ||
		    vfio_need_bar_restore(vdev))
			vfio_bar_restore(vdev);
	}

	count = vfio_default_config_write(vdev, pos, count, perm, offset, val);
	if (count < 0)
		return count;

	/*
	 * Save current memory/io enable bits in vconfig to allow for
	 * the test above next time.
	 */
	if (offset == PCI_COMMAND) {
		u16 mask = PCI_COMMAND_MEMORY | PCI_COMMAND_IO;

		*virt_cmd &= cpu_to_le16(~mask);
		*virt_cmd |= cpu_to_le16(new_cmd & mask);
	}

	/* Emulate INTx disable */
	if (offset >= PCI_COMMAND && offset <= PCI_COMMAND + 1) {
		bool virt_intx_disable;

		virt_intx_disable = !!(le16_to_cpu(*virt_cmd) &
				       PCI_COMMAND_INTX_DISABLE);

		if (virt_intx_disable && !vdev->virq_disabled) {
			vdev->virq_disabled = true;
			vfio_pci_intx_mask(vdev);
		} else if (!virt_intx_disable && vdev->virq_disabled) {
			vdev->virq_disabled = false;
			vfio_pci_intx_unmask(vdev);
		}
	}
	if (offset == PCI_COMMAND) {
		bool phys_mem, virt_mem, new_mem, phys_io, virt_io, new_io;
		u16 phys_cmd;

		ret = pci_user_read_config_word(pdev, PCI_COMMAND, &phys_cmd);
		if (ret)
			return ret;

		new_cmd = le32_to_cpu(val);

		phys_mem = !!(phys_cmd & PCI_COMMAND_MEMORY);
		virt_mem = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_MEMORY);
		new_mem = !!(new_cmd & PCI_COMMAND_MEMORY);

		phys_io = !!(phys_cmd & PCI_COMMAND_IO);
		virt_io = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_IO);
		new_io = !!(new_cmd & PCI_COMMAND_IO);

		/*
		 * If the user is writing mem/io enable (new_mem/io) and we
		 * think it's already enabled (virt_mem/io), but the hardware
		 * shows it disabled (phys_mem/io, then the device has
		 * undergone some kind of backdoor reset and needs to be
		 * restored before we allow it to enable the bars.
		 * SR-IOV devices will trigger this, but we catch them later
		 */
		if ((new_mem && virt_mem && !phys_mem) ||
		    (new_io && virt_io && !phys_io) ||
		    vfio_need_bar_restore(vdev))
			vfio_bar_restore(vdev);
	}

	count = vfio_default_config_write(vdev, pos, count, perm, offset, val);
	if (count < 0)
		return count;

	/*
	 * Save current memory/io enable bits in vconfig to allow for
	 * the test above next time.
	 */
	if (offset == PCI_COMMAND) {
		u16 mask = PCI_COMMAND_MEMORY | PCI_COMMAND_IO;

		*virt_cmd &= cpu_to_le16(~mask);
		*virt_cmd |= cpu_to_le16(new_cmd & mask);
	}
	if (offset == PCI_COMMAND) {
		u16 mask = PCI_COMMAND_MEMORY | PCI_COMMAND_IO;

		*virt_cmd &= cpu_to_le16(~mask);
		*virt_cmd |= cpu_to_le16(new_cmd & mask);
	}

	/* Emulate INTx disable */
	if (offset >= PCI_COMMAND && offset <= PCI_COMMAND + 1) {
		bool virt_intx_disable;

		virt_intx_disable = !!(le16_to_cpu(*virt_cmd) &
				       PCI_COMMAND_INTX_DISABLE);

		if (virt_intx_disable && !vdev->virq_disabled) {
			vdev->virq_disabled = true;
			vfio_pci_intx_mask(vdev);
		} else if (!virt_intx_disable && vdev->virq_disabled) {
			vdev->virq_disabled = false;
			vfio_pci_intx_unmask(vdev);
		}
	}
	if (*ctrl & cpu_to_le16(PCI_EXP_DEVCTL_BCR_FLR)) {
		u32 cap;
		int ret;

		*ctrl &= ~cpu_to_le16(PCI_EXP_DEVCTL_BCR_FLR);

		ret = pci_user_read_config_dword(vdev->pdev,
						 pos - offset + PCI_EXP_DEVCAP,
						 &cap);

		if (!ret && (cap & PCI_EXP_DEVCAP_FLR))
			pci_try_reset_function(vdev->pdev);
	}

	/*
	 * MPS is virtualized to the user, writes do not change the physical
	 * register since determining a proper MPS value requires a system wide
	 * device view.  The MRRS is largely independent of MPS, but since the
	 * user does not have that system-wide view, they might set a safe, but
	 * inefficiently low value.  Here we allow writes through to hardware,
	 * but we set the floor to the physical device MPS setting, so that
	 * we can at least use full TLPs, as defined by the MPS value.
	 *
	 * NB, if any devices actually depend on an artificially low MRRS
	 * setting, this will need to be revisited, perhaps with a quirk
	 * though pcie_set_readrq().
	 */
	if (readrq != (le16_to_cpu(*ctrl) & PCI_EXP_DEVCTL_READRQ)) {
		readrq = 128 <<
			((le16_to_cpu(*ctrl) & PCI_EXP_DEVCTL_READRQ) >> 12);
		readrq = max(readrq, pcie_get_mps(vdev->pdev));

		pcie_set_readrq(vdev->pdev, readrq);
	}

	return count;
}

/* Permissions for PCI Express capability */
static int __init init_pci_cap_exp_perm(struct perm_bits *perm)
{
	/* Alloc largest of possible sizes */
	if (alloc_perm_bits(perm, PCI_CAP_EXP_ENDPOINT_SIZEOF_V2))
		return -ENOMEM;

	perm->writefn = vfio_exp_config_write;

	p_setb(perm, PCI_CAP_LIST_NEXT, (u8)ALL_VIRT, NO_WRITE);

	/*
	 * Allow writes to device control fields, except devctl_phantom,
	 * which could confuse IOMMU, MPS, which can break communication
	 * with other physical devices, and the ARI bit in devctl2, which
	 * is set at probe time.  FLR and MRRS get virtualized via our
	 * writefn.
	 */
	p_setw(perm, PCI_EXP_DEVCTL,
	       PCI_EXP_DEVCTL_BCR_FLR | PCI_EXP_DEVCTL_PAYLOAD |
	       PCI_EXP_DEVCTL_READRQ, ~PCI_EXP_DEVCTL_PHANTOM);
	p_setw(perm, PCI_EXP_DEVCTL2, NO_VIRT, ~PCI_EXP_DEVCTL2_ARI);
	return 0;
}

static int vfio_af_config_write(struct vfio_pci_device *vdev, int pos,
				int count, struct perm_bits *perm,
				int offset, __le32 val)
{
	u8 *ctrl = vdev->vconfig + pos - offset + PCI_AF_CTRL;

	count = vfio_default_config_write(vdev, pos, count, perm, offset, val);
	if (count < 0)
		return count;

	/*
	 * The FLR bit is virtualized, if set and the device supports AF
	 * FLR, issue a reset_function.  Regardless, clear the bit, the spec
	 * requires it to be always read as zero.  NB, reset_function might
	 * not use an AF FLR, we don't have that level of granularity.
	 */
	if (*ctrl & PCI_AF_CTRL_FLR) {
		u8 cap;
		int ret;

		*ctrl &= ~PCI_AF_CTRL_FLR;

		ret = pci_user_read_config_byte(vdev->pdev,
						pos - offset + PCI_AF_CAP,
						&cap);

		if (!ret && (cap & PCI_AF_CAP_FLR) && (cap & PCI_AF_CAP_TP))
			pci_try_reset_function(vdev->pdev);
	}

	return count;
}

/* Permissions for Advanced Function capability */
static int __init init_pci_cap_af_perm(struct perm_bits *perm)
{
	if (alloc_perm_bits(perm, pci_cap_length[PCI_CAP_ID_AF]))
		return -ENOMEM;

	perm->writefn = vfio_af_config_write;

	p_setb(perm, PCI_CAP_LIST_NEXT, (u8)ALL_VIRT, NO_WRITE);
	p_setb(perm, PCI_AF_CTRL, PCI_AF_CTRL_FLR, PCI_AF_CTRL_FLR);
	return 0;
}

/* Permissions for Advanced Error Reporting extended capability */
static int __init init_pci_ext_cap_err_perm(struct perm_bits *perm)
{
	u32 mask;

	if (alloc_perm_bits(perm, pci_ext_cap_length[PCI_EXT_CAP_ID_ERR]))
		return -ENOMEM;

	/*
	 * Virtualize the first dword of all express capabilities
	 * because it includes the next pointer.  This lets us later
	 * remove capabilities from the chain if we need to.
	 */
	p_setd(perm, 0, ALL_VIRT, NO_WRITE);

	/* Writable bits mask */
	mask =	PCI_ERR_UNC_UND |		/* Undefined */
		PCI_ERR_UNC_DLP |		/* Data Link Protocol */
		PCI_ERR_UNC_SURPDN |		/* Surprise Down */
		PCI_ERR_UNC_POISON_TLP |	/* Poisoned TLP */
		PCI_ERR_UNC_FCP |		/* Flow Control Protocol */
		PCI_ERR_UNC_COMP_TIME |		/* Completion Timeout */
		PCI_ERR_UNC_COMP_ABORT |	/* Completer Abort */
		PCI_ERR_UNC_UNX_COMP |		/* Unexpected Completion */
		PCI_ERR_UNC_RX_OVER |		/* Receiver Overflow */
		PCI_ERR_UNC_MALF_TLP |		/* Malformed TLP */
		PCI_ERR_UNC_ECRC |		/* ECRC Error Status */
		PCI_ERR_UNC_UNSUP |		/* Unsupported Request */
		PCI_ERR_UNC_ACSV |		/* ACS Violation */
		PCI_ERR_UNC_INTN |		/* internal error */
		PCI_ERR_UNC_MCBTLP |		/* MC blocked TLP */
		PCI_ERR_UNC_ATOMEG |		/* Atomic egress blocked */
		PCI_ERR_UNC_TLPPRE;		/* TLP prefix blocked */
	p_setd(perm, PCI_ERR_UNCOR_STATUS, NO_VIRT, mask);
	p_setd(perm, PCI_ERR_UNCOR_MASK, NO_VIRT, mask);
	p_setd(perm, PCI_ERR_UNCOR_SEVER, NO_VIRT, mask);

	mask =	PCI_ERR_COR_RCVR |		/* Receiver Error Status */
		PCI_ERR_COR_BAD_TLP |		/* Bad TLP Status */
		PCI_ERR_COR_BAD_DLLP |		/* Bad DLLP Status */
		PCI_ERR_COR_REP_ROLL |		/* REPLAY_NUM Rollover */
		PCI_ERR_COR_REP_TIMER |		/* Replay Timer Timeout */
		PCI_ERR_COR_ADV_NFAT |		/* Advisory Non-Fatal */
		PCI_ERR_COR_INTERNAL |		/* Corrected Internal */
		PCI_ERR_COR_LOG_OVER;		/* Header Log Overflow */
	p_setd(perm, PCI_ERR_COR_STATUS, NO_VIRT, mask);
	p_setd(perm, PCI_ERR_COR_MASK, NO_VIRT, mask);

	mask =	PCI_ERR_CAP_ECRC_GENE |		/* ECRC Generation Enable */
		PCI_ERR_CAP_ECRC_CHKE;		/* ECRC Check Enable */
	p_setd(perm, PCI_ERR_CAP, NO_VIRT, mask);
	return 0;
}

/* Permissions for Power Budgeting extended capability */
static int __init init_pci_ext_cap_pwr_perm(struct perm_bits *perm)
{
	if (alloc_perm_bits(perm, pci_ext_cap_length[PCI_EXT_CAP_ID_PWR]))
		return -ENOMEM;

	p_setd(perm, 0, ALL_VIRT, NO_WRITE);

	/* Writing the data selector is OK, the info is still read-only */
	p_setb(perm, PCI_PWR_DATA, NO_VIRT, (u8)ALL_WRITE);
	return 0;
}

/*
 * Initialize the shared permission tables
 */
void vfio_pci_uninit_perm_bits(void)
{
	free_perm_bits(&cap_perms[PCI_CAP_ID_BASIC]);

	free_perm_bits(&cap_perms[PCI_CAP_ID_PM]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_VPD]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_PCIX]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_EXP]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_AF]);

	free_perm_bits(&ecap_perms[PCI_EXT_CAP_ID_ERR]);
	free_perm_bits(&ecap_perms[PCI_EXT_CAP_ID_PWR]);
}

int __init vfio_pci_init_perm_bits(void)
{
	int ret;

	/* Basic config space */
	ret = init_pci_cap_basic_perm(&cap_perms[PCI_CAP_ID_BASIC]);

	/* Capabilities */
	ret |= init_pci_cap_pm_perm(&cap_perms[PCI_CAP_ID_PM]);
	ret |= init_pci_cap_vpd_perm(&cap_perms[PCI_CAP_ID_VPD]);
	ret |= init_pci_cap_pcix_perm(&cap_perms[PCI_CAP_ID_PCIX]);
	cap_perms[PCI_CAP_ID_VNDR].writefn = vfio_raw_config_write;
	ret |= init_pci_cap_exp_perm(&cap_perms[PCI_CAP_ID_EXP]);
	ret |= init_pci_cap_af_perm(&cap_perms[PCI_CAP_ID_AF]);

	/* Extended capabilities */
	ret |= init_pci_ext_cap_err_perm(&ecap_perms[PCI_EXT_CAP_ID_ERR]);
	ret |= init_pci_ext_cap_pwr_perm(&ecap_perms[PCI_EXT_CAP_ID_PWR]);
	ecap_perms[PCI_EXT_CAP_ID_VNDR].writefn = vfio_raw_config_write;

	if (ret)
		vfio_pci_uninit_perm_bits();

	return ret;
}

static int vfio_find_cap_start(struct vfio_pci_device *vdev, int pos)
{
	u8 cap;
	int base = (pos >= PCI_CFG_SPACE_SIZE) ? PCI_CFG_SPACE_SIZE :
						 PCI_STD_HEADER_SIZEOF;
	cap = vdev->pci_config_map[pos];

	if (cap == PCI_CAP_ID_BASIC)
		return 0;

	/* XXX Can we have to abutting capabilities of the same type? */
	while (pos - 1 >= base && vdev->pci_config_map[pos - 1] == cap)
		pos--;

	return pos;
}

static int vfio_msi_config_read(struct vfio_pci_device *vdev, int pos,
				int count, struct perm_bits *perm,
				int offset, __le32 *val)
{
	/* Update max available queue size from msi_qmax */
	if (offset <= PCI_MSI_FLAGS && offset + count >= PCI_MSI_FLAGS) {
		__le16 *flags;
		int start;

		start = vfio_find_cap_start(vdev, pos);

		flags = (__le16 *)&vdev->vconfig[start];

		*flags &= cpu_to_le16(~PCI_MSI_FLAGS_QMASK);
		*flags |= cpu_to_le16(vdev->msi_qmax << 1);
	}

	return vfio_default_config_read(vdev, pos, count, perm, offset, val);
}

static int vfio_msi_config_write(struct vfio_pci_device *vdev, int pos,
				 int count, struct perm_bits *perm,
				 int offset, __le32 val)
{
	count = vfio_default_config_write(vdev, pos, count, perm, offset, val);
	if (count < 0)
		return count;

	/* Fixup and write configured queue size and enable to hardware */
	if (offset <= PCI_MSI_FLAGS && offset + count >= PCI_MSI_FLAGS) {
		__le16 *pflags;
		u16 flags;
		int start, ret;

		start = vfio_find_cap_start(vdev, pos);

		pflags = (__le16 *)&vdev->vconfig[start + PCI_MSI_FLAGS];

		flags = le16_to_cpu(*pflags);

		/* MSI is enabled via ioctl */
		if  (!is_msi(vdev))
			flags &= ~PCI_MSI_FLAGS_ENABLE;

		/* Check queue size */
		if ((flags & PCI_MSI_FLAGS_QSIZE) >> 4 > vdev->msi_qmax) {
			flags &= ~PCI_MSI_FLAGS_QSIZE;
			flags |= vdev->msi_qmax << 4;
		}
	if (*ctrl & PCI_AF_CTRL_FLR) {
		u8 cap;
		int ret;

		*ctrl &= ~PCI_AF_CTRL_FLR;

		ret = pci_user_read_config_byte(vdev->pdev,
						pos - offset + PCI_AF_CAP,
						&cap);

		if (!ret && (cap & PCI_AF_CAP_FLR) && (cap & PCI_AF_CAP_TP))
			pci_try_reset_function(vdev->pdev);
	}

	return count;
}

/* Permissions for Advanced Function capability */
static int __init init_pci_cap_af_perm(struct perm_bits *perm)
{
	if (alloc_perm_bits(perm, pci_cap_length[PCI_CAP_ID_AF]))
		return -ENOMEM;

	perm->writefn = vfio_af_config_write;

	p_setb(perm, PCI_CAP_LIST_NEXT, (u8)ALL_VIRT, NO_WRITE);
	p_setb(perm, PCI_AF_CTRL, PCI_AF_CTRL_FLR, PCI_AF_CTRL_FLR);
	return 0;
}

/* Permissions for Advanced Error Reporting extended capability */
static int __init init_pci_ext_cap_err_perm(struct perm_bits *perm)
{
	u32 mask;

	if (alloc_perm_bits(perm, pci_ext_cap_length[PCI_EXT_CAP_ID_ERR]))
		return -ENOMEM;

	/*
	 * Virtualize the first dword of all express capabilities
	 * because it includes the next pointer.  This lets us later
	 * remove capabilities from the chain if we need to.
	 */
	p_setd(perm, 0, ALL_VIRT, NO_WRITE);

	/* Writable bits mask */
	mask =	PCI_ERR_UNC_UND |		/* Undefined */
		PCI_ERR_UNC_DLP |		/* Data Link Protocol */
		PCI_ERR_UNC_SURPDN |		/* Surprise Down */
		PCI_ERR_UNC_POISON_TLP |	/* Poisoned TLP */
		PCI_ERR_UNC_FCP |		/* Flow Control Protocol */
		PCI_ERR_UNC_COMP_TIME |		/* Completion Timeout */
		PCI_ERR_UNC_COMP_ABORT |	/* Completer Abort */
		PCI_ERR_UNC_UNX_COMP |		/* Unexpected Completion */
		PCI_ERR_UNC_RX_OVER |		/* Receiver Overflow */
		PCI_ERR_UNC_MALF_TLP |		/* Malformed TLP */
		PCI_ERR_UNC_ECRC |		/* ECRC Error Status */
		PCI_ERR_UNC_UNSUP |		/* Unsupported Request */
		PCI_ERR_UNC_ACSV |		/* ACS Violation */
		PCI_ERR_UNC_INTN |		/* internal error */
		PCI_ERR_UNC_MCBTLP |		/* MC blocked TLP */
		PCI_ERR_UNC_ATOMEG |		/* Atomic egress blocked */
		PCI_ERR_UNC_TLPPRE;		/* TLP prefix blocked */
	p_setd(perm, PCI_ERR_UNCOR_STATUS, NO_VIRT, mask);
	p_setd(perm, PCI_ERR_UNCOR_MASK, NO_VIRT, mask);
	p_setd(perm, PCI_ERR_UNCOR_SEVER, NO_VIRT, mask);

	mask =	PCI_ERR_COR_RCVR |		/* Receiver Error Status */
		PCI_ERR_COR_BAD_TLP |		/* Bad TLP Status */
		PCI_ERR_COR_BAD_DLLP |		/* Bad DLLP Status */
		PCI_ERR_COR_REP_ROLL |		/* REPLAY_NUM Rollover */
		PCI_ERR_COR_REP_TIMER |		/* Replay Timer Timeout */
		PCI_ERR_COR_ADV_NFAT |		/* Advisory Non-Fatal */
		PCI_ERR_COR_INTERNAL |		/* Corrected Internal */
		PCI_ERR_COR_LOG_OVER;		/* Header Log Overflow */
	p_setd(perm, PCI_ERR_COR_STATUS, NO_VIRT, mask);
	p_setd(perm, PCI_ERR_COR_MASK, NO_VIRT, mask);

	mask =	PCI_ERR_CAP_ECRC_GENE |		/* ECRC Generation Enable */
		PCI_ERR_CAP_ECRC_CHKE;		/* ECRC Check Enable */
	p_setd(perm, PCI_ERR_CAP, NO_VIRT, mask);
	return 0;
}

/* Permissions for Power Budgeting extended capability */
static int __init init_pci_ext_cap_pwr_perm(struct perm_bits *perm)
{
	if (alloc_perm_bits(perm, pci_ext_cap_length[PCI_EXT_CAP_ID_PWR]))
		return -ENOMEM;

	p_setd(perm, 0, ALL_VIRT, NO_WRITE);

	/* Writing the data selector is OK, the info is still read-only */
	p_setb(perm, PCI_PWR_DATA, NO_VIRT, (u8)ALL_WRITE);
	return 0;
}

/*
 * Initialize the shared permission tables
 */
void vfio_pci_uninit_perm_bits(void)
{
	free_perm_bits(&cap_perms[PCI_CAP_ID_BASIC]);

	free_perm_bits(&cap_perms[PCI_CAP_ID_PM]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_VPD]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_PCIX]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_EXP]);
	free_perm_bits(&cap_perms[PCI_CAP_ID_AF]);

	free_perm_bits(&ecap_perms[PCI_EXT_CAP_ID_ERR]);
	free_perm_bits(&ecap_perms[PCI_EXT_CAP_ID_PWR]);
}

int __init vfio_pci_init_perm_bits(void)
{
	int ret;

	/* Basic config space */
	ret = init_pci_cap_basic_perm(&cap_perms[PCI_CAP_ID_BASIC]);

	/* Capabilities */
	ret |= init_pci_cap_pm_perm(&cap_perms[PCI_CAP_ID_PM]);
	ret |= init_pci_cap_vpd_perm(&cap_perms[PCI_CAP_ID_VPD]);
	ret |= init_pci_cap_pcix_perm(&cap_perms[PCI_CAP_ID_PCIX]);
	cap_perms[PCI_CAP_ID_VNDR].writefn = vfio_raw_config_write;
	ret |= init_pci_cap_exp_perm(&cap_perms[PCI_CAP_ID_EXP]);
	ret |= init_pci_cap_af_perm(&cap_perms[PCI_CAP_ID_AF]);

	/* Extended capabilities */
	ret |= init_pci_ext_cap_err_perm(&ecap_perms[PCI_EXT_CAP_ID_ERR]);
	ret |= init_pci_ext_cap_pwr_perm(&ecap_perms[PCI_EXT_CAP_ID_PWR]);
	ecap_perms[PCI_EXT_CAP_ID_VNDR].writefn = vfio_raw_config_write;

	if (ret)
		vfio_pci_uninit_perm_bits();

	return ret;
}

static int vfio_find_cap_start(struct vfio_pci_device *vdev, int pos)
{
	u8 cap;
	int base = (pos >= PCI_CFG_SPACE_SIZE) ? PCI_CFG_SPACE_SIZE :
						 PCI_STD_HEADER_SIZEOF;
	cap = vdev->pci_config_map[pos];

	if (cap == PCI_CAP_ID_BASIC)
		return 0;

	/* XXX Can we have to abutting capabilities of the same type? */
	while (pos - 1 >= base && vdev->pci_config_map[pos - 1] == cap)
		pos--;

	return pos;
}

static int vfio_msi_config_read(struct vfio_pci_device *vdev, int pos,
				int count, struct perm_bits *perm,
				int offset, __le32 *val)
{
	/* Update max available queue size from msi_qmax */
	if (offset <= PCI_MSI_FLAGS && offset + count >= PCI_MSI_FLAGS) {
		__le16 *flags;
		int start;

		start = vfio_find_cap_start(vdev, pos);

		flags = (__le16 *)&vdev->vconfig[start];

		*flags &= cpu_to_le16(~PCI_MSI_FLAGS_QMASK);
		*flags |= cpu_to_le16(vdev->msi_qmax << 1);
	}

	return vfio_default_config_read(vdev, pos, count, perm, offset, val);
}

static int vfio_msi_config_write(struct vfio_pci_device *vdev, int pos,
				 int count, struct perm_bits *perm,
				 int offset, __le32 val)
{
	count = vfio_default_config_write(vdev, pos, count, perm, offset, val);
	if (count < 0)
		return count;

	/* Fixup and write configured queue size and enable to hardware */
	if (offset <= PCI_MSI_FLAGS && offset + count >= PCI_MSI_FLAGS) {
		__le16 *pflags;
		u16 flags;
		int start, ret;

		start = vfio_find_cap_start(vdev, pos);

		pflags = (__le16 *)&vdev->vconfig[start + PCI_MSI_FLAGS];

		flags = le16_to_cpu(*pflags);

		/* MSI is enabled via ioctl */
		if  (!is_msi(vdev))
			flags &= ~PCI_MSI_FLAGS_ENABLE;

		/* Check queue size */
		if ((flags & PCI_MSI_FLAGS_QSIZE) >> 4 > vdev->msi_qmax) {
			flags &= ~PCI_MSI_FLAGS_QSIZE;
			flags |= vdev->msi_qmax << 4;
		}