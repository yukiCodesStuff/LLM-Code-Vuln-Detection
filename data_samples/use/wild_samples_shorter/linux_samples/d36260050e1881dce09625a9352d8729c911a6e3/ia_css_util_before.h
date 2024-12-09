 *
 */
extern unsigned int ia_css_util_input_format_bpp(
	enum ia_css_stream_format stream_format,
	bool two_ppc);

/* @brief check if input format it raw
 *
 *
 */
extern bool ia_css_util_is_input_format_raw(
	enum ia_css_stream_format stream_format);

/* @brief check if input format it yuv
 *
 * @param[in] stream_format
 *
 */
extern bool ia_css_util_is_input_format_yuv(
	enum ia_css_stream_format stream_format);

#endif /* __IA_CSS_UTIL_H__ */
