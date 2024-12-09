	}

	btusb_setup_intel_newgen_get_fw_name(ver, fwname, sizeof(fwname), "sfi");
	err = request_firmware(&fw, fwname, &hdev->dev);
	if (err < 0) {
		bt_dev_err(hdev, "Failed to load Intel firmware file %s (%d)",
			   fwname, err);
		return err;
	}

	bt_dev_info(hdev, "Found device firmware: %s", fwname);
	err = btusb_setup_intel_new_get_fw_name(ver, params, fwname,
						sizeof(fwname), "sfi");
	if (err < 0) {
		bt_dev_err(hdev, "Unsupported Intel firmware naming");
		return -EINVAL;
	}

	err = request_firmware(&fw, fwname, &hdev->dev);
	if (err < 0) {
		bt_dev_err(hdev, "Failed to load Intel firmware file %s (%d)",
			   fwname, err);
		return err;
	}