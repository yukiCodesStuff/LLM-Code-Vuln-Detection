	ATOMISP_INPUT_FORMAT_USER_DEF8,  /* User defined 8-bit data type 8 */
};

#define N_ATOMISP_INPUT_FORMAT (ATOMISP_INPUT_FORMAT_USER_DEF8 + 1)



enum intel_v4l2_subdev_type {
	RAW_CAMERA = 1,
	SOC_CAMERA = 2,
	CAMERA_MOTOR = 3,