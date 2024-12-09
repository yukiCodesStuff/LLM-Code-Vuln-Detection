	for (i = j = 0; i < data_len && j + 4 < dst_sz; i++) {
		dst[j++] = hex_asc[(data[i] >> 4) & 0x0f];
		dst[j++] = hex_asc[data[i] & 0x0f];
		dst[j++] = ' ';
	}
	dst[j]   = '\0';
	if (j > 0)
		dst[j-1] = '\n';
	if (i < data_len && j > 2)
		dst[j-2] = dst[j-3] = '.';
}

void picolcd_debug_out_report(struct picolcd_data *data,
		struct hid_device *hdev, struct hid_report *report)
{
	u8 raw_data[70];
	int raw_size = (report->size >> 3) + 1;
	char *buff;
#define BUFF_SZ 256

	/* Avoid unnecessary overhead if debugfs is disabled */
	if (list_empty(&hdev->debug_list))
		return;

	buff = kmalloc(BUFF_SZ, GFP_ATOMIC);
	if (!buff)
		return;

	snprintf(buff, BUFF_SZ, "\nout report %d (size %d) =  ",
			report->id, raw_size);
	hid_debug_event(hdev, buff);
	if (raw_size + 5 > sizeof(raw_data)) {
		kfree(buff);
		hid_debug_event(hdev, " TOO BIG\n");
		return;
	} else {
		raw_data[0] = report->id;
		hid_output_report(report, raw_data);
		dump_buff_as_hex(buff, BUFF_SZ, raw_data, raw_size);
		hid_debug_event(hdev, buff);
	}

	switch (report->id) {
	case REPORT_LED_STATE:
		/* 1 data byte with GPO state */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LED_STATE", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tGPO state: 0x%02x\n", raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_BRIGHTNESS:
		/* 1 data byte with brightness */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_BRIGHTNESS", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tBrightness: 0x%02x\n", raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_CONTRAST:
		/* 1 data byte with contrast */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_CONTRAST", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tContrast: 0x%02x\n", raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_RESET:
		/* 2 data bytes with reset duration in ms */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_RESET", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tDuration: 0x%02x%02x (%dms)\n",
				raw_data[2], raw_data[1], raw_data[2] << 8 | raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_LCD_CMD:
		/* 63 data bytes with LCD commands */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LCD_CMD", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		/* TODO: format decoding */
		break;
	case REPORT_LCD_DATA:
		/* 63 data bytes with LCD data */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LCD_CMD", report->id, raw_size-1);
		/* TODO: format decoding */
		hid_debug_event(hdev, buff);
		break;
	case REPORT_LCD_CMD_DATA:
		/* 63 data bytes with LCD commands and data */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LCD_CMD", report->id, raw_size-1);
		/* TODO: format decoding */
		hid_debug_event(hdev, buff);
		break;
	case REPORT_EE_READ:
		/* 3 data bytes with read area description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_EE_READ", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_EE_WRITE:
		/* 3+1..20 data bytes with write area description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_EE_WRITE", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
		hid_debug_event(hdev, buff);
		if (raw_data[3] == 0) {
			snprintf(buff, BUFF_SZ, "\tNo data\n");
		} else if (raw_data[3] + 4 <= raw_size) {
			snprintf(buff, BUFF_SZ, "\tData: ");
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
		} else {
			snprintf(buff, BUFF_SZ, "\tData overflowed\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_ERASE_MEMORY:
	case REPORT_BL_ERASE_MEMORY:
		/* 3 data bytes with pointer inside erase block */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_ERASE_MEMORY", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tAddress inside 64 byte block: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tAddress inside 64 byte block: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_READ_MEMORY:
	case REPORT_BL_READ_MEMORY:
		/* 4 data bytes with read area description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_READ_MEMORY", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[4]);
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_WRITE_MEMORY:
	case REPORT_BL_WRITE_MEMORY:
		/* 4+1..32 data bytes with write adrea description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_WRITE_MEMORY", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
			hid_debug_event(hdev, buff);
			if (raw_data[3] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[3] + 4 <= raw_size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[4]);
			hid_debug_event(hdev, buff);
			if (raw_data[4] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[4] + 5 <= raw_size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+5, raw_data[4]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_RESTART:
		/* TODO */
		break;
	case REPORT_EXIT_KEYBOARD:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_EXIT_KEYBOARD", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tRestart delay: %dms (0x%02x%02x)\n",
				raw_data[1] | (raw_data[2] << 8),
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_VERSION:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_DEVID:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_DEVID", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_SIZE:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_SPLASH_SIZE", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_HOOK_VERSION:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_HOOK_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_EXIT_FLASHER:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tRestart delay: %dms (0x%02x%02x)\n",
				raw_data[1] | (raw_data[2] << 8),
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	default:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"<unknown>", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	}
	wake_up_interruptible(&hdev->debug_wait);
	kfree(buff);
}

