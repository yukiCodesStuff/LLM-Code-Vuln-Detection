	if (status != EFI_SUCCESS) {
		printk(KERN_WARNING "efivars: get_variable() failed 0x%lx!\n",
			status);
	}
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
	char *str = buf;

	if (!entry || !buf)
		return 0;

	efi_guid_unparse(&var->VendorGuid, str);
	str += strlen(str);
	str += sprintf(str, "\n");

	return str - buf;
}

static ssize_t
efivar_attr_read(struct efivar_entry *entry, char *buf)
{
	struct efi_variable *var = &entry->var;
	char *str = buf;
	efi_status_t status;

	if (!entry || !buf)
		return -EINVAL;

	status = get_var_data(entry->efivars, var);
	if (status != EFI_SUCCESS)
		return -EIO;

	if (var->Attributes & EFI_VARIABLE_NON_VOLATILE)
		str += sprintf(str, "EFI_VARIABLE_NON_VOLATILE\n");
	if (var->Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS)
		str += sprintf(str, "EFI_VARIABLE_BOOTSERVICE_ACCESS\n");
	if (var->Attributes & EFI_VARIABLE_RUNTIME_ACCESS)
		str += sprintf(str, "EFI_VARIABLE_RUNTIME_ACCESS\n");
	if (var->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD)
		str += sprintf(str, "EFI_VARIABLE_HARDWARE_ERROR_RECORD\n");
	if (var->Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS)
		str += sprintf(str,
			"EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS\n");
	if (var->Attributes &
			EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)
		str += sprintf(str,
			"EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS\n");
	if (var->Attributes & EFI_VARIABLE_APPEND_WRITE)
		str += sprintf(str, "EFI_VARIABLE_APPEND_WRITE\n");
	return str - buf;
}

static ssize_t
efivar_size_read(struct efivar_entry *entry, char *buf)
{
	struct efi_variable *var = &entry->var;
	char *str = buf;
	efi_status_t status;

	if (!entry || !buf)
		return -EINVAL;

	status = get_var_data(entry->efivars, var);
	if (status != EFI_SUCCESS)
		return -EIO;

	str += sprintf(str, "0x%lx\n", var->DataSize);
	return str - buf;
}

static ssize_t
efivar_data_read(struct efivar_entry *entry, char *buf)
{
	struct efi_variable *var = &entry->var;
	efi_status_t status;

	if (!entry || !buf)
		return -EINVAL;

	status = get_var_data(entry->efivars, var);
	if (status != EFI_SUCCESS)
		return -EIO;

	memcpy(buf, var->Data, var->DataSize);
	return var->DataSize;
}
/*
 * We allow each variable to be edited via rewriting the
 * entire efi variable structure.
 */
