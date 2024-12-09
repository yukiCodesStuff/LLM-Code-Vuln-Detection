	.in_ext = wiimod_pro_in_ext,
};

/*
 * Drums
 * Guitar-Hero, Rock-Band and other games came bundled with drums which can
 * be plugged as extension to a Wiimote. Drum-reports are still not entirely
 * figured out, but the most important information is known.
 * We create a separate device for drums and report all information via this
 * input device.
 */

static inline void wiimod_drums_report_pressure(struct wiimote_data *wdata,
						__u8 none, __u8 which,
						__u8 pressure, __u8 onoff,
						__u8 *store, __u16 code,
						__u8 which_code)
{
	static const __u8 default_pressure = 3;

	if (!none && which == which_code) {
		*store = pressure;
		input_report_abs(wdata->extension.input, code, *store);
	} else if (onoff != !!*store) {
		*store = onoff ? default_pressure : 0;
		input_report_abs(wdata->extension.input, code, *store);
	}
}

static void wiimod_drums_in_ext(struct wiimote_data *wdata, const __u8 *ext)
{
	__u8 pressure, which, none, hhp, sx, sy;
	__u8 o, r, y, g, b, bass, bm, bp;

	/*   Byte |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    1   |  0  |  0  |              SX <5:0>             |
	 *    2   |  0  |  0  |              SY <5:0>             |
	 *   -----+-----+-----+-----------------------------+-----+
	 *    3   | HPP | NON |         WHICH <5:1>         |  ?  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    4   |   SOFT <7:5>    |  0  |  1  |  1  |  0  |  ?  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    5   |  ?  |  1  |  1  | B-  |  1  | B+  |  1  |  ?  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    6   |  O  |  R  |  Y  |  G  |  B  | BSS |  1  |  1  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 * All buttons are 0 if pressed
	 *
	 * With Motion+ enabled, the following bits will get invalid:
	 *   Byte |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    1   |  0  |  0  |              SX <5:1>       |XXXXX|
	 *    2   |  0  |  0  |              SY <5:1>       |XXXXX|
	 *   -----+-----+-----+-----------------------------+-----+
	 *    3   | HPP | NON |         WHICH <5:1>         |  ?  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    4   |   SOFT <7:5>    |  0  |  1  |  1  |  0  |  ?  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    5   |  ?  |  1  |  1  | B-  |  1  | B+  |  1  |XXXXX|
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    6   |  O  |  R  |  Y  |  G  |  B  | BSS |XXXXX|XXXXX|
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	pressure = 7 - (ext[3] >> 5);
	which = (ext[2] >> 1) & 0x1f;
	none = !!(ext[2] & 0x40);
	hhp = !(ext[2] & 0x80);
	sx = ext[0] & 0x3f;
	sy = ext[1] & 0x3f;
	o = !(ext[5] & 0x80);
	r = !(ext[5] & 0x40);
	y = !(ext[5] & 0x20);
	g = !(ext[5] & 0x10);
	b = !(ext[5] & 0x08);
	bass = !(ext[5] & 0x04);
	bm = !(ext[4] & 0x10);
	bp = !(ext[4] & 0x04);

	wiimod_drums_report_pressure(wdata, none, which, pressure,
				     o, &wdata->state.pressure_drums[0],
				     ABS_CYMBAL_RIGHT, 0x0e);
	wiimod_drums_report_pressure(wdata, none, which, pressure,
				     r, &wdata->state.pressure_drums[1],
				     ABS_TOM_LEFT, 0x19);
	wiimod_drums_report_pressure(wdata, none, which, pressure,
				     y, &wdata->state.pressure_drums[2],
				     ABS_CYMBAL_LEFT, 0x11);
	wiimod_drums_report_pressure(wdata, none, which, pressure,
				     g, &wdata->state.pressure_drums[3],
				     ABS_TOM_FAR_RIGHT, 0x12);
	wiimod_drums_report_pressure(wdata, none, which, pressure,
				     b, &wdata->state.pressure_drums[4],
				     ABS_TOM_RIGHT, 0x0f);

	/* Bass shares pressure with hi-hat (set via hhp) */
	wiimod_drums_report_pressure(wdata, none, hhp ? 0xff : which, pressure,
				     bass, &wdata->state.pressure_drums[5],
				     ABS_BASS, 0x1b);
	/* Hi-hat has no on/off values, just pressure. Force to off/0. */
	wiimod_drums_report_pressure(wdata, none, hhp ? which : 0xff, pressure,
				     0, &wdata->state.pressure_drums[6],
				     ABS_HI_HAT, 0x0e);

	input_report_abs(wdata->extension.input, ABS_X, sx - 0x20);
	input_report_abs(wdata->extension.input, ABS_Y, sy - 0x20);

	input_report_key(wdata->extension.input, BTN_START, bp);
	input_report_key(wdata->extension.input, BTN_SELECT, bm);

	input_sync(wdata->extension.input);
}

