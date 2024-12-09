 */
enum vfl_devnode_type {
	VFL_TYPE_GRABBER	= 0,
	VFL_TYPE_VBI		= 1,
	VFL_TYPE_RADIO		= 2,
	VFL_TYPE_SUBDEV		= 3,
	VFL_TYPE_SDR		= 4,
	VFL_TYPE_TOUCH		= 5,
};
#define VFL_TYPE_MAX VFL_TYPE_TOUCH

/**
 * enum  vfl_direction - Identifies if a &struct video_device corresponds
 *	to a receiver, a transmitter or a mem-to-mem device.