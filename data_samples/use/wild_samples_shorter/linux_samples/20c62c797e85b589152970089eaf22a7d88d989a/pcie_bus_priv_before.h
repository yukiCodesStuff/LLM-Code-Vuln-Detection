
	/* lock for tx reclaim operations */
	spinlock_t tx_reclaim_lock;
	u8 msi_enabled;
	int mps;

	struct workqueue_struct *workqueue;