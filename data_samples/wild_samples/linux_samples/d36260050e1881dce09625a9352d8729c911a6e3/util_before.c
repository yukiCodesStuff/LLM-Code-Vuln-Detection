	switch (in_err) {
		case 0:
			out_err = IA_CSS_SUCCESS;
			break;
		case EINVAL:
			out_err = IA_CSS_ERR_INVALID_ARGUMENTS;
			break;
		case ENODATA:
			out_err = IA_CSS_ERR_QUEUE_IS_EMPTY;
			break;
		case ENOSYS:
		case ENOTSUP:
			out_err = IA_CSS_ERR_INTERNAL_ERROR;
			break;
		case ENOBUFS:
			out_err = IA_CSS_ERR_QUEUE_IS_FULL;
			break;
		default:
			out_err = IA_CSS_ERR_INTERNAL_ERROR;
			break;
	}
	return out_err;
}

/* MW: Table look-up ??? */
unsigned int ia_css_util_input_format_bpp(
	enum ia_css_stream_format format,
	bool two_ppc)
{
	unsigned int rval = 0;
	switch (format) {
	case IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY:
	case IA_CSS_STREAM_FORMAT_YUV420_8:
	case IA_CSS_STREAM_FORMAT_YUV422_8:
	case IA_CSS_STREAM_FORMAT_RGB_888:
	case IA_CSS_STREAM_FORMAT_RAW_8:
	case IA_CSS_STREAM_FORMAT_BINARY_8:
	case IA_CSS_STREAM_FORMAT_EMBEDDED:
		rval = 8;
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_10:
	case IA_CSS_STREAM_FORMAT_YUV422_10:
	case IA_CSS_STREAM_FORMAT_RAW_10:
		rval = 10;
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_16:
	case IA_CSS_STREAM_FORMAT_YUV422_16:
		rval = 16;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_444:
		rval = 4;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_555:
		rval = 5;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_565:
		rval = 65;
		break;
	case IA_CSS_STREAM_FORMAT_RGB_666:
	case IA_CSS_STREAM_FORMAT_RAW_6:
		rval = 6;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_7:
		rval = 7;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_12:
		rval = 12;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_14:
		if (two_ppc)
			rval = 14;
		else
			rval = 12;
		break;
	case IA_CSS_STREAM_FORMAT_RAW_16:
		if (two_ppc)
			rval = 16;
		else
			rval = 12;
		break;
	default:
		rval = 0;
		break;

	}
	return rval;
}
{
	return IS_EVEN(resolution.height) && IS_EVEN(resolution.width);
}

#endif
bool ia_css_util_is_input_format_raw(enum ia_css_stream_format format)
{
	return ((format == IA_CSS_STREAM_FORMAT_RAW_6) ||
		(format == IA_CSS_STREAM_FORMAT_RAW_7) ||
		(format == IA_CSS_STREAM_FORMAT_RAW_8) ||
		(format == IA_CSS_STREAM_FORMAT_RAW_10) ||
		(format == IA_CSS_STREAM_FORMAT_RAW_12));
	/* raw_14 and raw_16 are not supported as input formats to the ISP.
	 * They can only be copied to a frame in memory using the
	 * copy binary.
	 */
}