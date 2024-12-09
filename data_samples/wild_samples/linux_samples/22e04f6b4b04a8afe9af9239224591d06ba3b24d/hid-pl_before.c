		if (report->field[0]->report_count >= 4) {
			report->field[0]->value[0] = 0x00;
			report->field[0]->value[1] = 0x00;
			strong = &report->field[0]->value[2];
			weak = &report->field[0]->value[3];
			debug("detected single-field device");
		} else if (report->maxfield >= 4 && report->field[0]->maxusage == 1 &&
				report->field[0]->usage[0].hid == (HID_UP_LED | 0x43)) {
			report->field[0]->value[0] = 0x00;
			report->field[1]->value[0] = 0x00;
			strong = &report->field[2]->value[0];
			weak = &report->field[3]->value[0];
			if (hid->vendor == USB_VENDOR_ID_JESS2)
				maxval = 0xff;
			debug("detected 4-field device");
		} else {
			hid_err(hid, "not enough fields or values\n");
			return -ENODEV;
		}

		plff = kzalloc(sizeof(struct plff_device), GFP_KERNEL);
		if (!plff)
			return -ENOMEM;

		dev = hidinput->input;

		set_bit(FF_RUMBLE, dev->ffbit);

		error = input_ff_create_memless(dev, plff, hid_plff_play);
		if (error) {
			kfree(plff);
			return error;
		}

		plff->report = report;
		plff->strong = strong;
		plff->weak = weak;
		plff->maxval = maxval;

		*strong = 0x00;
		*weak = 0x00;
		hid_hw_request(hid, plff->report, HID_REQ_SET_REPORT);
	}