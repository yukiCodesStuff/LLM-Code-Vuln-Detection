		rc = pci_add_dynid(&vfio_pci_driver, vendor, device,
				   subvendor, subdevice, class, class_mask, 0);
		if (rc)
			pr_warn("failed to add dynamic id [%04hx:%04hx[%04hx:%04hx]] class %#08x/%08x (%d)\n",
				vendor, device, subvendor, subdevice,
				class, class_mask, rc);
		else
			pr_info("add [%04hx:%04hx[%04hx:%04hx]] class %#08x/%08x\n",
				vendor, device, subvendor, subdevice,
				class, class_mask);
	}
}