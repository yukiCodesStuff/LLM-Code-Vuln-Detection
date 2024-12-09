			if (udata[i] > 3 && memscan(keys + 2, udata[i], 6) == keys + 8) {
				if (hidp_keycode[udata[i]])
					input_report_key(dev, hidp_keycode[udata[i]], 1);
				else
					BT_ERR("Unknown key (scancode %#x) pressed.", udata[i]);
			}
		}

		memcpy(keys, udata, 8);
		break;

	case 0x02:	/* Mouse report */
		input_report_key(dev, BTN_LEFT,   sdata[0] & 0x01);
		input_report_key(dev, BTN_RIGHT,  sdata[0] & 0x02);
		input_report_key(dev, BTN_MIDDLE, sdata[0] & 0x04);
		input_report_key(dev, BTN_SIDE,   sdata[0] & 0x08);
		input_report_key(dev, BTN_EXTRA,  sdata[0] & 0x10);

		input_report_rel(dev, REL_X, sdata[1]);
		input_report_rel(dev, REL_Y, sdata[2]);

		if (size > 3)
			input_report_rel(dev, REL_WHEEL, sdata[3]);
		break;
	}

	input_sync(dev);
}

static int hidp_send_report(struct hidp_session *session, struct hid_report *report)
{
	unsigned char hdr;
	u8 *buf;
	int rsize, ret;

	buf = hid_alloc_report_buf(report, GFP_ATOMIC);
	if (!buf)
		return -EIO;

	hid_output_report(report, buf);
	hdr = HIDP_TRANS_DATA | HIDP_DATA_RTYPE_OUPUT;

	rsize = ((report->size - 1) >> 3) + 1 + (report->id > 0);
	ret = hidp_send_intr_message(session, hdr, buf, rsize);

	kfree(buf);
	return ret;
}