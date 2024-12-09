#define FW_RX_DESC_UDP              (1 << 6)

struct fw_rx_desc_hl {
	u8 info0;
	u8 version;
	u8 len;
	u8 flags;
} __packed;