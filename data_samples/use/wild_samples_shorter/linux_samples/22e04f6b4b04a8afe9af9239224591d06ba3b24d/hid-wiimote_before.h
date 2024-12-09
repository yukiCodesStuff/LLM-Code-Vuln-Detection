	WIIMOTE_EXT_CLASSIC_CONTROLLER,
	WIIMOTE_EXT_BALANCE_BOARD,
	WIIMOTE_EXT_PRO_CONTROLLER,
	WIIMOTE_EXT_NUM,
};

enum wiimote_mptype {

	/* calibration data */
	__u16 calib_bboard[4][3];
};

struct wiimote_data {
	struct hid_device *hdev;