void picolcd_debug_raw_event(struct picolcd_data *data,
		struct hid_device *hdev, struct hid_report *report,
		u8 *raw_data, int size)
{
	char *buff;

#define BUFF_SZ 256
	/* Avoid unnecessary overhead if debugfs is disabled */
	if (list_empty(&hdev->debug_list))
		return;

	buff = kmalloc(BUFF_SZ, GFP_ATOMIC);
	if (!buff)
		return;

	switch (report->id) {
	case REPORT_ERROR_CODE:
		/* 2 data bytes with affected report and error code */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_ERROR_CODE", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[2] < ARRAY_SIZE(error_codes))
			snprintf(buff, BUFF_SZ, "\tError code 0x%02x (%s) in reply to report 0x%02x\n",
					raw_data[2], error_codes[raw_data[2]], raw_data[1]);
		else
			snprintf(buff, BUFF_SZ, "\tError code 0x%02x in reply to report 0x%02x\n",
					raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_KEY_STATE:
		/* 2 data bytes with key state */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_KEY_STATE", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[1] == 0)
			snprintf(buff, BUFF_SZ, "\tNo key pressed\n");
		else if (raw_data[2] == 0)
			snprintf(buff, BUFF_SZ, "\tOne key pressed: 0x%02x (%d)\n",
					raw_data[1], raw_data[1]);
		else
			snprintf(buff, BUFF_SZ, "\tTwo keys pressed: 0x%02x (%d), 0x%02x (%d)\n",
					raw_data[1], raw_data[1], raw_data[2], raw_data[2]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_IR_DATA:
		/* Up to 20 byes of IR scancode data */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_IR_DATA", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[1] == 0) {
			snprintf(buff, BUFF_SZ, "\tUnexpectedly 0 data length\n");
			hid_debug_event(hdev, buff);
		} else if (raw_data[1] + 1 <= size) {
			snprintf(buff, BUFF_SZ, "\tData length: %d\n\tIR Data: ",
					raw_data[1]);
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+2, raw_data[1]);
			hid_debug_event(hdev, buff);
		} else {
			snprintf(buff, BUFF_SZ, "\tOverflowing data length: %d\n",
					raw_data[1]-1);
			hid_debug_event(hdev, buff);
		}
		break;
	case REPORT_EE_DATA:
		/* Data buffer in response to REPORT_EE_READ or REPORT_EE_WRITE */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_EE_DATA", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
		hid_debug_event(hdev, buff);
		if (raw_data[3] == 0) {
			snprintf(buff, BUFF_SZ, "\tNo data\n");
			hid_debug_event(hdev, buff);
		} else if (raw_data[3] + 4 <= size) {
			snprintf(buff, BUFF_SZ, "\tData: ");
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			hid_debug_event(hdev, buff);
		} else {
			snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			hid_debug_event(hdev, buff);
		}
		break;
	case REPORT_MEMORY:
		/* Data buffer in response to REPORT_READ_MEMORY or REPORT_WRTIE_MEMORY */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
			hid_debug_event(hdev, buff);
			if (raw_data[3] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[3] + 4 <= size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[4]);
			hid_debug_event(hdev, buff);
			if (raw_data[4] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[4] + 5 <= size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+5, raw_data[4]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_VERSION:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tFirmware version: %d.%d\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_BL_ERASE_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_ERASE_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_BL_READ_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_READ_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_BL_WRITE_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_WRITE_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_DEVID:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_DEVID", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tSerial: 0x%02x%02x%02x%02x\n",
				raw_data[1], raw_data[2], raw_data[3], raw_data[4]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tType: 0x%02x\n",
				raw_data[5]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_SIZE:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_SPLASH_SIZE", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tTotal splash space: %d\n",
				(raw_data[2] << 8) | raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tUsed splash space: %d\n",
				(raw_data[4] << 8) | raw_data[3]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_HOOK_VERSION:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_HOOK_VERSION", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tFirmware version: %d.%d\n",
				raw_data[1], raw_data[2]);
		hid_debug_event(hdev, buff);
		break;
	default:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"<unknown>", report->id, size-1);
		hid_debug_event(hdev, buff);
		break;
	}
	wake_up_interruptible(&hdev->debug_wait);
	kfree(buff);
}

