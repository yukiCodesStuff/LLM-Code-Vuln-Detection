{
	struct hci_dev *hdev = data;

	if (val == 0 || val > hdev->conn_info_max_age)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->conn_info_min_age = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_info_min_age_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->conn_info_min_age;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_info_min_age_fops, conn_info_min_age_get,
			  conn_info_min_age_set, "%llu\n");

static int conn_info_max_age_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val == 0 || val < hdev->conn_info_min_age)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->conn_info_max_age = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_info_max_age_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->conn_info_max_age;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_info_max_age_fops, conn_info_max_age_get,
			  conn_info_max_age_set, "%llu\n");

static ssize_t use_debug_keys_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct hci_dev *hdev = file->private_data;
	char buf[3];

	buf[0] = hci_dev_test_flag(hdev, HCI_USE_DEBUG_KEYS) ? 'Y' : 'N';
	buf[1] = '\n';
	buf[2] = '\0';
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static const struct file_operations use_debug_keys_fops = {
	.open		= simple_open,
	.read		= use_debug_keys_read,
	.llseek		= default_llseek,
};

static ssize_t sc_only_mode_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct hci_dev *hdev = file->private_data;
	char buf[3];

	buf[0] = hci_dev_test_flag(hdev, HCI_SC_ONLY) ? 'Y' : 'N';
	buf[1] = '\n';
	buf[2] = '\0';
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}
{
	struct hci_dev *hdev = data;

	if (val == 0 || val < hdev->conn_info_min_age)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->conn_info_max_age = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_info_max_age_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->conn_info_max_age;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_info_max_age_fops, conn_info_max_age_get,
			  conn_info_max_age_set, "%llu\n");

static ssize_t use_debug_keys_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct hci_dev *hdev = file->private_data;
	char buf[3];

	buf[0] = hci_dev_test_flag(hdev, HCI_USE_DEBUG_KEYS) ? 'Y' : 'N';
	buf[1] = '\n';
	buf[2] = '\0';
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static const struct file_operations use_debug_keys_fops = {
	.open		= simple_open,
	.read		= use_debug_keys_read,
	.llseek		= default_llseek,
};

static ssize_t sc_only_mode_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct hci_dev *hdev = file->private_data;
	char buf[3];

	buf[0] = hci_dev_test_flag(hdev, HCI_SC_ONLY) ? 'Y' : 'N';
	buf[1] = '\n';
	buf[2] = '\0';
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}
{
	struct hci_dev *hdev = data;

	if (val == 0 || val % 2 || val > hdev->sniff_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->sniff_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int sniff_min_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->sniff_min_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(sniff_min_interval_fops, sniff_min_interval_get,
			  sniff_min_interval_set, "%llu\n");

static int sniff_max_interval_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val == 0 || val % 2 || val < hdev->sniff_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->sniff_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int sniff_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->sniff_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(sniff_max_interval_fops, sniff_max_interval_get,
			  sniff_max_interval_set, "%llu\n");

void hci_debugfs_create_bredr(struct hci_dev *hdev)
{
	debugfs_create_file("inquiry_cache", 0444, hdev->debugfs, hdev,
			    &inquiry_cache_fops);
	debugfs_create_file("link_keys", 0400, hdev->debugfs, hdev,
			    &link_keys_fops);
	debugfs_create_file("dev_class", 0444, hdev->debugfs, hdev,
			    &dev_class_fops);
	debugfs_create_file("voice_setting", 0444, hdev->debugfs, hdev,
			    &voice_setting_fops);

	/* If the controller does not support BR/EDR Secure Connections
	 * feature, then the BR/EDR SMP channel shall not be present.
	 *
	 * To test this with Bluetooth 4.0 controllers, create a debugfs
	 * switch that allows forcing BR/EDR SMP support and accepting
	 * cross-transport pairing on non-AES encrypted connections.
	 */
	if (!lmp_sc_capable(hdev))
		debugfs_create_file("force_bredr_smp", 0644, hdev->debugfs,
				    hdev, &force_bredr_smp_fops);

	if (lmp_ssp_capable(hdev)) {
		debugfs_create_file("ssp_debug_mode", 0444, hdev->debugfs,
				    hdev, &ssp_debug_mode_fops);
		debugfs_create_file("min_encrypt_key_size", 0644, hdev->debugfs,
				    hdev, &min_encrypt_key_size_fops);
		debugfs_create_file("auto_accept_delay", 0644, hdev->debugfs,
				    hdev, &auto_accept_delay_fops);
	}
{
	struct hci_dev *hdev = data;

	if (val == 0 || val % 2 || val < hdev->sniff_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->sniff_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int sniff_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->sniff_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(sniff_max_interval_fops, sniff_max_interval_get,
			  sniff_max_interval_set, "%llu\n");

void hci_debugfs_create_bredr(struct hci_dev *hdev)
{
	debugfs_create_file("inquiry_cache", 0444, hdev->debugfs, hdev,
			    &inquiry_cache_fops);
	debugfs_create_file("link_keys", 0400, hdev->debugfs, hdev,
			    &link_keys_fops);
	debugfs_create_file("dev_class", 0444, hdev->debugfs, hdev,
			    &dev_class_fops);
	debugfs_create_file("voice_setting", 0444, hdev->debugfs, hdev,
			    &voice_setting_fops);

	/* If the controller does not support BR/EDR Secure Connections
	 * feature, then the BR/EDR SMP channel shall not be present.
	 *
	 * To test this with Bluetooth 4.0 controllers, create a debugfs
	 * switch that allows forcing BR/EDR SMP support and accepting
	 * cross-transport pairing on non-AES encrypted connections.
	 */
	if (!lmp_sc_capable(hdev))
		debugfs_create_file("force_bredr_smp", 0644, hdev->debugfs,
				    hdev, &force_bredr_smp_fops);

	if (lmp_ssp_capable(hdev)) {
		debugfs_create_file("ssp_debug_mode", 0444, hdev->debugfs,
				    hdev, &ssp_debug_mode_fops);
		debugfs_create_file("min_encrypt_key_size", 0644, hdev->debugfs,
				    hdev, &min_encrypt_key_size_fops);
		debugfs_create_file("auto_accept_delay", 0644, hdev->debugfs,
				    hdev, &auto_accept_delay_fops);
	}
{
	struct hci_dev *hdev = data;

	if (val < 0x0006 || val > 0x0c80 || val > hdev->le_conn_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_conn_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_min_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_conn_min_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_min_interval_fops, conn_min_interval_get,
			  conn_min_interval_set, "%llu\n");

static int conn_max_interval_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x0006 || val > 0x0c80 || val < hdev->le_conn_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_conn_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_conn_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_max_interval_fops, conn_max_interval_get,
			  conn_max_interval_set, "%llu\n");

static int conn_latency_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val > 0x01f3)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_conn_latency = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_latency_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_conn_latency;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_latency_fops, conn_latency_get,
			  conn_latency_set, "%llu\n");

static int supervision_timeout_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x000a || val > 0x0c80)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_supv_timeout = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int supervision_timeout_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_supv_timeout;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(supervision_timeout_fops, supervision_timeout_get,
			  supervision_timeout_set, "%llu\n");

static int adv_channel_map_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x01 || val > 0x07)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_channel_map = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_channel_map_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_channel_map;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_channel_map_fops, adv_channel_map_get,
			  adv_channel_map_set, "%llu\n");

static int adv_min_interval_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val > hdev->le_adv_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_min_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_min_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_min_interval_fops, adv_min_interval_get,
			  adv_min_interval_set, "%llu\n");

