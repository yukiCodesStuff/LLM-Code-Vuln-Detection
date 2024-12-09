{
	struct idxd_user_context *ctx = filp->private_data;
	struct idxd_wq *wq = ctx->wq;
	struct idxd_device *idxd = wq->idxd;
	struct pci_dev *pdev = idxd->pdev;
	phys_addr_t base = pci_resource_start(pdev, IDXD_WQ_BAR);
	unsigned long pfn;
	int rc;

	dev_dbg(&pdev->dev, "%s called\n", __func__);

	/*
	 * Due to an erratum in some of the devices supported by the driver,
	 * direct user submission to the device can be unsafe.
	 * (See the INTEL-SA-01084 security advisory)
	 *
	 * For the devices that exhibit this behavior, require that the user
	 * has CAP_SYS_RAWIO capabilities.
	 */
	if (!idxd->user_submission_safe && !capable(CAP_SYS_RAWIO))
		return -EPERM;

	rc = check_vma(wq, vma, __func__);
	if (rc < 0)
		return rc;

	vm_flags_set(vma, VM_DONTCOPY);
	pfn = (base + idxd_get_wq_portal_full_offset(wq->id,
				IDXD_PORTAL_LIMITED)) >> PAGE_SHIFT;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vma->vm_private_data = ctx;

	return io_remap_pfn_range(vma, vma->vm_start, pfn, PAGE_SIZE,
			vma->vm_page_prot);
}

static int idxd_submit_user_descriptor(struct idxd_user_context *ctx,
				       struct dsa_hw_desc __user *udesc)
{
	struct idxd_wq *wq = ctx->wq;
	struct idxd_dev *idxd_dev = &wq->idxd->idxd_dev;
	const uint64_t comp_addr_align = is_dsa_dev(idxd_dev) ? 0x20 : 0x40;
	void __iomem *portal = idxd_wq_portal_addr(wq);
	struct dsa_hw_desc descriptor __aligned(64);
	int rc;

	rc = copy_from_user(&descriptor, udesc, sizeof(descriptor));
	if (rc)
		return -EFAULT;

	/*
	 * DSA devices are capable of indirect ("batch") command submission.
	 * On devices where direct user submissions are not safe, we cannot
	 * allow this since there is no good way for us to verify these
	 * indirect commands.
	 */
	if (is_dsa_dev(idxd_dev) && descriptor.opcode == DSA_OPCODE_BATCH &&
		!wq->idxd->user_submission_safe)
		return -EINVAL;
	/*
	 * As per the programming specification, the completion address must be
	 * aligned to 32 or 64 bytes. If this is violated the hardware
	 * engine can get very confused (security issue).
	 */
	if (!IS_ALIGNED(descriptor.completion_addr, comp_addr_align))
		return -EINVAL;

	if (wq_dedicated(wq))
		iosubmit_cmds512(portal, &descriptor, 1);
	else {
		descriptor.priv = 0;
		descriptor.pasid = ctx->pasid;
		rc = idxd_enqcmds(wq, portal, &descriptor);
		if (rc < 0)
			return rc;
	}
{
	struct idxd_user_context *ctx = filp->private_data;
	struct idxd_wq *wq = ctx->wq;
	struct idxd_device *idxd = wq->idxd;
	struct pci_dev *pdev = idxd->pdev;
	phys_addr_t base = pci_resource_start(pdev, IDXD_WQ_BAR);
	unsigned long pfn;
	int rc;

	dev_dbg(&pdev->dev, "%s called\n", __func__);

	/*
	 * Due to an erratum in some of the devices supported by the driver,
	 * direct user submission to the device can be unsafe.
	 * (See the INTEL-SA-01084 security advisory)
	 *
	 * For the devices that exhibit this behavior, require that the user
	 * has CAP_SYS_RAWIO capabilities.
	 */
	if (!idxd->user_submission_safe && !capable(CAP_SYS_RAWIO))
		return -EPERM;

	rc = check_vma(wq, vma, __func__);
	if (rc < 0)
		return rc;

	vm_flags_set(vma, VM_DONTCOPY);
	pfn = (base + idxd_get_wq_portal_full_offset(wq->id,
				IDXD_PORTAL_LIMITED)) >> PAGE_SHIFT;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vma->vm_private_data = ctx;

	return io_remap_pfn_range(vma, vma->vm_start, pfn, PAGE_SIZE,
			vma->vm_page_prot);
}

static int idxd_submit_user_descriptor(struct idxd_user_context *ctx,
				       struct dsa_hw_desc __user *udesc)
{
	struct idxd_wq *wq = ctx->wq;
	struct idxd_dev *idxd_dev = &wq->idxd->idxd_dev;
	const uint64_t comp_addr_align = is_dsa_dev(idxd_dev) ? 0x20 : 0x40;
	void __iomem *portal = idxd_wq_portal_addr(wq);
	struct dsa_hw_desc descriptor __aligned(64);
	int rc;

	rc = copy_from_user(&descriptor, udesc, sizeof(descriptor));
	if (rc)
		return -EFAULT;

	/*
	 * DSA devices are capable of indirect ("batch") command submission.
	 * On devices where direct user submissions are not safe, we cannot
	 * allow this since there is no good way for us to verify these
	 * indirect commands.
	 */
	if (is_dsa_dev(idxd_dev) && descriptor.opcode == DSA_OPCODE_BATCH &&
		!wq->idxd->user_submission_safe)
		return -EINVAL;
	/*
	 * As per the programming specification, the completion address must be
	 * aligned to 32 or 64 bytes. If this is violated the hardware
	 * engine can get very confused (security issue).
	 */
	if (!IS_ALIGNED(descriptor.completion_addr, comp_addr_align))
		return -EINVAL;

	if (wq_dedicated(wq))
		iosubmit_cmds512(portal, &descriptor, 1);
	else {
		descriptor.priv = 0;
		descriptor.pasid = ctx->pasid;
		rc = idxd_enqcmds(wq, portal, &descriptor);
		if (rc < 0)
			return rc;
	}
static const struct file_operations idxd_cdev_fops = {
	.owner = THIS_MODULE,
	.open = idxd_cdev_open,
	.release = idxd_cdev_release,
	.mmap = idxd_cdev_mmap,
	.write = idxd_cdev_write,
	.poll = idxd_cdev_poll,
};

int idxd_cdev_get_major(struct idxd_device *idxd)
{
	return MAJOR(ictx[idxd->data->type].devt);
}