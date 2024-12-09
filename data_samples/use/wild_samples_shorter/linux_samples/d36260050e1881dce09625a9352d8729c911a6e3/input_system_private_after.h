
STORAGE_CLASS_INPUT_SYSTEM_C void receiver_port_reg_store(
	const rx_ID_t				ID,
	const enum mipi_port_id			port_ID,
	const hrt_address			reg,
	const hrt_data				value)
{
	assert(ID < N_RX_ID);

STORAGE_CLASS_INPUT_SYSTEM_C hrt_data receiver_port_reg_load(
	const rx_ID_t				ID,
	const enum mipi_port_id			port_ID,
	const hrt_address			reg)
{
	assert(ID < N_RX_ID);
	assert(port_ID < N_MIPI_PORT_ID);