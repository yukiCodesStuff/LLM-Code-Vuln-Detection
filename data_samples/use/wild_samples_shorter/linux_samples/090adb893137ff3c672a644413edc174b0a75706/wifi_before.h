	void (*bt_wifi_media_status_notify) (struct ieee80211_hw *hw,
					     bool mstate);
	void (*bt_coex_off_before_lps) (struct ieee80211_hw *hw);
};

struct rtl_intf_ops {
	/*com */
	struct delayed_work fwevt_wq;

	struct work_struct lps_change_work;
};

struct rtl_debug {
	u32 dbgp_type[DBGP_TYPE_MAX];
		};
	};
	bool enter_ps;	/* true when entering PS */

	/*This must be the last item so
	   that it points to the data allocated
	   beyond  this structure like: