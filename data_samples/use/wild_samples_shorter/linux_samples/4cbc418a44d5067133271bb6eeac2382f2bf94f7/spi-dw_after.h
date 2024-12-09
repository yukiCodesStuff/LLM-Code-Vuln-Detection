	size_t			len;
	void			*tx;
	void			*tx_end;
	spinlock_t		buf_lock;
	void			*rx;
	void			*rx_end;
	int			dma_mapped;
	u8			n_bytes;	/* current is a 1/2 bytes op */