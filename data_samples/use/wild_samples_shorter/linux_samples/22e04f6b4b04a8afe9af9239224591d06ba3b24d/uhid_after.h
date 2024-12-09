	UHID_OPEN,
	UHID_CLOSE,
	UHID_OUTPUT,
	UHID_OUTPUT_EV,			/* obsolete! */
	UHID_INPUT,
	UHID_FEATURE,
	UHID_FEATURE_ANSWER,
};
	__u8 rtype;
} __attribute__((__packed__));

/* Obsolete! Newer kernels will no longer send these events but instead convert
 * it into raw output reports via UHID_OUTPUT. */
struct uhid_output_ev_req {
	__u16 type;
	__u16 code;
	__s32 value;