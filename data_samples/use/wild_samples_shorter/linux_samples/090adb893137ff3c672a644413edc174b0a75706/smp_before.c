
	BT_DBG("conn %p hcon %p level 0x%2.2x", conn, hcon, sec_level);

	if (!lmp_host_le_capable(hcon->hdev))
		return 1;

	if (sec_level == BT_SECURITY_LOW)
		return 1;
	__u8 reason;
	int err = 0;

	if (!lmp_host_le_capable(conn->hcon->hdev)) {
		err = -ENOTSUPP;
		reason = SMP_PAIRING_NOTSUPP;
		goto done;
	}