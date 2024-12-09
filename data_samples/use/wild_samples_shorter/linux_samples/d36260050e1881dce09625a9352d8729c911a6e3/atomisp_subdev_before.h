	u32     code;
	uint8_t bpp; /* bits per pixel */
	uint8_t depth; /* uncompressed */
	enum atomisp_css_stream_format atomisp_in_fmt;
	enum atomisp_css_bayer_order bayer_order;
	enum ia_css_stream_format css_stream_fmt;
};

struct atomisp_sub_device;

const struct atomisp_in_fmt_conv *atomisp_find_in_fmt_conv(u32 code);
#ifndef ISP2401
const struct atomisp_in_fmt_conv *atomisp_find_in_fmt_conv_by_atomisp_in_fmt(
	enum atomisp_css_stream_format atomisp_in_fmt);
#else
const struct atomisp_in_fmt_conv
    *atomisp_find_in_fmt_conv_by_atomisp_in_fmt(enum atomisp_css_stream_format
						atomisp_in_fmt);
#endif
const struct atomisp_in_fmt_conv *atomisp_find_in_fmt_conv_compressed(u32 code);
bool atomisp_subdev_format_conversion(struct atomisp_sub_device *asd,