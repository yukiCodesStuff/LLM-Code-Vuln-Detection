
	if (ir == WIIPROTO_FLAG_IR_BASIC) {
		if (wdata->state.flags & WIIPROTO_FLAG_ACCEL) {
			if (ext)
				return WIIPROTO_REQ_DRM_KAIE;
			else
				return WIIPROTO_REQ_DRM_KAI;
		} else {
			return WIIPROTO_REQ_DRM_KIE;
		}
	} else if (ir == WIIPROTO_FLAG_IR_EXT) {
	if (ret != 6)
		return WIIMOTE_EXT_NONE;

	hid_dbg(wdata->hdev, "extension ID: %02x:%02x %02x:%02x %02x:%02x\n",
		rmem[0], rmem[1], rmem[2], rmem[3], rmem[4], rmem[5]);

	if (rmem[0] == 0xff && rmem[1] == 0xff && rmem[2] == 0xff &&
	    rmem[3] == 0xff && rmem[4] == 0xff && rmem[5] == 0xff)
		return WIIMOTE_EXT_NONE;
		return WIIMOTE_EXT_BALANCE_BOARD;
	if (rmem[4] == 0x01 && rmem[5] == 0x20)
		return WIIMOTE_EXT_PRO_CONTROLLER;

	return WIIMOTE_EXT_UNKNOWN;
}

	/* map MP with correct pass-through mode */
	switch (exttype) {
	case WIIMOTE_EXT_CLASSIC_CONTROLLER:
		wmem = 0x07;
		break;
	case WIIMOTE_EXT_NUNCHUK:
		wmem = 0x05;
	if (ret != 6)
		return false;

	hid_dbg(wdata->hdev, "motion plus ID: %02x:%02x %02x:%02x %02x:%02x\n",
		rmem[0], rmem[1], rmem[2], rmem[3], rmem[4], rmem[5]);

	if (rmem[5] == 0x05)
		return true;

	hid_info(wdata->hdev, "unknown motion plus ID: %02x:%02x %02x:%02x %02x:%02x\n",
		 rmem[0], rmem[1], rmem[2], rmem[3], rmem[4], rmem[5]);

	return false;
}

	if (ret != 6)
		return WIIMOTE_MP_NONE;

	hid_dbg(wdata->hdev, "mapped motion plus ID: %02x:%02x %02x:%02x %02x:%02x\n",
		rmem[0], rmem[1], rmem[2], rmem[3], rmem[4], rmem[5]);

	if (rmem[0] == 0xff && rmem[1] == 0xff && rmem[2] == 0xff &&
	    rmem[3] == 0xff && rmem[4] == 0xff && rmem[5] == 0xff)
		return WIIMOTE_MP_NONE;
	[WIIMOTE_EXT_CLASSIC_CONTROLLER] = "Nintendo Wii Classic Controller",
	[WIIMOTE_EXT_BALANCE_BOARD] = "Nintendo Wii Balance Board",
	[WIIMOTE_EXT_PRO_CONTROLLER] = "Nintendo Wii U Pro Controller",
};

/*
 * Handle hotplug events
		wiimote_ext_unload(wdata);

		if (exttype == WIIMOTE_EXT_UNKNOWN) {
			hid_info(wdata->hdev, "cannot detect extension; %02x:%02x %02x:%02x %02x:%02x\n",
				 extdata[0], extdata[1], extdata[2],
				 extdata[3], extdata[4], extdata[5]);
		} else if (exttype == WIIMOTE_EXT_NONE) {
			spin_lock_irq(&wdata->state.lock);
			wdata->state.exttype = WIIMOTE_EXT_NONE;
			spin_unlock_irq(&wdata->state.lock);
		return sprintf(buf, "balanceboard\n");
	case WIIMOTE_EXT_PRO_CONTROLLER:
		return sprintf(buf, "procontroller\n");
	case WIIMOTE_EXT_UNKNOWN:
		/* fallthrough */
	default:
		return sprintf(buf, "unknown\n");