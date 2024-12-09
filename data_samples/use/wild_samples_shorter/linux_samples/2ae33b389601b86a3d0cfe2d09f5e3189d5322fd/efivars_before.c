	return status;
}

static ssize_t
efivar_guid_read(struct efivar_entry *entry, char *buf)
{
	struct efi_variable *var = &entry->var;
	}

	spin_lock_irq(&efivars->lock);
	status = efivars->ops->set_variable(new_var->VariableName,
					    &new_var->VendorGuid,
					    new_var->Attributes,
					    new_var->DataSize,
					    new_var->Data);

	spin_unlock_irq(&efivars->lock);

	if (status != EFI_SUCCESS) {
	u32 attributes;
	struct inode *inode = file->f_mapping->host;
	unsigned long datasize = count - sizeof(attributes);
	unsigned long newdatasize;
	u64 storage_size, remaining_size, max_size;
	ssize_t bytes = 0;

	if (count < sizeof(attributes))
		return -EINVAL;
	 * amounts of memory. Pick a default size of 64K if
	 * QueryVariableInfo() isn't supported by the firmware.
	 */
	spin_lock_irq(&efivars->lock);

	if (!efivars->ops->query_variable_info)
		status = EFI_UNSUPPORTED;
	else {
		const struct efivar_operations *fops = efivars->ops;
		status = fops->query_variable_info(attributes, &storage_size,
						   &remaining_size, &max_size);
	}

	spin_unlock_irq(&efivars->lock);

	if (status != EFI_SUCCESS) {
		if (status != EFI_UNSUPPORTED)
			return efi_status_to_err(status);

		remaining_size = 65536;
	}

	if (datasize > remaining_size)
		return -ENOSPC;

	data = kmalloc(datasize, GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	 */
	spin_lock_irq(&efivars->lock);

	status = efivars->ops->set_variable(var->var.VariableName,
					    &var->var.VendorGuid,
					    attributes, datasize,
					    data);
	if (len < GUID_LEN + 2)
		return false;

	/* GUID should be right after the first '-' */
	if (s - 1 != strchr(str, '-'))
		return false;

	/*
	 * Validate that 's' is of the correct format, e.g.

static struct dentry *efivarfs_alloc_dentry(struct dentry *parent, char *name)
{
	struct qstr q;

	q.name = name;
	q.len = strlen(name);

	if (efivarfs_d_hash(NULL, NULL, &q))
		return NULL;

	return d_alloc(parent, &q);
}

static int efivarfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct efivar_entry *entry, *n;
	struct efivars *efivars = &__efivars;
	char *name;

	efivarfs_sb = sb;

	sb->s_maxbytes          = MAX_LFS_FILESIZE;
			goto fail_name;

		dentry = efivarfs_alloc_dentry(root, name);
		if (!dentry)
			goto fail_inode;

		/* copied by the above to local storage in the dentry. */
		kfree(name);

fail_name:
	kfree(name);
fail:
	return -ENOMEM;
}

static struct dentry *efivarfs_mount(struct file_system_type *fs_type,
				    int flags, const char *dev_name, void *data)
	.mount   = efivarfs_mount,
	.kill_sb = efivarfs_kill_sb,
};

/*
 * Handle negative dentry.
 */
	efi_guid_t vendor = LINUX_EFI_CRASH_GUID;
	struct efivars *efivars = psi->data;
	int i, ret = 0;
	u64 storage_space, remaining_space, max_variable_size;
	efi_status_t status = EFI_NOT_FOUND;
	unsigned long flags;

	if (pstore_cannot_block_path(reason)) {
	 * size: a size of logging data
	 * DUMP_NAME_LEN * 2: a maximum size of variable name
	 */
	status = efivars->ops->query_variable_info(PSTORE_EFI_ATTRIBUTES,
						   &storage_space,
						   &remaining_space,
						   &max_variable_size);
	if (status || remaining_space < size + DUMP_NAME_LEN * 2) {
		spin_unlock_irqrestore(&efivars->lock, flags);
		*id = part;
		return -ENOSPC;
	}
		return -EINVAL;
	}

	/* now *really* create the variable via EFI */
	status = efivars->ops->set_variable(new_var->VariableName,
					    &new_var->VendorGuid,
					    new_var->Attributes,