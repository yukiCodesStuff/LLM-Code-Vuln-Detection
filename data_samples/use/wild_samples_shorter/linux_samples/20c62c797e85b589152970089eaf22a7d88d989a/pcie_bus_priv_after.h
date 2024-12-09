
	/* lock for tx reclaim operations */
	spinlock_t tx_reclaim_lock;
	/* lock for tx0 operations */
	spinlock_t tx0_lock;
	u8 msi_enabled;
	int mps;

	struct workqueue_struct *workqueue;