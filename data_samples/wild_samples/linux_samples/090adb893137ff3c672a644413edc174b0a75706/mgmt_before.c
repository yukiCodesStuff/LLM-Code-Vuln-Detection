		if (!lmp_bredr_capable(hdev)) {
			err = cmd_status(sk, hdev->id, MGMT_OP_START_DISCOVERY,
					 MGMT_STATUS_NOT_SUPPORTED);
			mgmt_pending_remove(cmd);
			goto failed;
		}

		err = hci_do_inquiry(hdev, INQUIRY_LEN_BREDR);
		break;

	case DISCOV_TYPE_LE:
		if (!lmp_host_le_capable(hdev)) {
			err = cmd_status(sk, hdev->id, MGMT_OP_START_DISCOVERY,
					 MGMT_STATUS_NOT_SUPPORTED);
			mgmt_pending_remove(cmd);
			goto failed;
		}
	if (powered) {
		if (powered_update_hci(hdev) == 0)
			return 0;

		mgmt_pending_foreach(MGMT_OP_SET_POWERED, hdev, settings_rsp,
				     &match);
		goto new_settings;
	}

	mgmt_pending_foreach(MGMT_OP_SET_POWERED, hdev, settings_rsp, &match);
	mgmt_pending_foreach(0, hdev, cmd_status_rsp, &status_not_powered);

	if (memcmp(hdev->dev_class, zero_cod, sizeof(zero_cod)) != 0)
		mgmt_event(MGMT_EV_CLASS_OF_DEV_CHANGED, hdev,
			   zero_cod, sizeof(zero_cod), NULL);

new_settings:
	err = new_settings(hdev, match.sk);

	if (match.sk)
		sock_put(match.sk);

	return err;
}

int mgmt_discoverable(struct hci_dev *hdev, u8 discoverable)
{
	struct cmd_lookup match = { NULL, hdev };
	bool changed = false;
	int err = 0;

	if (discoverable) {
		if (!test_and_set_bit(HCI_DISCOVERABLE, &hdev->dev_flags))
			changed = true;
	} else {
		if (test_and_clear_bit(HCI_DISCOVERABLE, &hdev->dev_flags))
			changed = true;
	}

	mgmt_pending_foreach(MGMT_OP_SET_DISCOVERABLE, hdev, settings_rsp,
			     &match);

	if (changed)
		err = new_settings(hdev, match.sk);

	if (match.sk)
		sock_put(match.sk);

	return err;
}