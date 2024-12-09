	unsigned int	width,
	unsigned int	height,
	unsigned int	ch_id,
	enum ia_css_stream_format	input_format,
	bool			two_ppc);

void ia_css_inputfifo_start_frame(
	unsigned int	ch_id,
	enum ia_css_stream_format	input_format,
	bool			two_ppc);

void ia_css_inputfifo_send_line(
	unsigned int	ch_id,

void ia_css_inputfifo_send_embedded_line(
	unsigned int	ch_id,
	enum ia_css_stream_format	data_type,
	const unsigned short	*data,
	unsigned int	width);

void ia_css_inputfifo_end_frame(