static ssize_t
efivar_store_raw(struct efivar_entry *entry, const char *buf, size_t count)
{
	struct efi_variable *new_var, *var = &entry->var;
	struct efivars *efivars = entry->efivars;
	efi_status_t status = EFI_NOT_FOUND;

	if (count != sizeof(struct efi_variable))
		return -EINVAL;

	new_var = (struct efi_variable *)buf;
	/*
	 * If only updating the variable data, then the name
	 * and guid should remain the same
	 */
	if (memcmp(new_var->VariableName, var->VariableName, sizeof(var->VariableName)) ||
		efi_guidcmp(new_var->VendorGuid, var->VendorGuid)) {
		printk(KERN_ERR "efivars: Cannot edit the wrong variable!\n");
		return -EINVAL;
	}
	    validate_var(new_var, new_var->Data, new_var->DataSize) == false) {
		printk(KERN_ERR "efivars: Malformed variable content\n");
		return -EINVAL;
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
		printk(KERN_WARNING "efivars: set_variable() failed: status=%lx\n",
			status);
		return -EIO;
	}
{
	struct efivar_entry *var = file->private_data;
	struct efivars *efivars;
	efi_status_t status;
	void *data;
	u32 attributes;
	struct inode *inode = file->f_mapping->host;
	unsigned long datasize = count - sizeof(attributes);
	unsigned long newdatasize, varsize;
	ssize_t bytes = 0;

	if (count < sizeof(attributes))
		return -EINVAL;

	if (copy_from_user(&attributes, userbuf, sizeof(attributes)))
		return -EFAULT;

	if (attributes & ~(EFI_VARIABLE_MASK))
		return -EINVAL;

	efivars = var->efivars;

	/*
	 * Ensure that the user can't allocate arbitrarily large
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
{
	struct efivar_entry *var = file->private_data;
	struct efivars *efivars;
	efi_status_t status;
	void *data;
	u32 attributes;
	struct inode *inode = file->f_mapping->host;
	unsigned long datasize = count - sizeof(attributes);
	unsigned long newdatasize, varsize;
	ssize_t bytes = 0;

	if (count < sizeof(attributes))
		return -EINVAL;

	if (copy_from_user(&attributes, userbuf, sizeof(attributes)))
		return -EFAULT;

	if (attributes & ~(EFI_VARIABLE_MASK))
		return -EINVAL;

	efivars = var->efivars;

	/*
	 * Ensure that the user can't allocate arbitrarily large
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
	if (validate_var(&var->var, data, datasize) == false) {
		bytes = -EINVAL;
		goto out;
	}

	/*
	 * The lock here protects the get_variable call, the conditional
	 * set_variable call, and removal of the variable from the efivars
	 * list (in the case of an authenticated delete).
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
	static const char dashes[GUID_LEN] = {
		[8] = 1, [13] = 1, [18] = 1, [23] = 1
	};
	const char *s = str + len - GUID_LEN;
	int i;

	/*
	 * We need a GUID, plus at least one letter for the variable name,
	 * plus the '-' separator
	 */
	if (len < GUID_LEN + 2)
		return false;

	/* GUID must be preceded by a '-' */
	if (*(s - 1) != '-')
		return false;

	/*
	 * Validate that 's' is of the correct format, e.g.
	 *
	 *	12345678-1234-1234-1234-123456789abc
	 */
	for (i = 0; i < GUID_LEN; i++) {
		if (dashes[i]) {
			if (*s++ != '-')
				return false;
		} else {
			if (!isxdigit(*s++))
				return false;
		}
	}

	return true;
}
static struct dentry_operations efivarfs_d_ops = {
	.d_compare = efivarfs_d_compare,
	.d_hash = efivarfs_d_hash,
	.d_delete = efivarfs_delete_dentry,
};

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
{
	struct inode *inode = NULL;
	struct dentry *root;
	struct efivar_entry *entry, *n;
	struct efivars *efivars = &__efivars;
	char *name;
	int err = -ENOMEM;

	efivarfs_sb = sb;

	sb->s_maxbytes          = MAX_LFS_FILESIZE;
	sb->s_blocksize         = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits    = PAGE_CACHE_SHIFT;
	sb->s_magic             = EFIVARFS_MAGIC;
	sb->s_op                = &efivarfs_ops;
	sb->s_d_op		= &efivarfs_d_ops;
	sb->s_time_gran         = 1;

	inode = efivarfs_get_inode(sb, NULL, S_IFDIR | 0755, 0);
	if (!inode)
		return -ENOMEM;
	inode->i_op = &efivarfs_dir_inode_operations;

	root = d_make_root(inode);
	sb->s_root = root;
	if (!root)
		return -ENOMEM;

	list_for_each_entry_safe(entry, n, &efivars->list, list) {
		struct dentry *dentry, *root = efivarfs_sb->s_root;
		unsigned long size = 0;
		int len, i;

		inode = NULL;

		len = utf16_strlen(entry->var.VariableName);

		/* name, plus '-', plus GUID, plus NUL*/
		name = kmalloc(len + 1 + GUID_LEN + 1, GFP_ATOMIC);
		if (!name)
			goto fail;

		for (i = 0; i < len; i++)
			name[i] = entry->var.VariableName[i] & 0xFF;

		name[len] = '-';

		efi_guid_unparse(&entry->var.VendorGuid, name + len + 1);

		name[len+GUID_LEN+1] = '\0';

		inode = efivarfs_get_inode(efivarfs_sb, root->d_inode,
					  S_IFREG | 0644, 0);
		if (!inode)
			goto fail_name;

		dentry = efivarfs_alloc_dentry(root, name);
		if (IS_ERR(dentry)) {
			err = PTR_ERR(dentry);
			goto fail_inode;
		}

		/* copied by the above to local storage in the dentry. */
		kfree(name);

		spin_lock_irq(&efivars->lock);
		efivars->ops->get_variable(entry->var.VariableName,
					   &entry->var.VendorGuid,
					   &entry->var.Attributes,
					   &size,
					   NULL);
		spin_unlock_irq(&efivars->lock);

		mutex_lock(&inode->i_mutex);
		inode->i_private = entry;
		i_size_write(inode, size + sizeof(entry->var.Attributes));
		mutex_unlock(&inode->i_mutex);
		d_add(dentry, inode);
	}
	list_for_each_entry_safe(entry, n, &efivars->list, list) {
		struct dentry *dentry, *root = efivarfs_sb->s_root;
		unsigned long size = 0;
		int len, i;

		inode = NULL;

		len = utf16_strlen(entry->var.VariableName);

		/* name, plus '-', plus GUID, plus NUL*/
		name = kmalloc(len + 1 + GUID_LEN + 1, GFP_ATOMIC);
		if (!name)
			goto fail;

		for (i = 0; i < len; i++)
			name[i] = entry->var.VariableName[i] & 0xFF;

		name[len] = '-';

		efi_guid_unparse(&entry->var.VendorGuid, name + len + 1);

		name[len+GUID_LEN+1] = '\0';

		inode = efivarfs_get_inode(efivarfs_sb, root->d_inode,
					  S_IFREG | 0644, 0);
		if (!inode)
			goto fail_name;

		dentry = efivarfs_alloc_dentry(root, name);
		if (IS_ERR(dentry)) {
			err = PTR_ERR(dentry);
			goto fail_inode;
		}
		if (IS_ERR(dentry)) {
			err = PTR_ERR(dentry);
			goto fail_inode;
		}

		/* copied by the above to local storage in the dentry. */
		kfree(name);

		spin_lock_irq(&efivars->lock);
		efivars->ops->get_variable(entry->var.VariableName,
					   &entry->var.VendorGuid,
					   &entry->var.Attributes,
					   &size,
					   NULL);
		spin_unlock_irq(&efivars->lock);

		mutex_lock(&inode->i_mutex);
		inode->i_private = entry;
		i_size_write(inode, size + sizeof(entry->var.Attributes));
		mutex_unlock(&inode->i_mutex);
		d_add(dentry, inode);
	}

	return 0;

fail_inode:
	iput(inode);
fail_name:
	kfree(name);
fail:
	return err;
}

