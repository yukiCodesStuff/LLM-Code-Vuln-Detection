	if (!r) {
		dev->irq_requested_type |= guest_irq_type;
		if (dev->ack_notifier.gsi != -1)
			kvm_register_irq_ack_notifier(kvm, &dev->ack_notifier);
	} else
		kvm_free_irq_source_id(kvm, dev->irq_source_id);

	return r;
}

/* TODO Deal with KVM_DEV_IRQ_ASSIGNED_MASK_MSIX */
static int kvm_vm_ioctl_assign_irq(struct kvm *kvm,
				   struct kvm_assigned_irq *assigned_irq)
{
	int r = -EINVAL;
	struct kvm_assigned_dev_kernel *match;
	unsigned long host_irq_type, guest_irq_type;

	if (!irqchip_in_kernel(kvm))
		return r;

	mutex_lock(&kvm->lock);
	r = -ENODEV;
	match = kvm_find_assigned_dev(&kvm->arch.assigned_dev_head,
				      assigned_irq->assigned_dev_id);
	if (!match)
		goto out;

	host_irq_type = (assigned_irq->flags & KVM_DEV_IRQ_HOST_MASK);
	guest_irq_type = (assigned_irq->flags & KVM_DEV_IRQ_GUEST_MASK);

	r = -EINVAL;
	/* can only assign one type at a time */
	if (hweight_long(host_irq_type) > 1)
		goto out;
	if (hweight_long(guest_irq_type) > 1)
		goto out;
	if (host_irq_type == 0 && guest_irq_type == 0)
		goto out;

	r = 0;
	if (host_irq_type)
		r = assign_host_irq(kvm, match, host_irq_type);
	if (r)
		goto out;

	if (guest_irq_type)
		r = assign_guest_irq(kvm, match, assigned_irq, guest_irq_type);
out:
	mutex_unlock(&kvm->lock);
	return r;
}

static int kvm_vm_ioctl_deassign_dev_irq(struct kvm *kvm,
					 struct kvm_assigned_irq
					 *assigned_irq)
{
	int r = -ENODEV;
	struct kvm_assigned_dev_kernel *match;
	unsigned long irq_type;

	mutex_lock(&kvm->lock);

	match = kvm_find_assigned_dev(&kvm->arch.assigned_dev_head,
				      assigned_irq->assigned_dev_id);
	if (!match)
		goto out;

	irq_type = assigned_irq->flags & (KVM_DEV_IRQ_HOST_MASK |
					  KVM_DEV_IRQ_GUEST_MASK);
	r = kvm_deassign_irq(kvm, match, irq_type);
out:
	mutex_unlock(&kvm->lock);
	return r;
}

/*
 * We want to test whether the caller has been granted permissions to
 * use this device.  To be able to configure and control the device,
 * the user needs access to PCI configuration space and BAR resources.
 * These are accessed through PCI sysfs.  PCI config space is often
 * passed to the process calling this ioctl via file descriptor, so we
 * can't rely on access to that file.  We can check for permissions
 * on each of the BAR resource files, which is a pretty clear
 * indicator that the user has been granted access to the device.
 */
static int probe_sysfs_permissions(struct pci_dev *dev)
{
#ifdef CONFIG_SYSFS
	int i;
	bool bar_found = false;

	for (i = PCI_STD_RESOURCES; i <= PCI_STD_RESOURCE_END; i++) {
		char *kpath, *syspath;
		struct path path;
		struct inode *inode;
		int r;

		if (!pci_resource_len(dev, i))
			continue;

		kpath = kobject_get_path(&dev->dev.kobj, GFP_KERNEL);
		if (!kpath)
			return -ENOMEM;

		/* Per sysfs-rules, sysfs is always at /sys */
		syspath = kasprintf(GFP_KERNEL, "/sys%s/resource%d", kpath, i);
		kfree(kpath);
		if (!syspath)
			return -ENOMEM;

		r = kern_path(syspath, LOOKUP_FOLLOW, &path);
		kfree(syspath);
		if (r)
			return r;

		inode = path.dentry->d_inode;

		r = inode_permission(inode, MAY_READ | MAY_WRITE | MAY_ACCESS);
		path_put(&path);
		if (r)
			return r;

		bar_found = true;
	}