void picolcd_init_devfs(struct picolcd_data *data,
		struct hid_report *eeprom_r, struct hid_report *eeprom_w,
		struct hid_report *flash_r, struct hid_report *flash_w,
		struct hid_report *reset)
{
	struct hid_device *hdev = data->hdev;

	mutex_init(&data->mutex_flash);

	/* reset */
	if (reset)
		data->debug_reset = debugfs_create_file("reset", 0600,
				hdev->debug_dir, data, &picolcd_debug_reset_fops);

	/* eeprom */
	if (eeprom_r || eeprom_w)
		data->debug_eeprom = debugfs_create_file("eeprom",
			(eeprom_w ? S_IWUSR : 0) | (eeprom_r ? S_IRUSR : 0),
			hdev->debug_dir, data, &picolcd_debug_eeprom_fops);

	/* flash */
	if (flash_r && flash_r->maxfield == 1 && flash_r->field[0]->report_size == 8)
		data->addr_sz = flash_r->field[0]->report_count - 1;
	else
		data->addr_sz = -1;
	if (data->addr_sz == 2 || data->addr_sz == 3) {
		data->debug_flash = debugfs_create_file("flash",
			(flash_w ? S_IWUSR : 0) | (flash_r ? S_IRUSR : 0),
			hdev->debug_dir, data, &picolcd_debug_flash_fops);
	} else if (flash_r || flash_w)
		hid_warn(hdev, "Unexpected FLASH access reports, please submit rdesc for review\n");
}

void picolcd_exit_devfs(struct picolcd_data *data)
{
	struct dentry *dent;

	dent = data->debug_reset;
	data->debug_reset = NULL;
	if (dent)
		debugfs_remove(dent);
	dent = data->debug_eeprom;
	data->debug_eeprom = NULL;
	if (dent)
		debugfs_remove(dent);
	dent = data->debug_flash;
	data->debug_flash = NULL;
	if (dent)
		debugfs_remove(dent);
	mutex_destroy(&data->mutex_flash);
}

