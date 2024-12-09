enum ia_css_err
ia_css_mipi_frame_calculate_size(const unsigned int width,
				const unsigned int height,
				const enum ia_css_stream_format format,
				const bool hasSOLandEOL,
				const unsigned int embedded_data_size_words,
				unsigned int *size_mem_words)
{
		     width_padded, height, format, hasSOLandEOL, embedded_data_size_words);

	switch (format) {
	case IA_CSS_STREAM_FORMAT_RAW_6:		/* 4p, 3B, 24bits */
		bits_per_pixel = 6;	break;
	case IA_CSS_STREAM_FORMAT_RAW_7:		/* 8p, 7B, 56bits */
		bits_per_pixel = 7;		break;
	case IA_CSS_STREAM_FORMAT_RAW_8:		/* 1p, 1B, 8bits */
	case IA_CSS_STREAM_FORMAT_BINARY_8:		/*  8bits, TODO: check. */
	case IA_CSS_STREAM_FORMAT_YUV420_8:		/* odd 2p, 2B, 16bits, even 2p, 4B, 32bits */
		bits_per_pixel = 8;		break;
	case IA_CSS_STREAM_FORMAT_YUV420_10:		/* odd 4p, 5B, 40bits, even 4p, 10B, 80bits */
	case IA_CSS_STREAM_FORMAT_RAW_10:		/* 4p, 5B, 40bits */
#if !defined(HAS_NO_PACKED_RAW_PIXELS)
		/* The changes will be reverted as soon as RAW
		 * Buffers are deployed by the 2401 Input System
		 * in the non-continuous use scenario.
		bits_per_pixel = 16;
#endif
		break;
	case IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY:	/* 2p, 3B, 24bits */
	case IA_CSS_STREAM_FORMAT_RAW_12:		/* 2p, 3B, 24bits */
		bits_per_pixel = 12;	break;
	case IA_CSS_STREAM_FORMAT_RAW_14:		/* 4p, 7B, 56bits */
		bits_per_pixel = 14;	break;
	case IA_CSS_STREAM_FORMAT_RGB_444:		/* 1p, 2B, 16bits */
	case IA_CSS_STREAM_FORMAT_RGB_555:		/* 1p, 2B, 16bits */
	case IA_CSS_STREAM_FORMAT_RGB_565:		/* 1p, 2B, 16bits */
	case IA_CSS_STREAM_FORMAT_YUV422_8:		/* 2p, 4B, 32bits */
		bits_per_pixel = 16;	break;
	case IA_CSS_STREAM_FORMAT_RGB_666:		/* 4p, 9B, 72bits */
		bits_per_pixel = 18;	break;
	case IA_CSS_STREAM_FORMAT_YUV422_10:		/* 2p, 5B, 40bits */
		bits_per_pixel = 20;	break;
	case IA_CSS_STREAM_FORMAT_RGB_888:		/* 1p, 3B, 24bits */
		bits_per_pixel = 24;	break;

	case IA_CSS_STREAM_FORMAT_YUV420_16:		/* Not supported */
	case IA_CSS_STREAM_FORMAT_YUV422_16:		/* Not supported */
	case IA_CSS_STREAM_FORMAT_RAW_16:		/* TODO: not specified in MIPI SPEC, check */
	default:
		return IA_CSS_ERR_INVALID_ARGUMENTS;
	}

	odd_line_bytes = (width_padded * bits_per_pixel + 7) >> 3; /* ceil ( bits per line / 8) */

	/* Even lines for YUV420 formats are double in bits_per_pixel. */
	if (format == IA_CSS_STREAM_FORMAT_YUV420_8
			|| format == IA_CSS_STREAM_FORMAT_YUV420_10
			|| format == IA_CSS_STREAM_FORMAT_YUV420_16) {
		even_line_bytes = (width_padded * 2 * bits_per_pixel + 7) >> 3; /* ceil ( bits per line / 8) */
	} else {
		even_line_bytes = odd_line_bytes;
	}

#if !defined(HAS_NO_INPUT_SYSTEM) && defined(USE_INPUT_SYSTEM_VERSION_2)
enum ia_css_err
ia_css_mipi_frame_enable_check_on_size(const enum ia_css_csi2_port port,
				const unsigned int	size_mem_words)
{
	uint32_t idx;

#else
	unsigned int width;
	unsigned int height;
	enum ia_css_stream_format format;
	bool pack_raw_pixels;

	unsigned int width_padded;
	unsigned int bits_per_pixel = 0;

	bits_per_pixel = sh_css_stream_format_2_bits_per_subpixel(format);
	bits_per_pixel =
		(format == IA_CSS_STREAM_FORMAT_RAW_10 && pack_raw_pixels) ? bits_per_pixel : 16;
	if (bits_per_pixel == 0)
		return IA_CSS_ERR_INTERNAL_ERROR;

	odd_line_bytes = (width_padded * bits_per_pixel + 7) >> 3; /* ceil ( bits per line / 8) */

	/* Even lines for YUV420 formats are double in bits_per_pixel. */
	if (format == IA_CSS_STREAM_FORMAT_YUV420_8
		|| format == IA_CSS_STREAM_FORMAT_YUV420_10) {
		even_line_bytes = (width_padded * 2 * bits_per_pixel + 7) >> 3; /* ceil ( bits per line / 8) */
	} else {
		even_line_bytes = odd_line_bytes;
	}