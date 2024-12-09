void picolcd_debug_out_report(struct picolcd_data *data,
		struct hid_device *hdev, struct hid_report *report)
{
	u8 *raw_data;
	int raw_size = (report->size >> 3) + 1;
	char *buff;
#define BUFF_SZ 256

	if (!buff)
		return;

	raw_data = hid_alloc_report_buf(report, GFP_ATOMIC);
	if (!raw_data) {
		kfree(buff);
		return;
	}

	snprintf(buff, BUFF_SZ, "\nout report %d (size %d) =  ",
			report->id, raw_size);
	hid_debug_event(hdev, buff);
	raw_data[0] = report->id;
	hid_output_report(report, raw_data);
	dump_buff_as_hex(buff, BUFF_SZ, raw_data, raw_size);
	hid_debug_event(hdev, buff);

	switch (report->id) {
	case REPORT_LED_STATE:
		/* 1 data byte with GPO state */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
		break;
	}
	wake_up_interruptible(&hdev->debug_wait);
	kfree(raw_data);
	kfree(buff);
}

void picolcd_debug_raw_event(struct picolcd_data *data,