static struct dentry *efivarfs_mount(struct file_system_type *fs_type,
				    int flags, const char *dev_name, void *data)
{
	return mount_single(fs_type, flags, data, efivarfs_fill_super);
}

static void efivarfs_kill_sb(struct super_block *sb)
{
	kill_litter_super(sb);
	efivarfs_sb = NULL;
}

static struct file_system_type efivarfs_type = {
	.name    = "efivarfs",
	.mount   = efivarfs_mount,
	.kill_sb = efivarfs_kill_sb,
};
MODULE_ALIAS_FS("efivarfs");

/*
 * Handle negative dentry.
 */
static struct dentry *efivarfs_lookup(struct inode *dir, struct dentry *dentry,
				      unsigned int flags)
{
	if (dentry->d_name.len > NAME_MAX)
		return ERR_PTR(-ENAMETOOLONG);
	d_add(dentry, NULL);
	return NULL;
}
static struct file_system_type efivarfs_type = {
	.name    = "efivarfs",
	.mount   = efivarfs_mount,
	.kill_sb = efivarfs_kill_sb,
};
MODULE_ALIAS_FS("efivarfs");

/*
 * Handle negative dentry.
 */
static struct dentry *efivarfs_lookup(struct inode *dir, struct dentry *dentry,
				      unsigned int flags)
{
	if (dentry->d_name.len > NAME_MAX)
		return ERR_PTR(-ENAMETOOLONG);
	d_add(dentry, NULL);
	return NULL;
}
{
	char name[DUMP_NAME_LEN];
	efi_char16_t efi_name[DUMP_NAME_LEN];
	efi_guid_t vendor = LINUX_EFI_CRASH_GUID;
	struct efivars *efivars = psi->data;
	int i, ret = 0;
	efi_status_t status = EFI_NOT_FOUND;
	unsigned long flags;

	if (pstore_cannot_block_path(reason)) {
		/*
		 * If the lock is taken by another cpu in non-blocking path,
		 * this driver returns without entering firmware to avoid
		 * hanging up.
		 */
		if (!spin_trylock_irqsave(&efivars->lock, flags))
			return -EBUSY;
	} else
		spin_lock_irqsave(&efivars->lock, flags);

	/*
	 * Check if there is a space enough to log.
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

	sprintf(name, "dump-type%u-%u-%d-%lu", type, part, count,
		get_seconds());

	for (i = 0; i < DUMP_NAME_LEN; i++)
		efi_name[i] = name[i];

	efivars->ops->set_variable(efi_name, &vendor, PSTORE_EFI_ATTRIBUTES,
				   size, psi->buf);

	spin_unlock_irqrestore(&efivars->lock, flags);

	if (reason == KMSG_DUMP_OOPS)
		schedule_work(&efivar_work);

	*id = part;
	return ret;
};

static int efi_pstore_erase(enum pstore_type_id type, u64 id, int count,
			    struct timespec time, struct pstore_info *psi)
{
	char name[DUMP_NAME_LEN];
	efi_char16_t efi_name[DUMP_NAME_LEN];
	char name_old[DUMP_NAME_LEN];
	efi_char16_t efi_name_old[DUMP_NAME_LEN];
	efi_guid_t vendor = LINUX_EFI_CRASH_GUID;
	struct efivars *efivars = psi->data;
	struct efivar_entry *entry, *found = NULL;
	int i;

	sprintf(name, "dump-type%u-%u-%d-%lu", type, (unsigned int)id, count,
		time.tv_sec);

	spin_lock_irq(&efivars->lock);

	for (i = 0; i < DUMP_NAME_LEN; i++)
		efi_name[i] = name[i];

	/*
	 * Clean up an entry with the same name
	 */

	list_for_each_entry(entry, &efivars->list, list) {
		get_var_data_locked(efivars, &entry->var);

		if (efi_guidcmp(entry->var.VendorGuid, vendor))
			continue;
		if (utf16_strncmp(entry->var.VariableName, efi_name,
				  utf16_strlen(efi_name))) {
			/*
			 * Check if an old format,
			 * which doesn't support holding
			 * multiple logs, remains.
			 */
			sprintf(name_old, "dump-type%u-%u-%lu", type,
				(unsigned int)id, time.tv_sec);

			for (i = 0; i < DUMP_NAME_LEN; i++)
				efi_name_old[i] = name_old[i];

			if (utf16_strncmp(entry->var.VariableName, efi_name_old,
					  utf16_strlen(efi_name_old)))
				continue;
		}

		/* found */
		found = entry;
		efivars->ops->set_variable(entry->var.VariableName,
					   &entry->var.VendorGuid,
					   PSTORE_EFI_ATTRIBUTES,
					   0, NULL);
		break;
	}

	if (found)
		list_del(&found->list);

	spin_unlock_irq(&efivars->lock);

	if (found)
		efivar_unregister(found);

	return 0;
}
#else
static int efi_pstore_open(struct pstore_info *psi)
{
	return 0;
}

