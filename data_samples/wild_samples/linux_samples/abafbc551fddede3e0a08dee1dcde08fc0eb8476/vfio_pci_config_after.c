{
	*(__le32 *)(&p->virt[off]) = cpu_to_le32(virt);
	*(__le32 *)(&p->write[off]) = cpu_to_le32(write);
}

/* Caller should hold memory_lock semaphore */
bool __vfio_pci_memory_enabled(struct vfio_pci_device *vdev)
{
	u16 cmd = le16_to_cpu(*(__le16 *)&vdev->vconfig[PCI_COMMAND]);

	return cmd & PCI_COMMAND_MEMORY;
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

		phys_io = !!(phys_cmd & PCI_COMMAND_IO);
		virt_io = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_IO);
		new_io = !!(new_cmd & PCI_COMMAND_IO);

		phys_mem = !!(phys_cmd & PCI_COMMAND_MEMORY);
		virt_mem = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_MEMORY);
		new_mem = !!(new_cmd & PCI_COMMAND_MEMORY);

		if (!new_mem)
			vfio_pci_zap_and_down_write_memory_lock(vdev);
		else
			down_write(&vdev->memory_lock);

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
	if (count < 0) {
		if (offset == PCI_COMMAND)
			up_write(&vdev->memory_lock);
		return count;
	}

	/*
	 * Save current memory/io enable bits in vconfig to allow for
	 * the test above next time.
	 */
	if (offset == PCI_COMMAND) {
		u16 mask = PCI_COMMAND_MEMORY | PCI_COMMAND_IO;

		*virt_cmd &= cpu_to_le16(~mask);
		*virt_cmd |= cpu_to_le16(new_cmd & mask);

		up_write(&vdev->memory_lock);
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

		phys_io = !!(phys_cmd & PCI_COMMAND_IO);
		virt_io = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_IO);
		new_io = !!(new_cmd & PCI_COMMAND_IO);

		phys_mem = !!(phys_cmd & PCI_COMMAND_MEMORY);
		virt_mem = !!(le16_to_cpu(*virt_cmd) & PCI_COMMAND_MEMORY);
		new_mem = !!(new_cmd & PCI_COMMAND_MEMORY);

		if (!new_mem)
			vfio_pci_zap_and_down_write_memory_lock(vdev);
		else
			down_write(&vdev->memory_lock);

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
	if (count < 0) {
		if (offset == PCI_COMMAND)
			up_write(&vdev->memory_lock);
		return count;
	}
	if (offset == PCI_COMMAND) {
		u16 mask = PCI_COMMAND_MEMORY | PCI_COMMAND_IO;

		*virt_cmd &= cpu_to_le16(~mask);
		*virt_cmd |= cpu_to_le16(new_cmd & mask);

		up_write(&vdev->memory_lock);
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

		if (!ret && (cap & PCI_EXP_DEVCAP_FLR)) {
			vfio_pci_zap_and_down_write_memory_lock(vdev);
			pci_try_reset_function(vdev->pdev);
			up_write(&vdev->memory_lock);
		}
	if (*ctrl & PCI_AF_CTRL_FLR) {
		u8 cap;
		int ret;

		*ctrl &= ~PCI_AF_CTRL_FLR;

		ret = pci_user_read_config_byte(vdev->pdev,
						pos - offset + PCI_AF_CAP,
						&cap);

		if (!ret && (cap & PCI_AF_CAP_FLR) && (cap & PCI_AF_CAP_TP)) {
			vfio_pci_zap_and_down_write_memory_lock(vdev);
			pci_try_reset_function(vdev->pdev);
			up_write(&vdev->memory_lock);
		}