
	BT_DBG("conn %p hcon %p level 0x%2.2x", conn, hcon, sec_level);

	if (!test_bit(HCI_LE_ENABLED, &hcon->hdev->dev_flags))
		return 1;

	if (sec_level == BT_SECURITY_LOW)
		return 1;
	__u8 reason;
	int err = 0;

	if (!test_bit(HCI_LE_ENABLED, &conn->hcon->hdev->dev_flags)) {
		err = -ENOTSUPP;
		reason = SMP_PAIRING_NOTSUPP;
		goto done;
	}