static int adv_max_interval_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val < hdev->le_adv_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_max_interval_fops, adv_max_interval_get,
			  adv_max_interval_set, "%llu\n");

static int min_key_size_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val > hdev->le_max_key_size || val < SMP_MIN_ENC_KEY_SIZE) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}
{
	struct hci_dev *hdev = data;

	if (val < 0x0006 || val > 0x0c80 || val < hdev->le_conn_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_conn_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_conn_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_max_interval_fops, conn_max_interval_get,
			  conn_max_interval_set, "%llu\n");

static int conn_latency_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val > 0x01f3)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_conn_latency = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int conn_latency_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_conn_latency;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(conn_latency_fops, conn_latency_get,
			  conn_latency_set, "%llu\n");

static int supervision_timeout_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x000a || val > 0x0c80)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_supv_timeout = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int supervision_timeout_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_supv_timeout;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(supervision_timeout_fops, supervision_timeout_get,
			  supervision_timeout_set, "%llu\n");

static int adv_channel_map_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x01 || val > 0x07)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_channel_map = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_channel_map_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_channel_map;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_channel_map_fops, adv_channel_map_get,
			  adv_channel_map_set, "%llu\n");

static int adv_min_interval_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val > hdev->le_adv_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_min_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_min_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_min_interval_fops, adv_min_interval_get,
			  adv_min_interval_set, "%llu\n");

static int adv_max_interval_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val < hdev->le_adv_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_max_interval_fops, adv_max_interval_get,
			  adv_max_interval_set, "%llu\n");

static int min_key_size_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val > hdev->le_max_key_size || val < SMP_MIN_ENC_KEY_SIZE) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val > hdev->le_adv_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_min_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_min_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_min_interval_fops, adv_min_interval_get,
			  adv_min_interval_set, "%llu\n");

static int adv_max_interval_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val < hdev->le_adv_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_max_interval_fops, adv_max_interval_get,
			  adv_max_interval_set, "%llu\n");

static int min_key_size_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val > hdev->le_max_key_size || val < SMP_MIN_ENC_KEY_SIZE) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val < hdev->le_adv_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
}

static int adv_max_interval_get(void *data, u64 *val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	*val = hdev->le_adv_max_interval;
	hci_dev_unlock(hdev);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(adv_max_interval_fops, adv_max_interval_get,
			  adv_max_interval_set, "%llu\n");

static int min_key_size_set(void *data, u64 val)
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val > hdev->le_max_key_size || val < SMP_MIN_ENC_KEY_SIZE) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}