{
	return CEIL_DIV(raw_bit_depth,8);
}

/* MW: These areMIPI / ISYS properties, not camera function properties */
static enum sh_stream_format
css2isp_stream_format(enum atomisp_input_format from)
{
	switch (from) {
	case ATOMISP_INPUT_FORMAT_YUV420_8_LEGACY:
		return sh_stream_format_yuv420_legacy;
	case ATOMISP_INPUT_FORMAT_YUV420_8:
	case ATOMISP_INPUT_FORMAT_YUV420_10:
	case ATOMISP_INPUT_FORMAT_YUV420_16:
		return sh_stream_format_yuv420;
	case ATOMISP_INPUT_FORMAT_YUV422_8:
	case ATOMISP_INPUT_FORMAT_YUV422_10:
	case ATOMISP_INPUT_FORMAT_YUV422_16:
		return sh_stream_format_yuv422;
	case ATOMISP_INPUT_FORMAT_RGB_444:
	case ATOMISP_INPUT_FORMAT_RGB_555:
	case ATOMISP_INPUT_FORMAT_RGB_565:
	case ATOMISP_INPUT_FORMAT_RGB_666:
	case ATOMISP_INPUT_FORMAT_RGB_888:
		return sh_stream_format_rgb;
	case ATOMISP_INPUT_FORMAT_RAW_6:
	case ATOMISP_INPUT_FORMAT_RAW_7:
	case ATOMISP_INPUT_FORMAT_RAW_8:
	case ATOMISP_INPUT_FORMAT_RAW_10:
	case ATOMISP_INPUT_FORMAT_RAW_12:
	case ATOMISP_INPUT_FORMAT_RAW_14:
	case ATOMISP_INPUT_FORMAT_RAW_16:
		return sh_stream_format_raw;
	case ATOMISP_INPUT_FORMAT_BINARY_8:
	default:
		return sh_stream_format_raw;
	}
}