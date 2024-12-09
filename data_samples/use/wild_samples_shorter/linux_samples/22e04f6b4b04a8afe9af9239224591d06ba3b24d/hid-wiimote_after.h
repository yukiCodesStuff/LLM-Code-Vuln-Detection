	WIIMOTE_EXT_CLASSIC_CONTROLLER,
	WIIMOTE_EXT_BALANCE_BOARD,
	WIIMOTE_EXT_PRO_CONTROLLER,
	WIIMOTE_EXT_GUITAR_HERO_DRUMS,
	WIIMOTE_EXT_GUITAR_HERO_GUITAR,
	WIIMOTE_EXT_NUM,
};

enum wiimote_mptype {

	/* calibration data */
	__u16 calib_bboard[4][3];
	__u8 pressure_drums[7];
};

struct wiimote_data {
	struct hid_device *hdev;