{
	u8 raw_data[70];
	int raw_size = (report->size >> 3) + 1;
	char *buff;
#define BUFF_SZ 256

	/* Avoid unnecessary overhead if debugfs is disabled */
	if (list_empty(&hdev->debug_list))
		return;

	buff = kmalloc(BUFF_SZ, GFP_ATOMIC);
	if (!buff)
		return;

	snprintf(buff, BUFF_SZ, "\nout report %d (size %d) =  ",
			report->id, raw_size);
	hid_debug_event(hdev, buff);
	if (raw_size + 5 > sizeof(raw_data)) {
		kfree(buff);
		hid_debug_event(hdev, " TOO BIG\n");
		return;
	} else {
		raw_data[0] = report->id;
		hid_output_report(report, raw_data);
		dump_buff_as_hex(buff, BUFF_SZ, raw_data, raw_size);
		hid_debug_event(hdev, buff);
	}

	switch (report->id) {
	case REPORT_LED_STATE:
		/* 1 data byte with GPO state */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LED_STATE", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tGPO state: 0x%02x\n", raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_BRIGHTNESS:
		/* 1 data byte with brightness */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_BRIGHTNESS", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tBrightness: 0x%02x\n", raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_CONTRAST:
		/* 1 data byte with contrast */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_CONTRAST", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tContrast: 0x%02x\n", raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_RESET:
		/* 2 data bytes with reset duration in ms */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_RESET", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tDuration: 0x%02x%02x (%dms)\n",
				raw_data[2], raw_data[1], raw_data[2] << 8 | raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_LCD_CMD:
		/* 63 data bytes with LCD commands */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LCD_CMD", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		/* TODO: format decoding */
		break;
	case REPORT_LCD_DATA:
		/* 63 data bytes with LCD data */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LCD_CMD", report->id, raw_size-1);
		/* TODO: format decoding */
		hid_debug_event(hdev, buff);
		break;
	case REPORT_LCD_CMD_DATA:
		/* 63 data bytes with LCD commands and data */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_LCD_CMD", report->id, raw_size-1);
		/* TODO: format decoding */
		hid_debug_event(hdev, buff);
		break;
	case REPORT_EE_READ:
		/* 3 data bytes with read area description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_EE_READ", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_EE_WRITE:
		/* 3+1..20 data bytes with write area description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_EE_WRITE", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
		hid_debug_event(hdev, buff);
		if (raw_data[3] == 0) {
			snprintf(buff, BUFF_SZ, "\tNo data\n");
		} else if (raw_data[3] + 4 <= raw_size) {
			snprintf(buff, BUFF_SZ, "\tData: ");
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
		} else {
			snprintf(buff, BUFF_SZ, "\tData overflowed\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_ERASE_MEMORY:
	case REPORT_BL_ERASE_MEMORY:
		/* 3 data bytes with pointer inside erase block */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_ERASE_MEMORY", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tAddress inside 64 byte block: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tAddress inside 64 byte block: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_READ_MEMORY:
	case REPORT_BL_READ_MEMORY:
		/* 4 data bytes with read area description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_READ_MEMORY", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[4]);
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_WRITE_MEMORY:
	case REPORT_BL_WRITE_MEMORY:
		/* 4+1..32 data bytes with write adrea description */
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_WRITE_MEMORY", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
			hid_debug_event(hdev, buff);
			if (raw_data[3] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[3] + 4 <= raw_size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[4]);
			hid_debug_event(hdev, buff);
			if (raw_data[4] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[4] + 5 <= raw_size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+5, raw_data[4]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_RESTART:
		/* TODO */
		break;
	case REPORT_EXIT_KEYBOARD:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_EXIT_KEYBOARD", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tRestart delay: %dms (0x%02x%02x)\n",
				raw_data[1] | (raw_data[2] << 8),
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_VERSION:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_DEVID:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_DEVID", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_SIZE:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_SPLASH_SIZE", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_HOOK_VERSION:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_HOOK_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_EXIT_FLASHER:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tRestart delay: %dms (0x%02x%02x)\n",
				raw_data[1] | (raw_data[2] << 8),
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	default:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"<unknown>", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	}
	wake_up_interruptible(&hdev->debug_wait);
	kfree(buff);
}

void picolcd_debug_raw_event(struct picolcd_data *data,
		struct hid_device *hdev, struct hid_report *report,
		u8 *raw_data, int size)
{
	char *buff;

#define BUFF_SZ 256
	/* Avoid unnecessary overhead if debugfs is disabled */
	if (list_empty(&hdev->debug_list))
		return;

	buff = kmalloc(BUFF_SZ, GFP_ATOMIC);
	if (!buff)
		return;

	switch (report->id) {
	case REPORT_ERROR_CODE:
		/* 2 data bytes with affected report and error code */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_ERROR_CODE", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[2] < ARRAY_SIZE(error_codes))
			snprintf(buff, BUFF_SZ, "\tError code 0x%02x (%s) in reply to report 0x%02x\n",
					raw_data[2], error_codes[raw_data[2]], raw_data[1]);
		else
			snprintf(buff, BUFF_SZ, "\tError code 0x%02x in reply to report 0x%02x\n",
					raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_KEY_STATE:
		/* 2 data bytes with key state */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_KEY_STATE", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[1] == 0)
			snprintf(buff, BUFF_SZ, "\tNo key pressed\n");
		else if (raw_data[2] == 0)
			snprintf(buff, BUFF_SZ, "\tOne key pressed: 0x%02x (%d)\n",
					raw_data[1], raw_data[1]);
		else
			snprintf(buff, BUFF_SZ, "\tTwo keys pressed: 0x%02x (%d), 0x%02x (%d)\n",
					raw_data[1], raw_data[1], raw_data[2], raw_data[2]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_IR_DATA:
		/* Up to 20 byes of IR scancode data */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_IR_DATA", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[1] == 0) {
			snprintf(buff, BUFF_SZ, "\tUnexpectedly 0 data length\n");
			hid_debug_event(hdev, buff);
		} else if (raw_data[1] + 1 <= size) {
			snprintf(buff, BUFF_SZ, "\tData length: %d\n\tIR Data: ",
					raw_data[1]);
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+2, raw_data[1]);
			hid_debug_event(hdev, buff);
		} else {
			snprintf(buff, BUFF_SZ, "\tOverflowing data length: %d\n",
					raw_data[1]-1);
			hid_debug_event(hdev, buff);
		}
		break;
	case REPORT_EE_DATA:
		/* Data buffer in response to REPORT_EE_READ or REPORT_EE_WRITE */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_EE_DATA", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
		hid_debug_event(hdev, buff);
		if (raw_data[3] == 0) {
			snprintf(buff, BUFF_SZ, "\tNo data\n");
			hid_debug_event(hdev, buff);
		} else if (raw_data[3] + 4 <= size) {
			snprintf(buff, BUFF_SZ, "\tData: ");
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			hid_debug_event(hdev, buff);
		} else {
			snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			hid_debug_event(hdev, buff);
		}
		break;
	case REPORT_MEMORY:
		/* Data buffer in response to REPORT_READ_MEMORY or REPORT_WRTIE_MEMORY */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
			hid_debug_event(hdev, buff);
			if (raw_data[3] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[3] + 4 <= size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[4]);
			hid_debug_event(hdev, buff);
			if (raw_data[4] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[4] + 5 <= size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+5, raw_data[4]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_VERSION:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tFirmware version: %d.%d\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_BL_ERASE_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_ERASE_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_BL_READ_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_READ_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_BL_WRITE_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_WRITE_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_DEVID:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_DEVID", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tSerial: 0x%02x%02x%02x%02x\n",
				raw_data[1], raw_data[2], raw_data[3], raw_data[4]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tType: 0x%02x\n",
				raw_data[5]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_SIZE:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_SPLASH_SIZE", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tTotal splash space: %d\n",
				(raw_data[2] << 8) | raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tUsed splash space: %d\n",
				(raw_data[4] << 8) | raw_data[3]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_HOOK_VERSION:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_HOOK_VERSION", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tFirmware version: %d.%d\n",
				raw_data[1], raw_data[2]);
		hid_debug_event(hdev, buff);
		break;
	default:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"<unknown>", report->id, size-1);
		hid_debug_event(hdev, buff);
		break;
	}
	wake_up_interruptible(&hdev->debug_wait);
	kfree(buff);
}

