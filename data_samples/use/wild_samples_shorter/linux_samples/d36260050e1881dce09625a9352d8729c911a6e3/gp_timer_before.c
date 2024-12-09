static void
gp_timer_reg_store(uint32_t reg, uint32_t value);

uint32_t
gp_timer_reg_load(uint32_t reg)
{
	return ia_css_device_load_uint32(
			GP_TIMER_BASE +