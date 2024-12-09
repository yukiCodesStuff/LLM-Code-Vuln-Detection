#define atomisp_css_irq_info  ia_css_irq_info
#define atomisp_css_isp_config ia_css_isp_config
#define atomisp_css_bayer_order ia_css_bayer_order
#define atomisp_css_capture_mode ia_css_capture_mode
#define atomisp_css_input_mode ia_css_input_mode
#define atomisp_css_frame ia_css_frame
#define atomisp_css_frame_format ia_css_frame_format
 */
#define CSS_ID(val)	(IA_ ## val)
#define CSS_EVENT(val)	(IA_CSS_EVENT_TYPE_ ## val)
#define CSS_FORMAT(val)	(ATOMISP_INPUT_FORMAT_ ## val)

#define CSS_EVENT_PORT_EOF	CSS_EVENT(PORT_EOF)
#define CSS_EVENT_FRAME_TAGGED	CSS_EVENT(FRAME_TAGGED)