static int efi_pstore_close(struct pstore_info *psi)
{
	return 0;
}

static ssize_t efi_pstore_read(u64 *id, enum pstore_type_id *type, int *count,
			       struct timespec *timespec,
			       char **buf, struct pstore_info *psi)
{
	return -1;
}

static int efi_pstore_write(enum pstore_type_id type,
		enum kmsg_dump_reason reason, u64 *id,
		unsigned int part, int count, size_t size,
		struct pstore_info *psi)
{
	return 0;
}

static int efi_pstore_erase(enum pstore_type_id type, u64 id, int count,
			    struct timespec time, struct pstore_info *psi)
{
	return 0;
}
#endif

static struct pstore_info efi_pstore_info = {
	.owner		= THIS_MODULE,
	.name		= "efi",
	.open		= efi_pstore_open,
	.close		= efi_pstore_close,
	.read		= efi_pstore_read,
	.write		= efi_pstore_write,
	.erase		= efi_pstore_erase,
};

static ssize_t efivar_create(struct file *filp, struct kobject *kobj,
			     struct bin_attribute *bin_attr,
			     char *buf, loff_t pos, size_t count)
{
	struct efi_variable *new_var = (struct efi_variable *)buf;
	struct efivars *efivars = bin_attr->private;
	struct efivar_entry *search_efivar, *n;
	unsigned long strsize1, strsize2;
	efi_status_t status = EFI_NOT_FOUND;
	int found = 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	if ((new_var->Attributes & ~EFI_VARIABLE_MASK) != 0 ||
	    validate_var(new_var, new_var->Data, new_var->DataSize) == false) {
		printk(KERN_ERR "efivars: Malformed variable content\n");
		return -EINVAL;
	}

	spin_lock_irq(&efivars->lock);

	/*
	 * Does this variable already exist?
	 */
	list_for_each_entry_safe(search_efivar, n, &efivars->list, list) {
		strsize1 = utf16_strsize(search_efivar->var.VariableName, 1024);
		strsize2 = utf16_strsize(new_var->VariableName, 1024);
		if (strsize1 == strsize2 &&
			!memcmp(&(search_efivar->var.VariableName),
				new_var->VariableName, strsize1) &&
			!efi_guidcmp(search_efivar->var.VendorGuid,
				new_var->VendorGuid)) {
			found = 1;
			break;
		}
	}
	if (found) {
		spin_unlock_irq(&efivars->lock);
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
					    new_var->DataSize,
					    new_var->Data);

	if (status != EFI_SUCCESS) {
		printk(KERN_WARNING "efivars: set_variable() failed: status=%lx\n",
			status);
		spin_unlock_irq(&efivars->lock);
		return -EIO;
	}
	spin_unlock_irq(&efivars->lock);

	/* Create the entry in sysfs.  Locking is not required here */
	status = efivar_create_sysfs_entry(efivars,
					   utf16_strsize(new_var->VariableName,
							 1024),
					   new_var->VariableName,
					   &new_var->VendorGuid);
	if (status) {
		printk(KERN_WARNING "efivars: variable created, but sysfs entry wasn't.\n");
	}
	return count;
}