void picolcd_init_devfs(struct picolcd_data *data,
		struct hid_report *eeprom_r, struct hid_report *eeprom_w,
		struct hid_report *flash_r, struct hid_report *flash_w,
		struct hid_report *reset)
{
	struct hid_device *hdev = data->hdev;

	mutex_init(&data->mutex_flash);

	/* reset */
	if (reset)
		data->debug_reset = debugfs_create_file("reset", 0600,
				hdev->debug_dir, data, &picolcd_debug_reset_fops);

	/* eeprom */
	if (eeprom_r || eeprom_w)
		data->debug_eeprom = debugfs_create_file("eeprom",
			(eeprom_w ? S_IWUSR : 0) | (eeprom_r ? S_IRUSR : 0),
			hdev->debug_dir, data, &picolcd_debug_eeprom_fops);

	/* flash */
	if (flash_r && flash_r->maxfield == 1 && flash_r->field[0]->report_size == 8)
		data->addr_sz = flash_r->field[0]->report_count - 1;
	else
		data->addr_sz = -1;
	if (data->addr_sz == 2 || data->addr_sz == 3) {
		data->debug_flash = debugfs_create_file("flash",
			(flash_w ? S_IWUSR : 0) | (flash_r ? S_IRUSR : 0),
			hdev->debug_dir, data, &picolcd_debug_flash_fops);
	} else if (flash_r || flash_w)
		hid_warn(hdev, "Unexpected FLASH access reports, please submit rdesc for review\n");
}

