
/* MW: These areMIPI / ISYS properties, not camera function properties */
static enum sh_stream_format
css2isp_stream_format(enum ia_css_stream_format from)
{
	switch (from) {
	case IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY:
		return sh_stream_format_yuv420_legacy;
	case IA_CSS_STREAM_FORMAT_YUV420_8:
	case IA_CSS_STREAM_FORMAT_YUV420_10:
	case IA_CSS_STREAM_FORMAT_YUV420_16:
		return sh_stream_format_yuv420;
	case IA_CSS_STREAM_FORMAT_YUV422_8:
	case IA_CSS_STREAM_FORMAT_YUV422_10:
	case IA_CSS_STREAM_FORMAT_YUV422_16:
		return sh_stream_format_yuv422;
	case IA_CSS_STREAM_FORMAT_RGB_444:
	case IA_CSS_STREAM_FORMAT_RGB_555:
	case IA_CSS_STREAM_FORMAT_RGB_565:
	case IA_CSS_STREAM_FORMAT_RGB_666:
	case IA_CSS_STREAM_FORMAT_RGB_888:
		return sh_stream_format_rgb;
	case IA_CSS_STREAM_FORMAT_RAW_6:
	case IA_CSS_STREAM_FORMAT_RAW_7:
	case IA_CSS_STREAM_FORMAT_RAW_8:
	case IA_CSS_STREAM_FORMAT_RAW_10:
	case IA_CSS_STREAM_FORMAT_RAW_12:
	case IA_CSS_STREAM_FORMAT_RAW_14:
	case IA_CSS_STREAM_FORMAT_RAW_16:
		return sh_stream_format_raw;
	case IA_CSS_STREAM_FORMAT_BINARY_8:
	default:
		return sh_stream_format_raw;
	}
}