static ssize_t efivar_delete(struct file *filp, struct kobject *kobj,
			     struct bin_attribute *bin_attr,
			     char *buf, loff_t pos, size_t count)
{
	struct efi_variable *del_var = (struct efi_variable *)buf;
	struct efivars *efivars = bin_attr->private;
	struct efivar_entry *search_efivar, *n;
	unsigned long strsize1, strsize2;
	efi_status_t status = EFI_NOT_FOUND;
	int found = 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	spin_lock_irq(&efivars->lock);

	/*
	 * Does this variable already exist?
	 */
	list_for_each_entry_safe(search_efivar, n, &efivars->list, list) {
		strsize1 = utf16_strsize(search_efivar->var.VariableName, 1024);
		strsize2 = utf16_strsize(del_var->VariableName, 1024);
		if (strsize1 == strsize2 &&
			!memcmp(&(search_efivar->var.VariableName),
				del_var->VariableName, strsize1) &&
			!efi_guidcmp(search_efivar->var.VendorGuid,
				del_var->VendorGuid)) {
			found = 1;
			break;
		}
	}
	if (!found) {
		spin_unlock_irq(&efivars->lock);
		return -EINVAL;
	}
	/* force the Attributes/DataSize to 0 to ensure deletion */
	del_var->Attributes = 0;
	del_var->DataSize = 0;

	status = efivars->ops->set_variable(del_var->VariableName,
					    &del_var->VendorGuid,
					    del_var->Attributes,
					    del_var->DataSize,
					    del_var->Data);

	if (status != EFI_SUCCESS) {
		printk(KERN_WARNING "efivars: set_variable() failed: status=%lx\n",
			status);
		spin_unlock_irq(&efivars->lock);
		return -EIO;
	}
	list_del(&search_efivar->list);
	/* We need to release this lock before unregistering. */
	spin_unlock_irq(&efivars->lock);
	efivar_unregister(search_efivar);

	/* It's dead Jim.... */
	return count;
}

static bool variable_is_present(efi_char16_t *variable_name, efi_guid_t *vendor)
{
	struct efivar_entry *entry, *n;
	struct efivars *efivars = &__efivars;
	unsigned long strsize1, strsize2;
	bool found = false;

	strsize1 = utf16_strsize(variable_name, 1024);
	list_for_each_entry_safe(entry, n, &efivars->list, list) {
		strsize2 = utf16_strsize(entry->var.VariableName, 1024);
		if (strsize1 == strsize2 &&
			!memcmp(variable_name, &(entry->var.VariableName),
				strsize2) &&
			!efi_guidcmp(entry->var.VendorGuid,
				*vendor)) {
			found = true;
			break;
		}
	}
	return found;
}

static void efivar_update_sysfs_entries(struct work_struct *work)
{
	struct efivars *efivars = &__efivars;
	efi_guid_t vendor;
	efi_char16_t *variable_name;
	unsigned long variable_name_size = 1024;
	efi_status_t status = EFI_NOT_FOUND;
	bool found;

	/* Add new sysfs entries */
	while (1) {
		variable_name = kzalloc(variable_name_size, GFP_KERNEL);
		if (!variable_name) {
			pr_err("efivars: Memory allocation failed.\n");
			return;
		}

		spin_lock_irq(&efivars->lock);
		found = false;
		while (1) {
			variable_name_size = 1024;
			status = efivars->ops->get_next_variable(
							&variable_name_size,
							variable_name,
							&vendor);
			if (status != EFI_SUCCESS) {
				break;
			} else {
				if (!variable_is_present(variable_name,
				    &vendor)) {
					found = true;
					break;
				}
			}
		}
		spin_unlock_irq(&efivars->lock);

		if (!found) {
			kfree(variable_name);
			break;
		} else
			efivar_create_sysfs_entry(efivars,
						  variable_name_size,
						  variable_name, &vendor);
	}
}

