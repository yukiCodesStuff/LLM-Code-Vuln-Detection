
	if (ir == WIIPROTO_FLAG_IR_BASIC) {
		if (wdata->state.flags & WIIPROTO_FLAG_ACCEL) {
			/* GEN10 and ealier devices bind IR formats to DRMs.
			 * Hence, we cannot use DRM_KAI here as it might be
			 * bound to IR_EXT. Use DRM_KAIE unconditionally so we
			 * work with all devices and our parsers can use the
			 * fixed formats, too. */
			return WIIPROTO_REQ_DRM_KAIE;
		} else {
			return WIIPROTO_REQ_DRM_KIE;
		}
	} else if (ir == WIIPROTO_FLAG_IR_EXT) {
	if (ret != 6)
		return WIIMOTE_EXT_NONE;

	hid_dbg(wdata->hdev, "extension ID: %6phC\n", rmem);

	if (rmem[0] == 0xff && rmem[1] == 0xff && rmem[2] == 0xff &&
	    rmem[3] == 0xff && rmem[4] == 0xff && rmem[5] == 0xff)
		return WIIMOTE_EXT_NONE;
		return WIIMOTE_EXT_BALANCE_BOARD;
	if (rmem[4] == 0x01 && rmem[5] == 0x20)
		return WIIMOTE_EXT_PRO_CONTROLLER;
	if (rmem[0] == 0x01 && rmem[1] == 0x00 &&
	    rmem[4] == 0x01 && rmem[5] == 0x03)
		return WIIMOTE_EXT_GUITAR_HERO_DRUMS;
	if (rmem[0] == 0x00 && rmem[1] == 0x00 &&
	    rmem[4] == 0x01 && rmem[5] == 0x03)
		return WIIMOTE_EXT_GUITAR_HERO_GUITAR;

	return WIIMOTE_EXT_UNKNOWN;
}

	/* map MP with correct pass-through mode */
	switch (exttype) {
	case WIIMOTE_EXT_CLASSIC_CONTROLLER:
	case WIIMOTE_EXT_GUITAR_HERO_DRUMS:
	case WIIMOTE_EXT_GUITAR_HERO_GUITAR:
		wmem = 0x07;
		break;
	case WIIMOTE_EXT_NUNCHUK:
		wmem = 0x05;
	if (ret != 6)
		return false;

	hid_dbg(wdata->hdev, "motion plus ID: %6phC\n", rmem);

	if (rmem[5] == 0x05)
		return true;

	hid_info(wdata->hdev, "unknown motion plus ID: %6phC\n", rmem);

	return false;
}

	if (ret != 6)
		return WIIMOTE_MP_NONE;

	hid_dbg(wdata->hdev, "mapped motion plus ID: %6phC\n", rmem);

	if (rmem[0] == 0xff && rmem[1] == 0xff && rmem[2] == 0xff &&
	    rmem[3] == 0xff && rmem[4] == 0xff && rmem[5] == 0xff)
		return WIIMOTE_MP_NONE;
	[WIIMOTE_EXT_CLASSIC_CONTROLLER] = "Nintendo Wii Classic Controller",
	[WIIMOTE_EXT_BALANCE_BOARD] = "Nintendo Wii Balance Board",
	[WIIMOTE_EXT_PRO_CONTROLLER] = "Nintendo Wii U Pro Controller",
	[WIIMOTE_EXT_GUITAR_HERO_DRUMS] = "Nintendo Wii Guitar Hero Drums",
	[WIIMOTE_EXT_GUITAR_HERO_GUITAR] = "Nintendo Wii Guitar Hero Guitar",
};

/*
 * Handle hotplug events
		wiimote_ext_unload(wdata);

		if (exttype == WIIMOTE_EXT_UNKNOWN) {
			hid_info(wdata->hdev, "cannot detect extension; %6phC\n",
				 extdata);
		} else if (exttype == WIIMOTE_EXT_NONE) {
			spin_lock_irq(&wdata->state.lock);
			wdata->state.exttype = WIIMOTE_EXT_NONE;
			spin_unlock_irq(&wdata->state.lock);
		return sprintf(buf, "balanceboard\n");
	case WIIMOTE_EXT_PRO_CONTROLLER:
		return sprintf(buf, "procontroller\n");
	case WIIMOTE_EXT_GUITAR_HERO_DRUMS:
		return sprintf(buf, "drums\n");
	case WIIMOTE_EXT_GUITAR_HERO_GUITAR:
		return sprintf(buf, "guitar\n");
	case WIIMOTE_EXT_UNKNOWN:
		/* fallthrough */
	default:
		return sprintf(buf, "unknown\n");