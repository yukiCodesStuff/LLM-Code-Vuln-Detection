/* Define for MXT_GEN_COMMAND_T6 */
#define MXT_BOOT_VALUE		0xa5
#define MXT_BACKUP_VALUE	0x55
#define MXT_BACKUP_TIME		50	/* msec */
#define MXT_RESET_TIME		200	/* msec */

#define MXT_FWRESET_TIME	175	/* msec */

/* MXT_SPT_GPIOPWM_T19 field */
#define MXT_GPIO0_MASK		0x04
#define MXT_GPIO1_MASK		0x08
#define MXT_GPIO2_MASK		0x10
#define MXT_GPIO3_MASK		0x20

/* Command to unlock bootloader */
#define MXT_UNLOCK_CMD_MSB	0xaa
#define MXT_UNLOCK_CMD_LSB	0xdc

/* Touchscreen absolute values */
#define MXT_MAX_AREA		0xff

#define MXT_PIXELS_PER_MM	20

struct mxt_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	const struct mxt_platform_data *pdata;
	struct mxt_object *object_table;
	struct mxt_info info;
	bool is_tp;

	unsigned int irq;
	unsigned int max_x;
	unsigned int max_y;

	u8 T6_reportid;
	u8 T9_reportid_min;
	u8 T9_reportid_max;
	u8 T19_reportid;
};

static bool mxt_object_readable(unsigned int type)
{
	return mxt_write_reg(data->client, reg + offset, val);
}

static void mxt_input_button(struct mxt_data *data, struct mxt_message *message)
{
	struct input_dev *input = data->input_dev;
	bool button;
	int i;

	/* Active-low switch */
	for (i = 0; i < MXT_NUM_GPIO; i++) {
		if (data->pdata->key_map[i] == KEY_RESERVED)
			continue;
		button = !(message->message[0] & MXT_GPIO0_MASK << i);
		input_report_key(input, data->pdata->key_map[i], button);
	}
}

static void mxt_input_touchevent(struct mxt_data *data,
				      struct mxt_message *message, int id)
{
	struct device *dev = &data->client->dev;
			int id = reportid - data->T9_reportid_min;
			mxt_input_touchevent(data, &message, id);
			update_input = true;
		} else if (message.reportid == data->T19_reportid) {
			mxt_input_button(data, &message);
			update_input = true;
		} else {
			mxt_dump_message(dev, &message);
		}
	} while (reportid != 0xff);
			data->T9_reportid_min = min_id;
			data->T9_reportid_max = max_id;
			break;
		case MXT_SPT_GPIOPWM_T19:
			data->T19_reportid = min_id;
			break;
		}
	}

	return 0;
	data->T6_reportid = 0;
	data->T9_reportid_min = 0;
	data->T9_reportid_max = 0;
	data->T19_reportid = 0;
}

static int mxt_initialize(struct mxt_data *data)
{
		goto err_free_mem;
	}

	data->is_tp = pdata && pdata->is_tp;

	input_dev->name = (data->is_tp) ? "Atmel maXTouch Touchpad" :
					  "Atmel maXTouch Touchscreen";
	snprintf(data->phys, sizeof(data->phys), "i2c-%u-%04x/input0",
		 client->adapter->nr, client->addr);

	input_dev->phys = data->phys;

	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

	if (data->is_tp) {
		int i;
		__set_bit(INPUT_PROP_POINTER, input_dev->propbit);
		__set_bit(INPUT_PROP_BUTTONPAD, input_dev->propbit);

		for (i = 0; i < MXT_NUM_GPIO; i++)
			if (pdata->key_map[i] != KEY_RESERVED)
				__set_bit(pdata->key_map[i], input_dev->keybit);

		__set_bit(BTN_TOOL_FINGER, input_dev->keybit);
		__set_bit(BTN_TOOL_DOUBLETAP, input_dev->keybit);
		__set_bit(BTN_TOOL_TRIPLETAP, input_dev->keybit);
		__set_bit(BTN_TOOL_QUADTAP, input_dev->keybit);
		__set_bit(BTN_TOOL_QUINTTAP, input_dev->keybit);

		input_abs_set_res(input_dev, ABS_X, MXT_PIXELS_PER_MM);
		input_abs_set_res(input_dev, ABS_Y, MXT_PIXELS_PER_MM);
		input_abs_set_res(input_dev, ABS_MT_POSITION_X,
				  MXT_PIXELS_PER_MM);
		input_abs_set_res(input_dev, ABS_MT_POSITION_Y,
				  MXT_PIXELS_PER_MM);
	}

	/* For single touch */
	input_set_abs_params(input_dev, ABS_X,
			     0, data->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_Y,
static const struct i2c_device_id mxt_id[] = {
	{ "qt602240_ts", 0 },
	{ "atmel_mxt_ts", 0 },
	{ "atmel_mxt_tp", 0 },
	{ "mXT224", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mxt_id);