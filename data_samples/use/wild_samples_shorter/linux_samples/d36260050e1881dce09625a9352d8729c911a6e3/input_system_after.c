#define ZERO (0x0)
#define ONE  (1U)

static const ib_buffer_t   IB_BUFFER_NULL = {0 ,0, 0 };

static input_system_error_t input_system_configure_channel(
	const channel_cfg_t		channel);


static inline void mipi_port_get_state(
	const rx_ID_t					ID,
	const enum mipi_port_id			port_ID,
	mipi_port_state_t				*state);

static inline void rx_channel_get_state(
	const rx_ID_t					ID,
	const rx_ID_t				ID,
	receiver_state_t			*state)
{
	enum mipi_port_id	port_id;
	unsigned int	ch_id;

	assert(ID < N_RX_ID);
	assert(state != NULL);
	state->raw16 = (uint16_t)receiver_reg_load(ID,
		_HRT_CSS_RECEIVER_RAW16_REG_IDX);

	for (port_id = (enum mipi_port_id)0; port_id < N_MIPI_PORT_ID; port_id++) {
		mipi_port_get_state(ID, port_id,
			&(state->mipi_port_state[port_id]));
	}
	for (ch_id = (unsigned int)0; ch_id < N_RX_CHANNEL_ID; ch_id++) {

void receiver_port_enable(
	const rx_ID_t			ID,
	const enum mipi_port_id		port_ID,
	const bool			cnd)
{
	hrt_data	reg = receiver_port_reg_load(ID, port_ID,
		_HRT_CSS_RECEIVER_DEVICE_READY_REG_IDX);

bool is_receiver_port_enabled(
	const rx_ID_t			ID,
	const enum mipi_port_id		port_ID)
{
	hrt_data	reg = receiver_port_reg_load(ID, port_ID,
		_HRT_CSS_RECEIVER_DEVICE_READY_REG_IDX);
	return ((reg & 0x01) != 0);

void receiver_irq_enable(
	const rx_ID_t			ID,
	const enum mipi_port_id		port_ID,
	const rx_irq_info_t		irq_info)
{
	receiver_port_reg_store(ID,
		port_ID, _HRT_CSS_RECEIVER_IRQ_ENABLE_REG_IDX, irq_info);

rx_irq_info_t receiver_get_irq_info(
	const rx_ID_t			ID,
	const enum mipi_port_id		port_ID)
{
	return receiver_port_reg_load(ID,
	port_ID, _HRT_CSS_RECEIVER_IRQ_STATUS_REG_IDX);
}

void receiver_irq_clear(
	const rx_ID_t			ID,
	const enum mipi_port_id		port_ID,
	const rx_irq_info_t		irq_info)
{
	receiver_port_reg_store(ID,
		port_ID, _HRT_CSS_RECEIVER_IRQ_STATUS_REG_IDX, irq_info);

static inline void mipi_port_get_state(
	const rx_ID_t				ID,
	const enum mipi_port_id			port_ID,
	mipi_port_state_t			*state)
{
	int	i;

}

// MW: "2400" in the name is not good, but this is to avoid a naming conflict
static input_system_cfg2400_t config;

static void receiver_rst(
	const rx_ID_t				ID)
{
	enum mipi_port_id		port_id;

	assert(ID < N_RX_ID);

// Disable all ports.