	*(__le32 *)(&p->write[off]) = cpu_to_le32(write);
}

/*
 * Restore the *real* BARs after we detect a FLR or backdoor reset.
 * (backdoor = some device specific technique that we didn't catch)
 */

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
	}

	count = vfio_default_config_write(vdev, pos, count, perm, offset, val);
	if (count < 0)
		return count;

	/*
	 * Save current memory/io enable bits in vconfig to allow for
	 * the test above next time.

		*virt_cmd &= cpu_to_le16(~mask);
		*virt_cmd |= cpu_to_le16(new_cmd & mask);
	}

	/* Emulate INTx disable */
	if (offset >= PCI_COMMAND && offset <= PCI_COMMAND + 1) {
						 pos - offset + PCI_EXP_DEVCAP,
						 &cap);

		if (!ret && (cap & PCI_EXP_DEVCAP_FLR))
			pci_try_reset_function(vdev->pdev);
	}

	/*
	 * MPS is virtualized to the user, writes do not change the physical
						pos - offset + PCI_AF_CAP,
						&cap);

		if (!ret && (cap & PCI_AF_CAP_FLR) && (cap & PCI_AF_CAP_TP))
			pci_try_reset_function(vdev->pdev);
	}

	return count;
}