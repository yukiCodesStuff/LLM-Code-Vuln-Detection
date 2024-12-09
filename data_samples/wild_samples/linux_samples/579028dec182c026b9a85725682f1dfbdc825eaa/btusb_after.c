	if (!bacmp(&ver->otp_bd_addr, BDADDR_ANY)) {
		bt_dev_info(hdev, "No device address configured");
		set_bit(HCI_QUIRK_INVALID_BDADDR, &hdev->quirks);
	}

	btusb_setup_intel_newgen_get_fw_name(ver, fwname, sizeof(fwname), "sfi");
	err = firmware_request_nowarn(&fw, fwname, &hdev->dev);
	if (err < 0) {
		if (!test_bit(BTUSB_BOOTLOADER, &data->flags)) {
			/* Firmware has already been loaded */
			set_bit(BTUSB_FIRMWARE_LOADED, &data->flags);
			return 0;
		}

		bt_dev_err(hdev, "Failed to load Intel firmware file %s (%d)",
			   fwname, err);

		return err;
	}
	if (!bacmp(&params->otp_bdaddr, BDADDR_ANY)) {
		bt_dev_info(hdev, "No device address configured");
		set_bit(HCI_QUIRK_INVALID_BDADDR, &hdev->quirks);
	}

download:
	/* With this Intel bootloader only the hardware variant and device
	 * revision information are used to select the right firmware for SfP
	 * and WsP.
	 *
	 * The firmware filename is ibt-<hw_variant>-<dev_revid>.sfi.
	 *
	 * Currently the supported hardware variants are:
	 *   11 (0x0b) for iBT3.0 (LnP/SfP)
	 *   12 (0x0c) for iBT3.5 (WsP)
	 *
	 * For ThP/JfP and for future SKU's, the FW name varies based on HW
	 * variant, HW revision and FW revision, as these are dependent on CNVi
	 * and RF Combination.
	 *
	 *   17 (0x11) for iBT3.5 (JfP)
	 *   18 (0x12) for iBT3.5 (ThP)
	 *
	 * The firmware file name for these will be
	 * ibt-<hw_variant>-<hw_revision>-<fw_revision>.sfi.
	 *
	 */
	err = btusb_setup_intel_new_get_fw_name(ver, params, fwname,
						sizeof(fwname), "sfi");
	if (err < 0) {
		if (!test_bit(BTUSB_BOOTLOADER, &data->flags)) {
			/* Firmware has already been loaded */
			set_bit(BTUSB_FIRMWARE_LOADED, &data->flags);
			return 0;
		}

		bt_dev_err(hdev, "Unsupported Intel firmware naming");
		return -EINVAL;
	}