void picolcd_exit_devfs(struct picolcd_data *data)
{
	struct dentry *dent;

	dent = data->debug_reset;
	data->debug_reset = NULL;
	if (dent)
		debugfs_remove(dent);
	dent = data->debug_eeprom;
	data->debug_eeprom = NULL;
	if (dent)
		debugfs_remove(dent);
	dent = data->debug_flash;
	data->debug_flash = NULL;
	if (dent)
		debugfs_remove(dent);
	mutex_destroy(&data->mutex_flash);
}

			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_RESTART:
		/* TODO */
		break;
	case REPORT_EXIT_KEYBOARD:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_EXIT_KEYBOARD", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tRestart delay: %dms (0x%02x%02x)\n",
				raw_data[1] | (raw_data[2] << 8),
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_VERSION:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_DEVID:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_DEVID", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_SIZE:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_SPLASH_SIZE", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_HOOK_VERSION:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_HOOK_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_EXIT_FLASHER:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tRestart delay: %dms (0x%02x%02x)\n",
				raw_data[1] | (raw_data[2] << 8),
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	default:
		snprintf(buff, BUFF_SZ, "out report %s (%d, size=%d)\n",
			"<unknown>", report->id, raw_size-1);
		hid_debug_event(hdev, buff);
		break;
	}
	wake_up_interruptible(&hdev->debug_wait);
	kfree(buff);
}

void picolcd_debug_raw_event(struct picolcd_data *data,
		struct hid_device *hdev, struct hid_report *report,
		u8 *raw_data, int size)
{
	char *buff;

#define BUFF_SZ 256
	/* Avoid unnecessary overhead if debugfs is disabled */
	if (list_empty(&hdev->debug_list))
		return;

	buff = kmalloc(BUFF_SZ, GFP_ATOMIC);
	if (!buff)
		return;

	switch (report->id) {
	case REPORT_ERROR_CODE:
		/* 2 data bytes with affected report and error code */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_ERROR_CODE", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[2] < ARRAY_SIZE(error_codes))
			snprintf(buff, BUFF_SZ, "\tError code 0x%02x (%s) in reply to report 0x%02x\n",
					raw_data[2], error_codes[raw_data[2]], raw_data[1]);
		else
			snprintf(buff, BUFF_SZ, "\tError code 0x%02x in reply to report 0x%02x\n",
					raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_KEY_STATE:
		/* 2 data bytes with key state */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_KEY_STATE", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[1] == 0)
			snprintf(buff, BUFF_SZ, "\tNo key pressed\n");
		else if (raw_data[2] == 0)
			snprintf(buff, BUFF_SZ, "\tOne key pressed: 0x%02x (%d)\n",
					raw_data[1], raw_data[1]);
		else
			snprintf(buff, BUFF_SZ, "\tTwo keys pressed: 0x%02x (%d), 0x%02x (%d)\n",
					raw_data[1], raw_data[1], raw_data[2], raw_data[2]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_IR_DATA:
		/* Up to 20 byes of IR scancode data */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_IR_DATA", report->id, size-1);
		hid_debug_event(hdev, buff);
		if (raw_data[1] == 0) {
			snprintf(buff, BUFF_SZ, "\tUnexpectedly 0 data length\n");
			hid_debug_event(hdev, buff);
		} else if (raw_data[1] + 1 <= size) {
			snprintf(buff, BUFF_SZ, "\tData length: %d\n\tIR Data: ",
					raw_data[1]);
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+2, raw_data[1]);
			hid_debug_event(hdev, buff);
		} else {
			snprintf(buff, BUFF_SZ, "\tOverflowing data length: %d\n",
					raw_data[1]-1);
			hid_debug_event(hdev, buff);
		}
		break;
	case REPORT_EE_DATA:
		/* Data buffer in response to REPORT_EE_READ or REPORT_EE_WRITE */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_EE_DATA", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
		hid_debug_event(hdev, buff);
		if (raw_data[3] == 0) {
			snprintf(buff, BUFF_SZ, "\tNo data\n");
			hid_debug_event(hdev, buff);
		} else if (raw_data[3] + 4 <= size) {
			snprintf(buff, BUFF_SZ, "\tData: ");
			hid_debug_event(hdev, buff);
			dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			hid_debug_event(hdev, buff);
		} else {
			snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			hid_debug_event(hdev, buff);
		}
		break;
	case REPORT_MEMORY:
		/* Data buffer in response to REPORT_READ_MEMORY or REPORT_WRTIE_MEMORY */
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		switch (data->addr_sz) {
		case 2:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x\n",
					raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[3]);
			hid_debug_event(hdev, buff);
			if (raw_data[3] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[3] + 4 <= size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+4, raw_data[3]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		case 3:
			snprintf(buff, BUFF_SZ, "\tData address: 0x%02x%02x%02x\n",
					raw_data[3], raw_data[2], raw_data[1]);
			hid_debug_event(hdev, buff);
			snprintf(buff, BUFF_SZ, "\tData length: %d\n", raw_data[4]);
			hid_debug_event(hdev, buff);
			if (raw_data[4] == 0) {
				snprintf(buff, BUFF_SZ, "\tNo data\n");
			} else if (raw_data[4] + 5 <= size) {
				snprintf(buff, BUFF_SZ, "\tData: ");
				hid_debug_event(hdev, buff);
				dump_buff_as_hex(buff, BUFF_SZ, raw_data+5, raw_data[4]);
			} else {
				snprintf(buff, BUFF_SZ, "\tData overflowed\n");
			}
			break;
		default:
			snprintf(buff, BUFF_SZ, "\tNot supported\n");
		}
		hid_debug_event(hdev, buff);
		break;
	case REPORT_VERSION:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_VERSION", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tFirmware version: %d.%d\n",
				raw_data[2], raw_data[1]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_BL_ERASE_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_ERASE_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_BL_READ_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_READ_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_BL_WRITE_MEMORY:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_BL_WRITE_MEMORY", report->id, size-1);
		hid_debug_event(hdev, buff);
		/* TODO */
		break;
	case REPORT_DEVID:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_DEVID", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tSerial: 0x%02x%02x%02x%02x\n",
				raw_data[1], raw_data[2], raw_data[3], raw_data[4]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tType: 0x%02x\n",
				raw_data[5]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_SPLASH_SIZE:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_SPLASH_SIZE", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tTotal splash space: %d\n",
				(raw_data[2] << 8) | raw_data[1]);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tUsed splash space: %d\n",
				(raw_data[4] << 8) | raw_data[3]);
		hid_debug_event(hdev, buff);
		break;
	case REPORT_HOOK_VERSION:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"REPORT_HOOK_VERSION", report->id, size-1);
		hid_debug_event(hdev, buff);
		snprintf(buff, BUFF_SZ, "\tFirmware version: %d.%d\n",
				raw_data[1], raw_data[2]);
		hid_debug_event(hdev, buff);
		break;
	default:
		snprintf(buff, BUFF_SZ, "report %s (%d, size=%d)\n",
			"<unknown>", report->id, size-1);
		hid_debug_event(hdev, buff);
		break;
	}
	wake_up_interruptible(&hdev->debug_wait);
	kfree(buff);
}

