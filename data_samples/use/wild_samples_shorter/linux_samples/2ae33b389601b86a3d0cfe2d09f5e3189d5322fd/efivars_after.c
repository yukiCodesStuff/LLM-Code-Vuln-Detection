	return status;
}

static efi_status_t
check_var_size_locked(struct efivars *efivars, u32 attributes,
			unsigned long size)
{
	u64 storage_size, remaining_size, max_size;
	efi_status_t status;
	const struct efivar_operations *fops = efivars->ops;

	if (!efivars->ops->query_variable_info)
		return EFI_UNSUPPORTED;

	status = fops->query_variable_info(attributes, &storage_size,
					   &remaining_size, &max_size);

	if (status != EFI_SUCCESS)
		return status;

	if (!storage_size || size > remaining_size || size > max_size ||
	    (remaining_size - size) < (storage_size / 2))
		return EFI_OUT_OF_RESOURCES;

	return status;
}


static efi_status_t
check_var_size(struct efivars *efivars, u32 attributes, unsigned long size)
{
	efi_status_t status;
	unsigned long flags;

	spin_lock_irqsave(&efivars->lock, flags);
	status = check_var_size_locked(efivars, attributes, size);
	spin_unlock_irqrestore(&efivars->lock, flags);

	return status;
}

static ssize_t
efivar_guid_read(struct efivar_entry *entry, char *buf)
{
	struct efi_variable *var = &entry->var;
	}

	spin_lock_irq(&efivars->lock);

	status = check_var_size_locked(efivars, new_var->Attributes,
	       new_var->DataSize + utf16_strsize(new_var->VariableName, 1024));

	if (status == EFI_SUCCESS || status == EFI_UNSUPPORTED)
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
	unsigned long newdatasize, varsize;
	ssize_t bytes = 0;

	if (count < sizeof(attributes))
		return -EINVAL;
	 * amounts of memory. Pick a default size of 64K if
	 * QueryVariableInfo() isn't supported by the firmware.
	 */

	varsize = datasize + utf16_strsize(var->var.VariableName, 1024);
	status = check_var_size(efivars, attributes, varsize);

	if (status != EFI_SUCCESS) {
		if (status != EFI_UNSUPPORTED)
			return efi_status_to_err(status);

		if (datasize > 65536)
			return -ENOSPC;
	}

	data = kmalloc(datasize, GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	 */
	spin_lock_irq(&efivars->lock);

	/*
	 * Ensure that the available space hasn't shrunk below the safe level
	 */

	status = check_var_size_locked(efivars, attributes, varsize);

	if (status != EFI_SUCCESS && status != EFI_UNSUPPORTED) {
		spin_unlock_irq(&efivars->lock);
		kfree(data);

		return efi_status_to_err(status);
	}

	status = efivars->ops->set_variable(var->var.VariableName,
					    &var->var.VendorGuid,
					    attributes, datasize,
					    data);
	if (len < GUID_LEN + 2)
		return false;

	/* GUID must be preceded by a '-' */
	if (*(s - 1) != '-')
		return false;

	/*
	 * Validate that 's' is of the correct format, e.g.

static struct dentry *efivarfs_alloc_dentry(struct dentry *parent, char *name)
{
	struct dentry *d;
	struct qstr q;
	int err;

	q.name = name;
	q.len = strlen(name);

	err = efivarfs_d_hash(NULL, NULL, &q);
	if (err)
		return ERR_PTR(err);

	d = d_alloc(parent, &q);
	if (d)
		return d;

	return ERR_PTR(-ENOMEM);
}

static int efivarfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct efivar_entry *entry, *n;
	struct efivars *efivars = &__efivars;
	char *name;
	int err = -ENOMEM;

	efivarfs_sb = sb;

	sb->s_maxbytes          = MAX_LFS_FILESIZE;
			goto fail_name;

		dentry = efivarfs_alloc_dentry(root, name);
		if (IS_ERR(dentry)) {
			err = PTR_ERR(dentry);
			goto fail_inode;
		}

		/* copied by the above to local storage in the dentry. */
		kfree(name);

fail_name:
	kfree(name);
fail:
	return err;
}

static struct dentry *efivarfs_mount(struct file_system_type *fs_type,
				    int flags, const char *dev_name, void *data)
	.mount   = efivarfs_mount,
	.kill_sb = efivarfs_kill_sb,
};
MODULE_ALIAS_FS("efivarfs");

/*
 * Handle negative dentry.
 */
	efi_guid_t vendor = LINUX_EFI_CRASH_GUID;
	struct efivars *efivars = psi->data;
	int i, ret = 0;
	efi_status_t status = EFI_NOT_FOUND;
	unsigned long flags;

	if (pstore_cannot_block_path(reason)) {
	 * size: a size of logging data
	 * DUMP_NAME_LEN * 2: a maximum size of variable name
	 */

	status = check_var_size_locked(efivars, PSTORE_EFI_ATTRIBUTES,
					 size + DUMP_NAME_LEN * 2);

	if (status) {
		spin_unlock_irqrestore(&efivars->lock, flags);
		*id = part;
		return -ENOSPC;
	}
		return -EINVAL;
	}

	status = check_var_size_locked(efivars, new_var->Attributes,
	       new_var->DataSize + utf16_strsize(new_var->VariableName, 1024));

	if (status && status != EFI_UNSUPPORTED) {
		spin_unlock_irq(&efivars->lock);
		return efi_status_to_err(status);
	}

	/* now *really* create the variable via EFI */
	status = efivars->ops->set_variable(new_var->VariableName,
					    &new_var->VendorGuid,
					    new_var->Attributes,