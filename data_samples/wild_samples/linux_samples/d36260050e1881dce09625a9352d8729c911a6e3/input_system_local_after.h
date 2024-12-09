struct rx_cfg_s {
	rx_mode_t			mode;	/* The HW config */
	enum mipi_port_id		port;	/* The port ID to apply the control on */
	unsigned int		timeout;
	unsigned int		initcount;
	unsigned int		synccount;
	unsigned int		rxcount;
	mipi_predictor_t	comp;	/* Just for backward compatibility */
	bool                is_two_ppc;
};

/* NOTE: The base has already an offset of 0x0100 */
static const hrt_address MIPI_PORT_OFFSET[N_MIPI_PORT_ID] = {
	0x00000000UL,
	0x00000100UL,
	0x00000200UL};

static const mipi_lane_cfg_t MIPI_PORT_MAXLANES[N_MIPI_PORT_ID] = {
	MIPI_4LANE_CFG,
	MIPI_1LANE_CFG,
	MIPI_2LANE_CFG};

static const bool MIPI_PORT_ACTIVE[N_RX_MODE][N_MIPI_PORT_ID] = {
	{true, true, false},
	{true, true, false},
	{true, true, false},
	{true, true, false},
	{true, true, true},
	{true, true, true},
	{true, true, true},
	{true, true, true}};

static const mipi_lane_cfg_t MIPI_PORT_LANES[N_RX_MODE][N_MIPI_PORT_ID] = {
	{MIPI_4LANE_CFG, MIPI_1LANE_CFG, MIPI_0LANE_CFG},
	{MIPI_3LANE_CFG, MIPI_1LANE_CFG, MIPI_0LANE_CFG},
	{MIPI_2LANE_CFG, MIPI_1LANE_CFG, MIPI_0LANE_CFG},
	{MIPI_1LANE_CFG, MIPI_1LANE_CFG, MIPI_0LANE_CFG},
	{MIPI_2LANE_CFG, MIPI_1LANE_CFG, MIPI_2LANE_CFG},
	{MIPI_3LANE_CFG, MIPI_1LANE_CFG, MIPI_1LANE_CFG},
	{MIPI_2LANE_CFG, MIPI_1LANE_CFG, MIPI_1LANE_CFG},
	{MIPI_1LANE_CFG, MIPI_1LANE_CFG, MIPI_1LANE_CFG}};

static const hrt_address SUB_SYSTEM_OFFSET[N_SUB_SYSTEM_ID] = {
	0x00001000UL,
	0x00002000UL,
	0x00003000UL,
	0x00004000UL,
	0x00005000UL,
	0x00009000UL,
	0x0000A000UL,
	0x0000B000UL,
	0x0000C000UL};

struct capture_unit_state_s {
	int	Packet_Length;
	int	Received_Length;
	int	Received_Short_Packets;
	int	Received_Long_Packets;
	int	Last_Command;
	int	Next_Command;
	int	Last_Acknowledge;
	int	Next_Acknowledge;
	int	FSM_State_Info;
	int	StartMode;
	int	Start_Addr;
	int	Mem_Region_Size;
	int	Num_Mem_Regions;
/*	int	Init;   write-only registers
	int	Start;
	int	Stop;      */
};

struct acquisition_unit_state_s {
/*	int	Init;   write-only register */
	int	Received_Short_Packets;
	int	Received_Long_Packets;
	int	Last_Command;
	int	Next_Command;
	int	Last_Acknowledge;
	int	Next_Acknowledge;
	int	FSM_State_Info;
	int	Int_Cntr_Info;
	int	Start_Addr;
	int	Mem_Region_Size;
	int	Num_Mem_Regions;
};

struct ctrl_unit_state_s {
	int	last_cmd;
	int	next_cmd;
	int	last_ack;
	int	next_ack;
	int	top_fsm_state;
	int	captA_fsm_state;
	int	captB_fsm_state;
	int	captC_fsm_state;
	int	acq_fsm_state;
	int	captA_start_addr;
	int	captB_start_addr;
	int	captC_start_addr;
	int	captA_mem_region_size;
	int	captB_mem_region_size;
	int	captC_mem_region_size;
	int	captA_num_mem_regions;
	int	captB_num_mem_regions;
	int	captC_num_mem_regions;
	int	acq_start_addr;
	int	acq_mem_region_size;
	int	acq_num_mem_regions;
/*	int	ctrl_init;  write only register */
	int	capt_reserve_one_mem_region;
};

struct input_system_state_s {
	int	str_multicastA_sel;
	int	str_multicastB_sel;
	int	str_multicastC_sel;
	int	str_mux_sel;
	int	str_mon_status;
	int	str_mon_irq_cond;
	int	str_mon_irq_en;
	int	isys_srst;
	int	isys_slv_reg_srst;
	int	str_deint_portA_cnt;
	int	str_deint_portB_cnt;
	struct capture_unit_state_s		capture_unit[N_CAPTURE_UNIT_ID];
	struct acquisition_unit_state_s	acquisition_unit[N_ACQUISITION_UNIT_ID];
	struct ctrl_unit_state_s		ctrl_unit_state[N_CTRL_UNIT_ID];
};

struct mipi_port_state_s {
	int	device_ready;
	int	irq_status;
	int	irq_enable;
	uint32_t	timeout_count;
	uint16_t	init_count;
	uint16_t	raw16_18;
	uint32_t	sync_count;		/*4 x uint8_t */
	uint32_t	rx_count;		/*4 x uint8_t */
	uint8_t		lane_sync_count[MIPI_4LANE_CFG];
	uint8_t		lane_rx_count[MIPI_4LANE_CFG];
};

struct rx_channel_state_s {
	uint32_t	comp_scheme0;
	uint32_t	comp_scheme1;
	mipi_predictor_t		pred[N_MIPI_FORMAT_CUSTOM];
	mipi_compressor_t		comp[N_MIPI_FORMAT_CUSTOM];
};

struct receiver_state_s {
	uint8_t	fs_to_ls_delay;
	uint8_t	ls_to_data_delay;
	uint8_t	data_to_le_delay;
	uint8_t	le_to_fe_delay;
	uint8_t	fe_to_fs_delay;
	uint8_t	le_to_fs_delay;
	bool	is_two_ppc;
	int	backend_rst;
	uint16_t	raw18;
	bool		force_raw8;
	uint16_t	raw16;
	struct mipi_port_state_s	mipi_port_state[N_MIPI_PORT_ID];
	struct rx_channel_state_s	rx_channel_state[N_RX_CHANNEL_ID];
	int	be_gsp_acc_ovl;
	int	be_srst;
	int	be_is_two_ppc;
	int	be_comp_format0;
	int	be_comp_format1;
	int	be_comp_format2;
	int	be_comp_format3;
	int	be_sel;
	int	be_raw16_config;
	int	be_raw18_config;
	int	be_force_raw8;
	int	be_irq_status;
	int	be_irq_clear;
};

#endif /* __INPUT_SYSTEM_LOCAL_H_INCLUDED__ */