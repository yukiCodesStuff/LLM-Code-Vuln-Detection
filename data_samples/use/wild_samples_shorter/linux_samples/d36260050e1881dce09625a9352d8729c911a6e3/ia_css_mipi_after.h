 *
 */
enum ia_css_err
ia_css_mipi_frame_enable_check_on_size(const enum mipi_port_id port,
				const unsigned int	size_mem_words);
#endif

/* @brief Calculate the size of a mipi frame.
enum ia_css_err
ia_css_mipi_frame_calculate_size(const unsigned int width,
				const unsigned int height,
				const enum atomisp_input_format format,
				const bool hasSOLandEOL,
				const unsigned int embedded_data_size_words,
				unsigned int *size_mem_words);
