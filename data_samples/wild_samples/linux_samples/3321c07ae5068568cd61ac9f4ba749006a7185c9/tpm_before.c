{
	struct tpm_chip *chip = file->private_data;
	ssize_t ret_size;

	del_singleshot_timer_sync(&chip->user_read_timer);
	flush_work_sync(&chip->work);
	ret_size = atomic_read(&chip->data_pending);
	atomic_set(&chip->data_pending, 0);
	if (ret_size > 0) {	/* relay data */
		if (size < ret_size)
			ret_size = size;

		mutex_lock(&chip->buffer_mutex);
		if (copy_to_user(buf, chip->data_buffer, ret_size))
			ret_size = -EFAULT;
		mutex_unlock(&chip->buffer_mutex);
	}

	return ret_size;
}
EXPORT_SYMBOL_GPL(tpm_read);

void tpm_remove_hardware(struct device *dev)
{
	struct tpm_chip *chip = dev_get_drvdata(dev);

	if (chip == NULL) {
		dev_err(dev, "No device data found\n");
		return;
	}

	spin_lock(&driver_lock);
	list_del_rcu(&chip->list);
	spin_unlock(&driver_lock);
	synchronize_rcu();

	misc_deregister(&chip->vendor.miscdev);
	sysfs_remove_group(&dev->kobj, chip->vendor.attr_group);
	tpm_bios_log_teardown(chip->bios_dir);

	/* write it this way to be explicit (chip->dev == dev) */
	put_device(chip->dev);
}
EXPORT_SYMBOL_GPL(tpm_remove_hardware);

#define TPM_ORD_SAVESTATE cpu_to_be32(152)
#define SAVESTATE_RESULT_SIZE 10

static struct tpm_input_header savestate_header = {
	.tag = TPM_TAG_RQU_COMMAND,
	.length = cpu_to_be32(10),
	.ordinal = TPM_ORD_SAVESTATE
};

/*
 * We are about to suspend. Save the TPM state
 * so that it can be restored.
 */
int tpm_pm_suspend(struct device *dev, pm_message_t pm_state)
{
	struct tpm_chip *chip = dev_get_drvdata(dev);
	struct tpm_cmd_t cmd;
	int rc;

	u8 dummy_hash[TPM_DIGEST_SIZE] = { 0 };

	if (chip == NULL)
		return -ENODEV;

	/* for buggy tpm, flush pcrs with extend to selected dummy */
	if (tpm_suspend_pcr) {
		cmd.header.in = pcrextend_header;
		cmd.params.pcrextend_in.pcr_idx = cpu_to_be32(tpm_suspend_pcr);
		memcpy(cmd.params.pcrextend_in.hash, dummy_hash,
		       TPM_DIGEST_SIZE);
		rc = transmit_cmd(chip, &cmd, EXTEND_PCR_RESULT_SIZE,
				  "extending dummy pcr before suspend");
	}
{
	struct tpm_chip *chip = file->private_data;
	ssize_t ret_size;

	del_singleshot_timer_sync(&chip->user_read_timer);
	flush_work_sync(&chip->work);
	ret_size = atomic_read(&chip->data_pending);
	atomic_set(&chip->data_pending, 0);
	if (ret_size > 0) {	/* relay data */
		if (size < ret_size)
			ret_size = size;

		mutex_lock(&chip->buffer_mutex);
		if (copy_to_user(buf, chip->data_buffer, ret_size))
			ret_size = -EFAULT;
		mutex_unlock(&chip->buffer_mutex);
	}

	return ret_size;
}
EXPORT_SYMBOL_GPL(tpm_read);

void tpm_remove_hardware(struct device *dev)
{
	struct tpm_chip *chip = dev_get_drvdata(dev);

	if (chip == NULL) {
		dev_err(dev, "No device data found\n");
		return;
	}

	spin_lock(&driver_lock);
	list_del_rcu(&chip->list);
	spin_unlock(&driver_lock);
	synchronize_rcu();

	misc_deregister(&chip->vendor.miscdev);
	sysfs_remove_group(&dev->kobj, chip->vendor.attr_group);
	tpm_bios_log_teardown(chip->bios_dir);

	/* write it this way to be explicit (chip->dev == dev) */
	put_device(chip->dev);
}
EXPORT_SYMBOL_GPL(tpm_remove_hardware);

#define TPM_ORD_SAVESTATE cpu_to_be32(152)
#define SAVESTATE_RESULT_SIZE 10

static struct tpm_input_header savestate_header = {
	.tag = TPM_TAG_RQU_COMMAND,
	.length = cpu_to_be32(10),
	.ordinal = TPM_ORD_SAVESTATE
};

/*
 * We are about to suspend. Save the TPM state
 * so that it can be restored.
 */
int tpm_pm_suspend(struct device *dev, pm_message_t pm_state)
{
	struct tpm_chip *chip = dev_get_drvdata(dev);
	struct tpm_cmd_t cmd;
	int rc;

	u8 dummy_hash[TPM_DIGEST_SIZE] = { 0 };

	if (chip == NULL)
		return -ENODEV;

	/* for buggy tpm, flush pcrs with extend to selected dummy */
	if (tpm_suspend_pcr) {
		cmd.header.in = pcrextend_header;
		cmd.params.pcrextend_in.pcr_idx = cpu_to_be32(tpm_suspend_pcr);
		memcpy(cmd.params.pcrextend_in.hash, dummy_hash,
		       TPM_DIGEST_SIZE);
		rc = transmit_cmd(chip, &cmd, EXTEND_PCR_RESULT_SIZE,
				  "extending dummy pcr before suspend");
	}