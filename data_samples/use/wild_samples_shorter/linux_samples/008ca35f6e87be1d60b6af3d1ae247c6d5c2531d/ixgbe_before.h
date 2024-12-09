	u8 trusted;
	int xcast_mode;
	unsigned int vf_api;
};

enum ixgbevf_xcast_modes {
	IXGBEVF_XCAST_MODE_NONE = 0,
#define IXGBE_TRY_LINK_TIMEOUT (4 * HZ)
#define IXGBE_SFP_POLL_JIFFIES (2 * HZ)	/* SFP poll every 2 seconds */

/* board specific private data structure */
struct ixgbe_adapter {
	unsigned long active_vlans[BITS_TO_LONGS(VLAN_N_VID)];
	/* OS defined structs */
#define IXGBE_FLAG2_RX_LEGACY			BIT(16)
#define IXGBE_FLAG2_IPSEC_ENABLED		BIT(17)
#define IXGBE_FLAG2_VF_IPSEC_ENABLED		BIT(18)

	/* Tx fast path data */
	int num_tx_queues;
	u16 tx_itr_setting;