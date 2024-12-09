	spinlock_t			_lock;
	struct list_head		node;
	struct omap_hwmod_ocp_if	*_mpu_port;
	u16				flags;
	u8				mpu_rt_idx;
	u8				response_lat;
	u8				rst_lines_cnt;