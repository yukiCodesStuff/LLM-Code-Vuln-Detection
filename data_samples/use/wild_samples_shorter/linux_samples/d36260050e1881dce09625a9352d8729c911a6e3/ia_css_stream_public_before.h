 */
struct ia_css_stream_isys_stream_config {
	struct ia_css_resolution  input_res; /** Resolution of input data */
	enum ia_css_stream_format format; /** Format of input stream. This data
					       format will be mapped to MIPI data
					       type internally. */
	int linked_isys_stream_id; /** default value is -1, other value means
							current isys_stream shares the same buffer with
							Used for CSS 2400/1 System and deprecated for other
							systems (replaced by input_effective_res in
							ia_css_pipe_config) */
	enum ia_css_stream_format format; /** Format of input stream. This data
					       format will be mapped to MIPI data
					       type internally. */
	enum ia_css_bayer_order bayer_order; /** Bayer order for RAW streams */
};
 *
 * This function will return the stream format.
 */
enum ia_css_stream_format
ia_css_stream_get_format(const struct ia_css_stream *stream);

/* @brief Check if the stream is configured for 2 pixels per clock
 * @param[in]	stream The stream.
 */
void
ia_css_stream_send_input_embedded_line(const struct ia_css_stream *stream,
			      enum ia_css_stream_format format,
			      const unsigned short *data,
			      unsigned int width);

/* @brief End an input frame on the CSS input FIFO.