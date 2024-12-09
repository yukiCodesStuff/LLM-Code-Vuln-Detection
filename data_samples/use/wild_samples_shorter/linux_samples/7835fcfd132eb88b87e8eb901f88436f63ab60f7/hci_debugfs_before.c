{
	struct hci_dev *hdev = data;

	if (val == 0 || val > hdev->conn_info_max_age)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->conn_info_min_age = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	if (val == 0 || val < hdev->conn_info_min_age)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->conn_info_max_age = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	if (val == 0 || val % 2 || val > hdev->sniff_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->sniff_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	if (val == 0 || val % 2 || val < hdev->sniff_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->sniff_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	if (val < 0x0006 || val > 0x0c80 || val > hdev->le_conn_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_conn_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	if (val < 0x0006 || val > 0x0c80 || val < hdev->le_conn_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_conn_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val > hdev->le_adv_max_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_min_interval = val;
	hci_dev_unlock(hdev);

	return 0;
{
	struct hci_dev *hdev = data;

	if (val < 0x0020 || val > 0x4000 || val < hdev->le_adv_min_interval)
		return -EINVAL;

	hci_dev_lock(hdev);
	hdev->le_adv_max_interval = val;
	hci_dev_unlock(hdev);

	return 0;