static int wiimod_drums_open(struct input_dev *dev)
{
	struct wiimote_data *wdata = input_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags |= WIIPROTO_FLAG_EXT_USED;
	wiiproto_req_drm(wdata, WIIPROTO_REQ_NULL);
	spin_unlock_irqrestore(&wdata->state.lock, flags);

	return 0;
}

static void wiimod_drums_close(struct input_dev *dev)
{
	struct wiimote_data *wdata = input_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags &= ~WIIPROTO_FLAG_EXT_USED;
	wiiproto_req_drm(wdata, WIIPROTO_REQ_NULL);
	spin_unlock_irqrestore(&wdata->state.lock, flags);
}

static int wiimod_drums_probe(const struct wiimod_ops *ops,
			      struct wiimote_data *wdata)
{
	int ret;

	wdata->extension.input = input_allocate_device();
	if (!wdata->extension.input)
		return -ENOMEM;

	input_set_drvdata(wdata->extension.input, wdata);
	wdata->extension.input->open = wiimod_drums_open;
	wdata->extension.input->close = wiimod_drums_close;
	wdata->extension.input->dev.parent = &wdata->hdev->dev;
	wdata->extension.input->id.bustype = wdata->hdev->bus;
	wdata->extension.input->id.vendor = wdata->hdev->vendor;
	wdata->extension.input->id.product = wdata->hdev->product;
	wdata->extension.input->id.version = wdata->hdev->version;
	wdata->extension.input->name = WIIMOTE_NAME " Drums";

	set_bit(EV_KEY, wdata->extension.input->evbit);
	set_bit(BTN_START, wdata->extension.input->keybit);
	set_bit(BTN_SELECT, wdata->extension.input->keybit);

	set_bit(EV_ABS, wdata->extension.input->evbit);
	set_bit(ABS_X, wdata->extension.input->absbit);
	set_bit(ABS_Y, wdata->extension.input->absbit);
	set_bit(ABS_TOM_LEFT, wdata->extension.input->absbit);
	set_bit(ABS_TOM_RIGHT, wdata->extension.input->absbit);
	set_bit(ABS_TOM_FAR_RIGHT, wdata->extension.input->absbit);
	set_bit(ABS_CYMBAL_LEFT, wdata->extension.input->absbit);
	set_bit(ABS_CYMBAL_RIGHT, wdata->extension.input->absbit);
	set_bit(ABS_BASS, wdata->extension.input->absbit);
	set_bit(ABS_HI_HAT, wdata->extension.input->absbit);
	input_set_abs_params(wdata->extension.input,
			     ABS_X, -32, 31, 1, 1);
	input_set_abs_params(wdata->extension.input,
			     ABS_Y, -32, 31, 1, 1);
	input_set_abs_params(wdata->extension.input,
			     ABS_TOM_LEFT, 0, 7, 0, 0);
	input_set_abs_params(wdata->extension.input,
			     ABS_TOM_RIGHT, 0, 7, 0, 0);
	input_set_abs_params(wdata->extension.input,
			     ABS_TOM_FAR_RIGHT, 0, 7, 0, 0);
	input_set_abs_params(wdata->extension.input,
			     ABS_CYMBAL_LEFT, 0, 7, 0, 0);
	input_set_abs_params(wdata->extension.input,
			     ABS_CYMBAL_RIGHT, 0, 7, 0, 0);
	input_set_abs_params(wdata->extension.input,
			     ABS_BASS, 0, 7, 0, 0);
	input_set_abs_params(wdata->extension.input,
			     ABS_HI_HAT, 0, 7, 0, 0);

	ret = input_register_device(wdata->extension.input);
	if (ret)
		goto err_free;

	return 0;

err_free:
	input_free_device(wdata->extension.input);
	wdata->extension.input = NULL;
	return ret;
}

