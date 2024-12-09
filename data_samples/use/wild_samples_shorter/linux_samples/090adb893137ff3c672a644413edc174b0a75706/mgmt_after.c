		break;

	case DISCOV_TYPE_LE:
		if (!test_bit(HCI_LE_ENABLED, &hdev->dev_flags)) {
			err = cmd_status(sk, hdev->id, MGMT_OP_START_DISCOVERY,
					 MGMT_STATUS_NOT_SUPPORTED);
			mgmt_pending_remove(cmd);
			goto failed;
	return err;
}

int mgmt_set_powered_failed(struct hci_dev *hdev, int err)
{
	struct pending_cmd *cmd;
	u8 status;

	cmd = mgmt_pending_find(MGMT_OP_SET_POWERED, hdev);
	if (!cmd)
		return -ENOENT;

	if (err == -ERFKILL)
		status = MGMT_STATUS_RFKILLED;
	else
		status = MGMT_STATUS_FAILED;

	err = cmd_status(cmd->sk, hdev->id, MGMT_OP_SET_POWERED, status);

	mgmt_pending_remove(cmd);

	return err;
}

int mgmt_discoverable(struct hci_dev *hdev, u8 discoverable)
{
	struct cmd_lookup match = { NULL, hdev };
	bool changed = false;