/*
 * Let's not leave out systab information that snuck into
 * the efivars driver
 */
static ssize_t systab_show(struct kobject *kobj,
			   struct kobj_attribute *attr, char *buf)
{
	char *str = buf;

	if (!kobj || !buf)
		return -EINVAL;

	if (efi.mps != EFI_INVALID_TABLE_ADDR)
		str += sprintf(str, "MPS=0x%lx\n", efi.mps);
	if (efi.acpi20 != EFI_INVALID_TABLE_ADDR)
		str += sprintf(str, "ACPI20=0x%lx\n", efi.acpi20);
	if (efi.acpi != EFI_INVALID_TABLE_ADDR)
		str += sprintf(str, "ACPI=0x%lx\n", efi.acpi);
	if (efi.smbios != EFI_INVALID_TABLE_ADDR)
		str += sprintf(str, "SMBIOS=0x%lx\n", efi.smbios);
	if (efi.hcdp != EFI_INVALID_TABLE_ADDR)
		str += sprintf(str, "HCDP=0x%lx\n", efi.hcdp);
	if (efi.boot_info != EFI_INVALID_TABLE_ADDR)
		str += sprintf(str, "BOOTINFO=0x%lx\n", efi.boot_info);
	if (efi.uga != EFI_INVALID_TABLE_ADDR)
		str += sprintf(str, "UGA=0x%lx\n", efi.uga);

	return str - buf;
}

static struct kobj_attribute efi_attr_systab =
			__ATTR(systab, 0400, systab_show, NULL);

static struct attribute *efi_subsys_attrs[] = {
	&efi_attr_systab.attr,
	NULL,	/* maybe more in the future? */
};

static struct attribute_group efi_subsys_attr_group = {
	.attrs = efi_subsys_attrs,
};

static struct kobject *efi_kobj;

/*
 * efivar_create_sysfs_entry()
 * Requires:
 *    variable_name_size = number of bytes required to hold
 *                         variable_name (not counting the NULL
 *                         character at the end.
 *    efivars->lock is not held on entry or exit.
 * Returns 1 on failure, 0 on success
 */
static int
efivar_create_sysfs_entry(struct efivars *efivars,
			  unsigned long variable_name_size,
			  efi_char16_t *variable_name,
			  efi_guid_t *vendor_guid)
{
	int i, short_name_size;
	char *short_name;
	struct efivar_entry *new_efivar;

	/*
	 * Length of the variable bytes in ASCII, plus the '-' separator,
	 * plus the GUID, plus trailing NUL
	 */
	short_name_size = variable_name_size / sizeof(efi_char16_t)
				+ 1 + GUID_LEN + 1;

	short_name = kzalloc(short_name_size, GFP_KERNEL);
	new_efivar = kzalloc(sizeof(struct efivar_entry), GFP_KERNEL);

	if (!short_name || !new_efivar)  {
		kfree(short_name);
		kfree(new_efivar);
		return 1;
	}

	new_efivar->efivars = efivars;
	memcpy(new_efivar->var.VariableName, variable_name,
		variable_name_size);
	memcpy(&(new_efivar->var.VendorGuid), vendor_guid, sizeof(efi_guid_t));

	/* Convert Unicode to normal chars (assume top bits are 0),
	   ala UTF-8 */
	for (i=0; i < (int)(variable_name_size / sizeof(efi_char16_t)); i++) {
		short_name[i] = variable_name[i] & 0xFF;
	}
	/* This is ugly, but necessary to separate one vendor's
	   private variables from another's.         */

	*(short_name + strlen(short_name)) = '-';
	efi_guid_unparse(vendor_guid, short_name + strlen(short_name));

	new_efivar->kobj.kset = efivars->kset;
	i = kobject_init_and_add(&new_efivar->kobj, &efivar_ktype, NULL,
				 "%s", short_name);
	if (i) {
		kfree(short_name);
		kfree(new_efivar);
		return 1;
	}

	kobject_uevent(&new_efivar->kobj, KOBJ_ADD);
	kfree(short_name);
	short_name = NULL;

	spin_lock_irq(&efivars->lock);
	list_add(&new_efivar->list, &efivars->list);
	spin_unlock_irq(&efivars->lock);

	return 0;
}

