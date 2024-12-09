static const struct wiimod_ops wiimod_pro = {
	.flags = WIIMOD_FLAG_EXT16,
	.arg = 0,
	.probe = wiimod_pro_probe,
	.remove = wiimod_pro_remove,
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
 * don't allow MP hotplugging.
 */

static int wiimod_builtin_mp_probe(const struct wiimod_ops *ops,
				   struct wiimote_data *wdata)
{
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags |= WIIPROTO_FLAG_BUILTIN_MP;
	spin_unlock_irqrestore(&wdata->state.lock, flags);

	return 0;
}

static void wiimod_builtin_mp_remove(const struct wiimod_ops *ops,
				     struct wiimote_data *wdata)
{
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags |= WIIPROTO_FLAG_BUILTIN_MP;
	spin_unlock_irqrestore(&wdata->state.lock, flags);
}

static const struct wiimod_ops wiimod_builtin_mp = {
	.flags = 0,
	.arg = 0,
	.probe = wiimod_builtin_mp_probe,
	.remove = wiimod_builtin_mp_remove,
};

/*
 * No Motion Plus
 * This module simply sets the WIIPROTO_FLAG_NO_MP protocol flag which
 * disables motion-plus. This is needed for devices that advertise this but we
 * don't know how to use it (or whether it is actually present).
 */

static int wiimod_no_mp_probe(const struct wiimod_ops *ops,
			      struct wiimote_data *wdata)
{
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags |= WIIPROTO_FLAG_NO_MP;
	spin_unlock_irqrestore(&wdata->state.lock, flags);

	return 0;
}

static void wiimod_no_mp_remove(const struct wiimod_ops *ops,
				struct wiimote_data *wdata)
{
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags |= WIIPROTO_FLAG_NO_MP;
	spin_unlock_irqrestore(&wdata->state.lock, flags);
}

static const struct wiimod_ops wiimod_no_mp = {
	.flags = 0,
	.arg = 0,
	.probe = wiimod_no_mp_probe,
	.remove = wiimod_no_mp_remove,
};

/*
 * Motion Plus
 * The Motion Plus extension provides rotation sensors (gyro) as a small
 * extension device for Wii Remotes. Many devices have them built-in so
 * you cannot see them from the outside.
 * Motion Plus extensions are special because they are on a separate extension
 * port and allow other extensions to be used simultaneously. This is all
 * handled by the Wiimote Core so we don't have to deal with it.
 */

static void wiimod_mp_in_mp(struct wiimote_data *wdata, const __u8 *ext)
{
	__s32 x, y, z;

	/*        |   8    7    6    5    4    3 |  2  |  1  |
	 *   -----+------------------------------+-----+-----+
	 *    1   |               Yaw Speed <7:0>            |
	 *    2   |              Roll Speed <7:0>            |
	 *    3   |             Pitch Speed <7:0>            |
	 *   -----+------------------------------+-----+-----+
	 *    4   |       Yaw Speed <13:8>       | Yaw |Pitch|
	 *   -----+------------------------------+-----+-----+
	 *    5   |      Roll Speed <13:8>       |Roll | Ext |
	 *   -----+------------------------------+-----+-----+
	 *    6   |     Pitch Speed <13:8>       |  1  |  0  |
	 *   -----+------------------------------+-----+-----+
	 * The single bits Yaw, Roll, Pitch in the lower right corner specify
	 * whether the wiimote is rotating fast (0) or slow (1). Speed for slow
	 * roation is 440 deg/s and for fast rotation 2000 deg/s. To get a
	 * linear scale we multiply by 2000/440 = ~4.5454 which is 18 for fast
	 * and 9 for slow.
	 * If the wiimote is not rotating the sensor reports 2^13 = 8192.
	 * Ext specifies whether an extension is connected to the motionp.
	 * which is parsed by wiimote-core.
	 */

	x = ext[0];
	y = ext[1];
	z = ext[2];

	x |= (((__u16)ext[3]) << 6) & 0xff00;
	y |= (((__u16)ext[4]) << 6) & 0xff00;
	z |= (((__u16)ext[5]) << 6) & 0xff00;

	x -= 8192;
	y -= 8192;
	z -= 8192;

	if (!(ext[3] & 0x02))
		x *= 18;
	else
		x *= 9;
	if (!(ext[4] & 0x02))
		y *= 18;
	else
		y *= 9;
	if (!(ext[3] & 0x01))
		z *= 18;
	else
		z *= 9;

	input_report_abs(wdata->mp, ABS_RX, x);
	input_report_abs(wdata->mp, ABS_RY, y);
	input_report_abs(wdata->mp, ABS_RZ, z);
	input_sync(wdata->mp);
}

static int wiimod_mp_open(struct input_dev *dev)
{
	struct wiimote_data *wdata = input_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags |= WIIPROTO_FLAG_MP_USED;
	wiiproto_req_drm(wdata, WIIPROTO_REQ_NULL);
	__wiimote_schedule(wdata);
	spin_unlock_irqrestore(&wdata->state.lock, flags);

	return 0;
}

static void wiimod_mp_close(struct input_dev *dev)
{
	struct wiimote_data *wdata = input_get_drvdata(dev);
	unsigned long flags;

	spin_lock_irqsave(&wdata->state.lock, flags);
	wdata->state.flags &= ~WIIPROTO_FLAG_MP_USED;
	wiiproto_req_drm(wdata, WIIPROTO_REQ_NULL);
	__wiimote_schedule(wdata);
	spin_unlock_irqrestore(&wdata->state.lock, flags);
}

static int wiimod_mp_probe(const struct wiimod_ops *ops,
			   struct wiimote_data *wdata)
{
	int ret;

	wdata->mp = input_allocate_device();
	if (!wdata->mp)
		return -ENOMEM;

	input_set_drvdata(wdata->mp, wdata);
	wdata->mp->open = wiimod_mp_open;
	wdata->mp->close = wiimod_mp_close;
	wdata->mp->dev.parent = &wdata->hdev->dev;
	wdata->mp->id.bustype = wdata->hdev->bus;
	wdata->mp->id.vendor = wdata->hdev->vendor;
	wdata->mp->id.product = wdata->hdev->product;
	wdata->mp->id.version = wdata->hdev->version;
	wdata->mp->name = WIIMOTE_NAME " Motion Plus";

	set_bit(EV_ABS, wdata->mp->evbit);
	set_bit(ABS_RX, wdata->mp->absbit);
	set_bit(ABS_RY, wdata->mp->absbit);
	set_bit(ABS_RZ, wdata->mp->absbit);
	input_set_abs_params(wdata->mp,
			     ABS_RX, -16000, 16000, 4, 8);
	input_set_abs_params(wdata->mp,
			     ABS_RY, -16000, 16000, 4, 8);
	input_set_abs_params(wdata->mp,
			     ABS_RZ, -16000, 16000, 4, 8);

	ret = input_register_device(wdata->mp);
	if (ret)
		goto err_free;

	return 0;

err_free:
	input_free_device(wdata->mp);
	wdata->mp = NULL;
	return ret;
}

static void wiimod_mp_remove(const struct wiimod_ops *ops,
			     struct wiimote_data *wdata)
{
	if (!wdata->mp)
		return;

	input_unregister_device(wdata->mp);
	wdata->mp = NULL;
}

const struct wiimod_ops wiimod_mp = {
	.flags = 0,
	.arg = 0,
	.probe = wiimod_mp_probe,
	.remove = wiimod_mp_remove,
	.in_mp = wiimod_mp_in_mp,
};

/* module table */

static const struct wiimod_ops wiimod_dummy;

const struct wiimod_ops *wiimod_table[WIIMOD_NUM] = {
	[WIIMOD_KEYS] = &wiimod_keys,
	[WIIMOD_RUMBLE] = &wiimod_rumble,
	[WIIMOD_BATTERY] = &wiimod_battery,
	[WIIMOD_LED1] = &wiimod_leds[0],
	[WIIMOD_LED2] = &wiimod_leds[1],
	[WIIMOD_LED3] = &wiimod_leds[2],
	[WIIMOD_LED4] = &wiimod_leds[3],
	[WIIMOD_ACCEL] = &wiimod_accel,
	[WIIMOD_IR] = &wiimod_ir,
	[WIIMOD_BUILTIN_MP] = &wiimod_builtin_mp,
	[WIIMOD_NO_MP] = &wiimod_no_mp,
};

const struct wiimod_ops *wiimod_ext_table[WIIMOTE_EXT_NUM] = {
	[WIIMOTE_EXT_NONE] = &wiimod_dummy,
	[WIIMOTE_EXT_UNKNOWN] = &wiimod_dummy,
	[WIIMOTE_EXT_NUNCHUK] = &wiimod_nunchuk,
	[WIIMOTE_EXT_CLASSIC_CONTROLLER] = &wiimod_classic,
	[WIIMOTE_EXT_BALANCE_BOARD] = &wiimod_bboard,
	[WIIMOTE_EXT_PRO_CONTROLLER] = &wiimod_pro,
	[WIIMOTE_EXT_GUITAR_HERO_DRUMS] = &wiimod_drums,
	[WIIMOTE_EXT_GUITAR_HERO_GUITAR] = &wiimod_guitar,
};
const struct wiimod_ops *wiimod_ext_table[WIIMOTE_EXT_NUM] = {
	[WIIMOTE_EXT_NONE] = &wiimod_dummy,
	[WIIMOTE_EXT_UNKNOWN] = &wiimod_dummy,
	[WIIMOTE_EXT_NUNCHUK] = &wiimod_nunchuk,
	[WIIMOTE_EXT_CLASSIC_CONTROLLER] = &wiimod_classic,
	[WIIMOTE_EXT_BALANCE_BOARD] = &wiimod_bboard,
	[WIIMOTE_EXT_PRO_CONTROLLER] = &wiimod_pro,
	[WIIMOTE_EXT_GUITAR_HERO_DRUMS] = &wiimod_drums,
	[WIIMOTE_EXT_GUITAR_HERO_GUITAR] = &wiimod_guitar,
};