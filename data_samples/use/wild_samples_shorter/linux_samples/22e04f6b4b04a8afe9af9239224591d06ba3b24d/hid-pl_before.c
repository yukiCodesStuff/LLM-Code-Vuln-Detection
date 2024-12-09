			strong = &report->field[0]->value[2];
			weak = &report->field[0]->value[3];
			debug("detected single-field device");
		} else if (report->maxfield >= 4 && report->field[0]->maxusage == 1 &&
				report->field[0]->usage[0].hid == (HID_UP_LED | 0x43)) {
			report->field[0]->value[0] = 0x00;
			report->field[1]->value[0] = 0x00;
			strong = &report->field[2]->value[0];
			weak = &report->field[3]->value[0];