static int
create_efivars_bin_attributes(struct efivars *efivars)
{
	struct bin_attribute *attr;
	int error;

	/* new_var */
	attr = kzalloc(sizeof(*attr), GFP_KERNEL);
	if (!attr)
		return -ENOMEM;

	attr->attr.name = "new_var";
	attr->attr.mode = 0200;
	attr->write = efivar_create;
	attr->private = efivars;
	efivars->new_var = attr;

	/* del_var */
	attr = kzalloc(sizeof(*attr), GFP_KERNEL);
	if (!attr) {
		error = -ENOMEM;
		goto out_free;
	}
	attr->attr.name = "del_var";
	attr->attr.mode = 0200;
	attr->write = efivar_delete;
	attr->private = efivars;
	efivars->del_var = attr;

	sysfs_bin_attr_init(efivars->new_var);
	sysfs_bin_attr_init(efivars->del_var);

	/* Register */
	error = sysfs_create_bin_file(&efivars->kset->kobj,
				      efivars->new_var);
	if (error) {
		printk(KERN_ERR "efivars: unable to create new_var sysfs file"
			" due to error %d\n", error);
		goto out_free;
	}
	error = sysfs_create_bin_file(&efivars->kset->kobj,
				      efivars->del_var);
	if (error) {
		printk(KERN_ERR "efivars: unable to create del_var sysfs file"
			" due to error %d\n", error);
		sysfs_remove_bin_file(&efivars->kset->kobj,
				      efivars->new_var);
		goto out_free;
	}

	return 0;
out_free:
	kfree(efivars->del_var);
	efivars->del_var = NULL;
	kfree(efivars->new_var);
	efivars->new_var = NULL;
	return error;
}

void unregister_efivars(struct efivars *efivars)
{
	struct efivar_entry *entry, *n;

	list_for_each_entry_safe(entry, n, &efivars->list, list) {
		spin_lock_irq(&efivars->lock);
		list_del(&entry->list);
		spin_unlock_irq(&efivars->lock);
		efivar_unregister(entry);
	}
	if (efivars->new_var)
		sysfs_remove_bin_file(&efivars->kset->kobj, efivars->new_var);
	if (efivars->del_var)
		sysfs_remove_bin_file(&efivars->kset->kobj, efivars->del_var);
	kfree(efivars->new_var);
	kfree(efivars->del_var);
	kobject_put(efivars->kobject);
	kset_unregister(efivars->kset);
}
EXPORT_SYMBOL_GPL(unregister_efivars);

int register_efivars(struct efivars *efivars,
		     const struct efivar_operations *ops,
		     struct kobject *parent_kobj)
{
	efi_status_t status = EFI_NOT_FOUND;
	efi_guid_t vendor_guid;
	efi_char16_t *variable_name;
	unsigned long variable_name_size = 1024;
	int error = 0;

	variable_name = kzalloc(variable_name_size, GFP_KERNEL);
	if (!variable_name) {
		printk(KERN_ERR "efivars: Memory allocation failed.\n");
		return -ENOMEM;
	}

	spin_lock_init(&efivars->lock);
	INIT_LIST_HEAD(&efivars->list);
	efivars->ops = ops;

	efivars->kset = kset_create_and_add("vars", NULL, parent_kobj);
	if (!efivars->kset) {
		printk(KERN_ERR "efivars: Subsystem registration failed.\n");
		error = -ENOMEM;
		goto out;
	}

	efivars->kobject = kobject_create_and_add("efivars", parent_kobj);
	if (!efivars->kobject) {
		pr_err("efivars: Subsystem registration failed.\n");
		error = -ENOMEM;
		kset_unregister(efivars->kset);
		goto out;
	}

	/*
	 * Per EFI spec, the maximum storage allocated for both
	 * the variable name and variable data is 1024 bytes.
	 */

	do {
		variable_name_size = 1024;

		status = ops->get_next_variable(&variable_name_size,
						variable_name,
						&vendor_guid);
		switch (status) {
		case EFI_SUCCESS:
			efivar_create_sysfs_entry(efivars,
						  variable_name_size,
						  variable_name,
						  &vendor_guid);
			break;
		case EFI_NOT_FOUND:
			break;
		default:
			printk(KERN_WARNING "efivars: get_next_variable: status=%lx\n",
				status);
			status = EFI_NOT_FOUND;
			break;
		}
	} while (status != EFI_NOT_FOUND);

	error = create_efivars_bin_attributes(efivars);
	if (error)
		unregister_efivars(efivars);

	efivars->efi_pstore_info = efi_pstore_info;

