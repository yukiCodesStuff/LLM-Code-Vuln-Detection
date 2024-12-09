#endif
struct inputfifo_instance {
	unsigned int				ch_id;
	enum ia_css_stream_format	input_format;
	bool						two_ppc;
	bool						streaming;
	unsigned int				hblank_cycles;
	unsigned int				marker_cycles;


static enum inputfifo_mipi_data_type inputfifo_determine_type(
	enum ia_css_stream_format input_format)
{
	enum inputfifo_mipi_data_type type;

	type = inputfifo_mipi_data_type_regular;
	if (input_format == IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY) {
		type =
			inputfifo_mipi_data_type_yuv420_legacy;
	} else if (input_format == IA_CSS_STREAM_FORMAT_YUV420_8  ||
		   input_format == IA_CSS_STREAM_FORMAT_YUV420_10 ||
		   input_format == IA_CSS_STREAM_FORMAT_YUV420_16) {
		type =
			inputfifo_mipi_data_type_yuv420;
	} else if (input_format >= IA_CSS_STREAM_FORMAT_RGB_444 &&
		   input_format <= IA_CSS_STREAM_FORMAT_RGB_888) {
		type =
			inputfifo_mipi_data_type_rgb;
	}
	return type;
	unsigned int width,
	unsigned int height,
	unsigned int ch_id,
	enum ia_css_stream_format input_format,
	bool two_ppc)
{
	unsigned int fmt_type, hblank_cycles, marker_cycles;
	enum inputfifo_mipi_data_type type;

void ia_css_inputfifo_start_frame(
	unsigned int ch_id,
	enum ia_css_stream_format input_format,
	bool two_ppc)
{
	struct inputfifo_instance *s2mi;
	s2mi = inputfifo_get_inst(ch_id);

void ia_css_inputfifo_send_embedded_line(
	unsigned int	ch_id,
	enum ia_css_stream_format	data_type,
	const unsigned short	*data,
	unsigned int	width)
{
	struct inputfifo_instance *s2mi;