static void wiimod_drums_remove(const struct wiimod_ops *ops,
				struct wiimote_data *wdata)
{
	if (!wdata->extension.input)
		return;

	input_unregister_device(wdata->extension.input);
	wdata->extension.input = NULL;
}

static const struct wiimod_ops wiimod_drums = {
	.flags = 0,
	.arg = 0,
	.probe = wiimod_drums_probe,
	.remove = wiimod_drums_remove,
	.in_ext = wiimod_drums_in_ext,
};

/*
 * Guitar
 * Guitar-Hero, Rock-Band and other games came bundled with guitars which can
 * be plugged as extension to a Wiimote.
 * We create a separate device for guitars and report all information via this
 * input device.
 */

static void wiimod_guitar_in_ext(struct wiimote_data *wdata, const __u8 *ext)
{
	__u8 sx, sy, tb, wb, bd, bm, bp, bo, br, bb, bg, by, bu;

	/*   Byte |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    1   |  0  |  0  |              SX <5:0>             |
	 *    2   |  0  |  0  |              SY <5:0>             |
	 *   -----+-----+-----+-----+-----------------------------+
	 *    3   |  0  |  0  |  0  |      TB <4:0>               |
	 *   -----+-----+-----+-----+-----------------------------+
	 *    4   |  0  |  0  |  0  |      WB <4:0>               |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    5   |  1  | BD  |  1  | B-  |  1  | B+  |  1  |  1  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    6   | BO  | BR  | BB  | BG  | BY  |  1  |  1  | BU  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 * All buttons are 0 if pressed
	 *
	 * With Motion+ enabled, the following bits will get invalid:
	 *   Byte |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    1   |  0  |  0  |              SX <5:1>       |XXXXX|
	 *    2   |  0  |  0  |              SY <5:1>       |XXXXX|
	 *   -----+-----+-----+-----+-----------------------+-----+
	 *    3   |  0  |  0  |  0  |      TB <4:0>               |
	 *   -----+-----+-----+-----+-----------------------------+
	 *    4   |  0  |  0  |  0  |      WB <4:0>               |
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    5   |  1  | BD  |  1  | B-  |  1  | B+  |  1  |XXXXX|
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 *    6   | BO  | BR  | BB  | BG  | BY  |  1  |XXXXX|XXXXX|
	 *   -----+-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	sx = ext[0] & 0x3f;
	sy = ext[1] & 0x3f;
	tb = ext[2] & 0x1f;
	wb = ext[3] & 0x1f;
	bd = !(ext[4] & 0x40);
	bm = !(ext[4] & 0x10);
	bp = !(ext[4] & 0x04);
	bo = !(ext[5] & 0x80);
	br = !(ext[5] & 0x40);
	bb = !(ext[5] & 0x20);
	bg = !(ext[5] & 0x10);
	by = !(ext[5] & 0x08);
	bu = !(ext[5] & 0x01);

	input_report_abs(wdata->extension.input, ABS_X, sx - 0x20);
	input_report_abs(wdata->extension.input, ABS_Y, sy - 0x20);
	input_report_abs(wdata->extension.input, ABS_FRET_BOARD, tb);
	input_report_abs(wdata->extension.input, ABS_WHAMMY_BAR, wb - 0x10);

	input_report_key(wdata->extension.input, BTN_MODE, bm);
	input_report_key(wdata->extension.input, BTN_START, bp);
	input_report_key(wdata->extension.input, BTN_STRUM_BAR_UP, bu);
	input_report_key(wdata->extension.input, BTN_STRUM_BAR_DOWN, bd);
	input_report_key(wdata->extension.input, BTN_FRET_FAR_UP, bg);
	input_report_key(wdata->extension.input, BTN_FRET_UP, br);
	input_report_key(wdata->extension.input, BTN_FRET_MID, by);
	input_report_key(wdata->extension.input, BTN_FRET_LOW, bb);
	input_report_key(wdata->extension.input, BTN_FRET_FAR_LOW, bo);

	input_sync(wdata->extension.input);
}

static int wiimod_guitar_open(struct input_dev *dev)
{
	struct wiimote_data *wdata = input_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags |= WIIPROTO_FLAG_EXT_USED;
	wiiproto_req_drm(wdata, WIIPROTO_REQ_NULL);
	spin_unlock_irqrestore(&wdata->state.lock, flags);

	return 0;
}

static void wiimod_guitar_close(struct input_dev *dev)
{
	struct wiimote_data *wdata = input_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags &= ~WIIPROTO_FLAG_EXT_USED;
	wiiproto_req_drm(wdata, WIIPROTO_REQ_NULL);
	spin_unlock_irqrestore(&wdata->state.lock, flags);
}

static int wiimod_guitar_probe(const struct wiimod_ops *ops,
			       struct wiimote_data *wdata)
{
	int ret;

	wdata->extension.input = input_allocate_device();
	if (!wdata->extension.input)
		return -ENOMEM;

	input_set_drvdata(wdata->extension.input, wdata);
	wdata->extension.input->open = wiimod_guitar_open;
	wdata->extension.input->close = wiimod_guitar_close;
	wdata->extension.input->dev.parent = &wdata->hdev->dev;
	wdata->extension.input->id.bustype = wdata->hdev->bus;
	wdata->extension.input->id.vendor = wdata->hdev->vendor;
	wdata->extension.input->id.product = wdata->hdev->product;
	wdata->extension.input->id.version = wdata->hdev->version;
	wdata->extension.input->name = WIIMOTE_NAME " Guitar";

	set_bit(EV_KEY, wdata->extension.input->evbit);
	set_bit(BTN_MODE, wdata->extension.input->keybit);
	set_bit(BTN_START, wdata->extension.input->keybit);
	set_bit(BTN_FRET_FAR_UP, wdata->extension.input->keybit);
	set_bit(BTN_FRET_UP, wdata->extension.input->keybit);
	set_bit(BTN_FRET_MID, wdata->extension.input->keybit);
	set_bit(BTN_FRET_LOW, wdata->extension.input->keybit);
	set_bit(BTN_FRET_FAR_LOW, wdata->extension.input->keybit);
	set_bit(BTN_STRUM_BAR_UP, wdata->extension.input->keybit);
	set_bit(BTN_STRUM_BAR_DOWN, wdata->extension.input->keybit);

	set_bit(EV_ABS, wdata->extension.input->evbit);
	set_bit(ABS_X, wdata->extension.input->absbit);
	set_bit(ABS_Y, wdata->extension.input->absbit);
	set_bit(ABS_FRET_BOARD, wdata->extension.input->absbit);
	set_bit(ABS_WHAMMY_BAR, wdata->extension.input->absbit);
	input_set_abs_params(wdata->extension.input,
			     ABS_X, -32, 31, 1, 1);
	input_set_abs_params(wdata->extension.input,
			     ABS_Y, -32, 31, 1, 1);
	input_set_abs_params(wdata->extension.input,
			     ABS_FRET_BOARD, 0, 0x1f, 1, 1);
	input_set_abs_params(wdata->extension.input,
			     ABS_WHAMMY_BAR, 0, 0x0f, 1, 1);

	ret = input_register_device(wdata->extension.input);
	if (ret)
		goto err_free;

	return 0;

err_free:
	input_free_device(wdata->extension.input);
	wdata->extension.input = NULL;
	return ret;
}

static void wiimod_guitar_remove(const struct wiimod_ops *ops,
				 struct wiimote_data *wdata)
{
	if (!wdata->extension.input)
		return;

	input_unregister_device(wdata->extension.input);
	wdata->extension.input = NULL;
}

static const struct wiimod_ops wiimod_guitar = {
	.flags = 0,
	.arg = 0,
	.probe = wiimod_guitar_probe,
	.remove = wiimod_guitar_remove,
	.in_ext = wiimod_guitar_in_ext,
};

/*
 * Builtin Motion Plus
 * This module simply sets the WIIPROTO_FLAG_BUILTIN_MP protocol flag which
 * disables polling for Motion-Plus. This should be set only for devices which
	[WIIMOTE_EXT_CLASSIC_CONTROLLER] = &wiimod_classic,
	[WIIMOTE_EXT_BALANCE_BOARD] = &wiimod_bboard,
	[WIIMOTE_EXT_PRO_CONTROLLER] = &wiimod_pro,
	[WIIMOTE_EXT_GUITAR_HERO_DRUMS] = &wiimod_drums,
	[WIIMOTE_EXT_GUITAR_HERO_GUITAR] = &wiimod_guitar,
};