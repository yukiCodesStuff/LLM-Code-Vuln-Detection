		break;

	case DISCOV_TYPE_LE:
		if (!lmp_host_le_capable(hdev)) {
			err = cmd_status(sk, hdev->id, MGMT_OP_START_DISCOVERY,
					 MGMT_STATUS_NOT_SUPPORTED);
			mgmt_pending_remove(cmd);
			goto failed;
	return err;
}

int mgmt_discoverable(struct hci_dev *hdev, u8 discoverable)
{
	struct cmd_lookup match = { NULL, hdev };
	bool changed = false;