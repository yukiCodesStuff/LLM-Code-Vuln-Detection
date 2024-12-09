	WCN36XX_HAL_START_SCAN_OFFLOAD_RSP = 205,
	WCN36XX_HAL_STOP_SCAN_OFFLOAD_REQ = 206,
	WCN36XX_HAL_STOP_SCAN_OFFLOAD_RSP = 207,
	WCN36XX_HAL_UPDATE_CHANNEL_LIST_REQ = 208,
	WCN36XX_HAL_UPDATE_CHANNEL_LIST_RSP = 209,
	WCN36XX_HAL_SCAN_OFFLOAD_IND = 210,

	WCN36XX_HAL_AVOID_FREQ_RANGE_IND = 233,

	u32 status;
} __packed;

#define WCN36XX_HAL_CHAN_REG1_MIN_PWR_MASK  0x000000ff
#define WCN36XX_HAL_CHAN_REG1_MAX_PWR_MASK  0x0000ff00
#define WCN36XX_HAL_CHAN_REG1_REG_PWR_MASK  0x00ff0000
#define WCN36XX_HAL_CHAN_REG1_CLASS_ID_MASK 0xff000000
#define WCN36XX_HAL_CHAN_REG2_ANT_GAIN_MASK 0x000000ff
#define WCN36XX_HAL_CHAN_INFO_FLAG_PASSIVE  BIT(7)
#define WCN36XX_HAL_CHAN_INFO_FLAG_DFS      BIT(10)
#define WCN36XX_HAL_CHAN_INFO_FLAG_HT       BIT(11)
#define WCN36XX_HAL_CHAN_INFO_FLAG_VHT      BIT(12)
#define WCN36XX_HAL_CHAN_INFO_PHY_11A       0
#define WCN36XX_HAL_CHAN_INFO_PHY_11BG      1
#define WCN36XX_HAL_DEFAULT_ANT_GAIN        6
#define WCN36XX_HAL_DEFAULT_MIN_POWER       6

struct wcn36xx_hal_channel_param {
	u32 mhz;
	u32 band_center_freq1;
	u32 band_center_freq2;
	u32 channel_info;
	u32 reg_info_1;
	u32 reg_info_2;
} __packed;

struct wcn36xx_hal_update_channel_list_req_msg {
	struct wcn36xx_hal_msg_header header;

	u8 num_channel;
	struct wcn36xx_hal_channel_param channels[80];
} __packed;

enum wcn36xx_hal_rate_index {
	HW_RATE_INDEX_1MBPS	= 0x82,
	HW_RATE_INDEX_2MBPS	= 0x84,
	HW_RATE_INDEX_5_5MBPS	= 0x8B,