	efivars->efi_pstore_info.buf = kmalloc(4096, GFP_KERNEL);
	if (efivars->efi_pstore_info.buf) {
		efivars->efi_pstore_info.bufsize = 1024;
		efivars->efi_pstore_info.data = efivars;
		spin_lock_init(&efivars->efi_pstore_info.buf_lock);
		pstore_register(&efivars->efi_pstore_info);
	}

	register_filesystem(&efivarfs_type);

out:
	kfree(variable_name);

	return error;
}
EXPORT_SYMBOL_GPL(register_efivars);

/*
 * For now we register the efi subsystem with the firmware subsystem
 * and the vars subsystem with the efi subsystem.  In the future, it
 * might make sense to split off the efi subsystem into its own
 * driver, but for now only efivars will register with it, so just
 * include it here.
 */

static int __init
efivars_init(void)
{
	int error = 0;

	printk(KERN_INFO "EFI Variables Facility v%s %s\n", EFIVARS_VERSION,
	       EFIVARS_DATE);

	if (!efi_enabled(EFI_RUNTIME_SERVICES))
		return 0;

	/* For now we'll register the efi directory at /sys/firmware/efi */
	efi_kobj = kobject_create_and_add("efi", firmware_kobj);
	if (!efi_kobj) {
		printk(KERN_ERR "efivars: Firmware registration failed.\n");
		return -ENOMEM;
	}

	ops.get_variable = efi.get_variable;
	ops.set_variable = efi.set_variable;
	ops.get_next_variable = efi.get_next_variable;
	ops.query_variable_info = efi.query_variable_info;

	error = register_efivars(&__efivars, &ops, efi_kobj);
	if (error)
		goto err_put;

	/* Don't forget the systab entry */
	error = sysfs_create_group(efi_kobj, &efi_subsys_attr_group);
	if (error) {
		printk(KERN_ERR
		       "efivars: Sysfs attribute export failed with error %d.\n",
		       error);
		goto err_unregister;
	}

	return 0;

err_unregister:
	unregister_efivars(&__efivars);
err_put:
	kobject_put(efi_kobj);
	return error;
}

static void __exit
efivars_exit(void)
{
	cancel_work_sync(&efivar_work);

	if (efi_enabled(EFI_RUNTIME_SERVICES)) {
		unregister_efivars(&__efivars);
		kobject_put(efi_kobj);
	}
}

module_init(efivars_init);
module_exit(efivars_exit);

	if (pstore_cannot_block_path(reason)) {
		/*
		 * If the lock is taken by another cpu in non-blocking path,
		 * this driver returns without entering firmware to avoid
		 * hanging up.
		 */
		if (!spin_trylock_irqsave(&efivars->lock, flags))
			return -EBUSY;
	} else
		spin_lock_irqsave(&efivars->lock, flags);

	/*
	 * Check if there is a space enough to log.
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
	if (found) {
		spin_unlock_irq(&efivars->lock);
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
					    new_var->DataSize,
					    new_var->Data);

	if (status != EFI_SUCCESS) {
		printk(KERN_WARNING "efivars: set_variable() failed: status=%lx\n",
			status);
		spin_unlock_irq(&efivars->lock);
		return -EIO;
	}
	spin_unlock_irq(&efivars->lock);

	/* Create the entry in sysfs.  Locking is not required here */
	status = efivar_create_sysfs_entry(efivars,
					   utf16_strsize(new_var->VariableName,
							 1024),
					   new_var->VariableName,
					   &new_var->VendorGuid);
	if (status) {
		printk(KERN_WARNING "efivars: variable created, but sysfs entry wasn't.\n");
	}
	return count;
}

static ssize_t efivar_delete(struct file *filp, struct kobject *kobj,
			     struct bin_attribute *bin_attr,
			     char *buf, loff_t pos, size_t count)
{
	struct efi_variable *del_var = (struct efi_variable *)buf;
	struct efivars *efivars = bin_attr->private;
	struct efivar_entry *search_efivar, *n;
	unsigned long strsize1, strsize2;
	efi_status_t status = EFI_NOT_FOUND;
	int found = 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	spin_lock_irq(&efivars->lock);

	/*
	 * Does this variable already exist?
	 */
	list_for_each_entry_safe(search_efivar, n, &efivars->list, list) {
		strsize1 = utf16_strsize(search_efivar->var.VariableName, 1024);
		strsize2 = utf16_strsize(del_var->VariableName, 1024);
		if (strsize1 == strsize2 &&
			!memcmp(&(search_efivar->var.VariableName),
				del_var->VariableName, strsize1) &&
			!efi_guidcmp(search_efivar->var.VendorGuid,
				del_var->VendorGuid)) {
			found = 1;
			break;
		}