{
	assert(ID < N_RX_ID);
	assert(RX_BASE[ID] != (hrt_address)-1);
	return ia_css_device_load_uint32(RX_BASE[ID] + reg*sizeof(hrt_data));
}

STORAGE_CLASS_INPUT_SYSTEM_C void receiver_port_reg_store(
	const rx_ID_t				ID,
	const enum mipi_port_id			port_ID,
	const hrt_address			reg,
	const hrt_data				value)
{
	assert(ID < N_RX_ID);
	assert(port_ID < N_MIPI_PORT_ID);
	assert(RX_BASE[ID] != (hrt_address)-1);
	assert(MIPI_PORT_OFFSET[port_ID] != (hrt_address)-1);
	ia_css_device_store_uint32(RX_BASE[ID] + MIPI_PORT_OFFSET[port_ID] + reg*sizeof(hrt_data), value);
	return;
}
{
	assert(ID < N_RX_ID);
	assert(port_ID < N_MIPI_PORT_ID);
	assert(RX_BASE[ID] != (hrt_address)-1);
	assert(MIPI_PORT_OFFSET[port_ID] != (hrt_address)-1);
	ia_css_device_store_uint32(RX_BASE[ID] + MIPI_PORT_OFFSET[port_ID] + reg*sizeof(hrt_data), value);
	return;
}

STORAGE_CLASS_INPUT_SYSTEM_C hrt_data receiver_port_reg_load(
	const rx_ID_t				ID,
	const enum mipi_port_id			port_ID,
	const hrt_address			reg)
{
	assert(ID < N_RX_ID);
	assert(port_ID < N_MIPI_PORT_ID);
	assert(RX_BASE[ID] != (hrt_address)-1);
	assert(MIPI_PORT_OFFSET[port_ID] != (hrt_address)-1);
	return ia_css_device_load_uint32(RX_BASE[ID] + MIPI_PORT_OFFSET[port_ID] + reg*sizeof(hrt_data));
}