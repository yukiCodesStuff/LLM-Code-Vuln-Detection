
static int hidp_send_report(struct hidp_session *session, struct hid_report *report)
{
	unsigned char buf[32], hdr;
	int rsize;

	rsize = ((report->size - 1) >> 3) + 1 + (report->id > 0);
	if (rsize > sizeof(buf))
		return -EIO;

	hid_output_report(report, buf);
	hdr = HIDP_TRANS_DATA | HIDP_DATA_RTYPE_OUPUT;

	return hidp_send_intr_message(session, hdr, buf, rsize);
}

static int hidp_hidinput_event(struct input_dev *dev, unsigned int type,
			       unsigned int code, int value)