
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

static int hidp_hidinput_event(struct input_dev *dev, unsigned int type,
			       unsigned int code, int value)