void picolcd_init_devfs(struct picolcd_data *data,
		struct hid_report *eeprom_r, struct hid_report *eeprom_w,
		struct hid_report *flash_r, struct hid_report *flash_w,
		struct hid_report *reset)
{
	struct hid_device *hdev = data->hdev;

	mutex_init(&data->mutex_flash);

	/* reset */
	if (reset)
		data->debug_reset = debugfs_create_file("reset", 0600,
				hdev->debug_dir, data, &picolcd_debug_reset_fops);

	/* eeprom */
	if (eeprom_r || eeprom_w)
		data->debug_eeprom = debugfs_create_file("eeprom",
			(eeprom_w ? S_IWUSR : 0) | (eeprom_r ? S_IRUSR : 0),
			hdev->debug_dir, data, &picolcd_debug_eeprom_fops);

	/* flash */
	if (flash_r && flash_r->maxfield == 1 && flash_r->field[0]->report_size == 8)
		data->addr_sz = flash_r->field[0]->report_count - 1;
	else
		data->addr_sz = -1;
	if (data->addr_sz == 2 || data->addr_sz == 3) {
		data->debug_flash = debugfs_create_file("flash",
			(flash_w ? S_IWUSR : 0) | (flash_r ? S_IRUSR : 0),
			hdev->debug_dir, data, &picolcd_debug_flash_fops);
	} else if (flash_r || flash_w)
		hid_warn(hdev, "Unexpected FLASH access reports, please submit rdesc for review\n");
}

void picolcd_exit_devfs(struct picolcd_data *data)
{
	struct dentry *dent;

	dent = data->debug_reset;
	data->debug_reset = NULL;
	if (dent)
		debugfs_remove(dent);
	dent = data->debug_eeprom;
	data->debug_eeprom = NULL;
	if (dent)
		debugfs_remove(dent);
	dent = data->debug_flash;
	data->debug_flash = NULL;
	if (dent)
		debugfs_remove(dent);
	mutex_destroy(&data->mutex_flash);
}
