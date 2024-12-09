#define FW_RX_DESC_UDP              (1 << 6)

struct fw_rx_desc_hl {
	union {
		struct {
		u8 discard:1,
		   forward:1,
		   any_err:1,
		   dup_err:1,
		   reserved:1,
		   inspect:1,
		   extension:2;
		} bits;
		u8 info0;
	} u;

	u8 version;
	u8 len;
	u8 flags;
} __packed;