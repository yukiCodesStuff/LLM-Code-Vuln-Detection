 *  to process sensor metadata.
 */
struct ia_css_metadata_config {
	enum atomisp_input_format data_type; /** Data type of CSI-2 embedded
			data. The default value is ATOMISP_INPUT_FORMAT_EMBEDDED. For
			certain sensors, user can choose non-default data type for embedded
			data. */
	struct ia_css_resolution  resolution; /** Resolution */
};