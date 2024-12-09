 */
enum vfl_devnode_type {
	VFL_TYPE_GRABBER	= 0,
	VFL_TYPE_VBI,
	VFL_TYPE_RADIO,
	VFL_TYPE_SUBDEV,
	VFL_TYPE_SDR,
	VFL_TYPE_TOUCH,
	VFL_TYPE_MAX /* Shall be the last one */
};

/**
 * enum  vfl_direction - Identifies if a &struct video_device corresponds
 *	to a receiver, a transmitter or a mem-to-mem device.