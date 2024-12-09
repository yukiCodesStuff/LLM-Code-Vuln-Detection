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

		*virt_cmd &= cpu_to_le16(~mask);
		*virt_cmd |= cpu_to_le16(new_cmd & mask);

		up_write(&vdev->memory_lock);
	}

	/* Emulate INTx disable */
	if (offset >= PCI_COMMAND && offset <= PCI_COMMAND + 1) {
						 pos - offset + PCI_EXP_DEVCAP,
						 &cap);

		if (!ret && (cap & PCI_EXP_DEVCAP_FLR)) {
			vfio_pci_zap_and_down_write_memory_lock(vdev);
			pci_try_reset_function(vdev->pdev);
			up_write(&vdev->memory_lock);
		}
	}

	/*
	 * MPS is virtualized to the user, writes do not change the physical
						pos - offset + PCI_AF_CAP,
						&cap);

		if (!ret && (cap & PCI_AF_CAP_FLR) && (cap & PCI_AF_CAP_TP)) {
			vfio_pci_zap_and_down_write_memory_lock(vdev);
			pci_try_reset_function(vdev->pdev);
			up_write(&vdev->memory_lock);
		}
	}

	return count;
}