{
	IOCTL32_Command_struct __user *arg32 =
	    (IOCTL32_Command_struct __user *) arg;
	IOCTL_Command_struct arg64;
	IOCTL_Command_struct __user *p = compat_alloc_user_space(sizeof(arg64));
	int err;
	u32 cp;

	err = 0;
	err |=
	    copy_from_user(&arg64.LUN_info, &arg32->LUN_info,
			   sizeof(arg64.LUN_info));
	err |=
	    copy_from_user(&arg64.Request, &arg32->Request,
			   sizeof(arg64.Request));
	err |=
	    copy_from_user(&arg64.error_info, &arg32->error_info,
			   sizeof(arg64.error_info));
	err |= get_user(arg64.buf_size, &arg32->buf_size);
	err |= get_user(cp, &arg32->buf);
	arg64.buf = compat_ptr(cp);
	err |= copy_to_user(p, &arg64, sizeof(arg64));

	if (err)
		return -EFAULT;

	err = cciss_ioctl(bdev, mode, CCISS_PASSTHRU, (unsigned long)p);
	if (err)
		return err;
	err |=
	    copy_in_user(&arg32->error_info, &p->error_info,
			 sizeof(arg32->error_info));
	if (err)
		return -EFAULT;
	return err;
}

static int cciss_ioctl32_big_passthru(struct block_device *bdev, fmode_t mode,
				      unsigned cmd, unsigned long arg)
{
	BIG_IOCTL32_Command_struct __user *arg32 =
	    (BIG_IOCTL32_Command_struct __user *) arg;
	BIG_IOCTL_Command_struct arg64;
	BIG_IOCTL_Command_struct __user *p =
	    compat_alloc_user_space(sizeof(arg64));
	int err;
	u32 cp;

	memset(&arg64, 0, sizeof(arg64));
	err = 0;
	err |=
	    copy_from_user(&arg64.LUN_info, &arg32->LUN_info,
			   sizeof(arg64.LUN_info));
	err |=
	    copy_from_user(&arg64.Request, &arg32->Request,
			   sizeof(arg64.Request));
	err |=
	    copy_from_user(&arg64.error_info, &arg32->error_info,
			   sizeof(arg64.error_info));
	err |= get_user(arg64.buf_size, &arg32->buf_size);
	err |= get_user(arg64.malloc_size, &arg32->malloc_size);
	err |= get_user(cp, &arg32->buf);
	arg64.buf = compat_ptr(cp);
	err |= copy_to_user(p, &arg64, sizeof(arg64));

	if (err)
		return -EFAULT;

	err = cciss_ioctl(bdev, mode, CCISS_BIG_PASSTHRU, (unsigned long)p);
	if (err)
		return err;
	err |=
	    copy_in_user(&arg32->error_info, &p->error_info,
			 sizeof(arg32->error_info));
	if (err)
		return -EFAULT;
	return err;
}
#endif

static int cciss_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	drive_info_struct *drv = get_drv(bdev->bd_disk);

	if (!drv->cylinders)
		return -ENXIO;

	geo->heads = drv->heads;
	geo->sectors = drv->sectors;
	geo->cylinders = drv->cylinders;
	return 0;
}

static void check_ioctl_unit_attention(ctlr_info_t *h, CommandList_struct *c)
{
	if (c->err_info->CommandStatus == CMD_TARGET_STATUS &&
			c->err_info->ScsiStatus != SAM_STAT_CHECK_CONDITION)
		(void)check_for_unit_attention(h, c);
}

static int cciss_getpciinfo(ctlr_info_t *h, void __user *argp)
{
	cciss_pci_info_struct pciinfo;

	if (!argp)
		return -EINVAL;
	pciinfo.domain = pci_domain_nr(h->pdev->bus);
	pciinfo.bus = h->pdev->bus->number;
	pciinfo.dev_fn = h->pdev->devfn;
	pciinfo.board_id = h->board_id;
	if (copy_to_user(argp, &pciinfo, sizeof(cciss_pci_info_struct)))
		return -EFAULT;
	return 0;
}

static int cciss_getintinfo(ctlr_info_t *h, void __user *argp)
{
	cciss_coalint_struct intinfo;
	unsigned long flags;

	if (!argp)
		return -EINVAL;
	spin_lock_irqsave(&h->lock, flags);
	intinfo.delay = readl(&h->cfgtable->HostWrite.CoalIntDelay);
	intinfo.count = readl(&h->cfgtable->HostWrite.CoalIntCount);
	spin_unlock_irqrestore(&h->lock, flags);
	if (copy_to_user
	    (argp, &intinfo, sizeof(cciss_coalint_struct)))
		return -EFAULT;
	return 0;
}

static int cciss_setintinfo(ctlr_info_t *h, void __user *argp)
{
	cciss_coalint_struct intinfo;
	unsigned long flags;
	int i;

	if (!argp)
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (copy_from_user(&intinfo, argp, sizeof(intinfo)))
		return -EFAULT;
	if ((intinfo.delay == 0) && (intinfo.count == 0))
		return -EINVAL;
	spin_lock_irqsave(&h->lock, flags);
	/* Update the field, and then ring the doorbell */
	writel(intinfo.delay, &(h->cfgtable->HostWrite.CoalIntDelay));
	writel(intinfo.count, &(h->cfgtable->HostWrite.CoalIntCount));
	writel(CFGTBL_ChangeReq, h->vaddr + SA5_DOORBELL);

	for (i = 0; i < MAX_IOCTL_CONFIG_WAIT; i++) {
		if (!(readl(h->vaddr + SA5_DOORBELL) & CFGTBL_ChangeReq))
			break;
		udelay(1000); /* delay and try again */
	}