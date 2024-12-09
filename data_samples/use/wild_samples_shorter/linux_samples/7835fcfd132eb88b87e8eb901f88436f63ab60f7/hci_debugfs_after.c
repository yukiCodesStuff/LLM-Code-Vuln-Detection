{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val == 0 || val > hdev->conn_info_max_age) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->conn_info_min_age = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val == 0 || val < hdev->conn_info_min_age) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->conn_info_max_age = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val == 0 || val % 2 || val > hdev->sniff_max_interval) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->sniff_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val == 0 || val % 2 || val < hdev->sniff_min_interval) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->sniff_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val < 0x0006 || val > 0x0c80 || val > hdev->le_conn_max_interval) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->le_conn_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val < 0x0006 || val > 0x0c80 || val < hdev->le_conn_min_interval) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->le_conn_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val < 0x0020 || val > 0x4000 || val > hdev->le_adv_max_interval) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->le_adv_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	hci_dev_lock(hdev);
	if (val < 0x0020 || val > 0x4000 || val < hdev->le_adv_min_interval) {
		hci_dev_unlock(hdev);
		return -EINVAL;
	}

	hdev->le_adv_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;