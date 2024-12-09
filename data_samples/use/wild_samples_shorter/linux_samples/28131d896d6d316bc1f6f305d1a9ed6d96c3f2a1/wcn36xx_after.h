	enum wcn36xx_hal_bss_type bss_type;

	/* Power management */
	enum wcn36xx_power_state pw_state;

	u8 bss_index;
	/* Returned from WCN36XX_HAL_ADD_STA_SELF_RSP */
	} rekey_data;

	struct list_head sta_list;

	int bmps_fail_ct;
};

/**
 * struct wcn36xx_sta - holds STA related fields
	struct sk_buff		*tx_ack_skb;
	struct timer_list	tx_ack_timer;

	/* For A-MSDU re-aggregation */
	struct sk_buff_head amsdu;

	/* RF module */
	unsigned		rf_id;

#ifdef CONFIG_WCN36XX_DEBUGFS
	/* Debug file system entry */
	struct wcn36xx_dfs_entry    dfs;
#endif /* CONFIG_WCN36XX_DEBUGFS */
};

static inline bool wcn36xx_is_fw_version(struct wcn36xx *wcn,
					 u8 major,