	int rc;

	dev_dbg(&pdev->dev, "%s called\n", __func__);
	rc = check_vma(wq, vma, __func__);
	if (rc < 0)
		return rc;

			vma->vm_page_prot);
}

static __poll_t idxd_cdev_poll(struct file *filp,
			       struct poll_table_struct *wait)
{
	struct idxd_user_context *ctx = filp->private_data;
	.open = idxd_cdev_open,
	.release = idxd_cdev_release,
	.mmap = idxd_cdev_mmap,
	.poll = idxd_cdev_poll,
};

int idxd_cdev_get_major(struct idxd_device *idxd)