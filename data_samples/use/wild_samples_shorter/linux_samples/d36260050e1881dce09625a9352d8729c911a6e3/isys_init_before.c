#elif defined(USE_INPUT_SYSTEM_VERSION_2401)
input_system_error_t ia_css_isys_init(void)
{
	input_system_error_t error = INPUT_SYSTEM_ERR_NO_ERROR;

	ia_css_isys_csi_rx_lut_rmgr_init();
	ia_css_isys_ibuf_rmgr_init();
	ia_css_isys_dma_channel_rmgr_init();
	ia_css_isys_stream2mmio_sid_rmgr_init();
	isys_irqc_status_enable(ISYS_IRQ1_ID);
	isys_irqc_status_enable(ISYS_IRQ2_ID);

	return error;
}
#endif

#if defined(USE_INPUT_SYSTEM_VERSION_2)