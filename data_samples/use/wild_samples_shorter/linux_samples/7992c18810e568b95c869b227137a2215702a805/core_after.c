		del_timer(&session->timer);
}

static void hidp_process_report(struct hidp_session *session, int type,
				const u8 *data, unsigned int len, int intr)
{
	if (len > HID_MAX_BUFFER_SIZE)
		len = HID_MAX_BUFFER_SIZE;
