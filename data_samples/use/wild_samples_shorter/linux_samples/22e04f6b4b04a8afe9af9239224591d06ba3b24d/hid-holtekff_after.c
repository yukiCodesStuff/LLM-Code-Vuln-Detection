		holtekff->field->value[i] = data[i];
	}

	dbg_hid("sending %7ph\n", data);

	hid_hw_request(hid, holtekff->field->report, HID_REQ_SET_REPORT);
}
