struct dw_spi {
	struct spi_controller	*master;
	enum dw_ssi_type	type;

	void __iomem		*regs;
	unsigned long		paddr;
	int			irq;
	u32			fifo_len;	/* depth of the FIFO buffer */
	u32			max_freq;	/* max bus freq supported */

	int			cs_override;
	u32			reg_io_width;	/* DR I/O width in bytes */
	u16			bus_num;
	u16			num_cs;		/* supported slave numbers */
	void (*set_cs)(struct spi_device *spi, bool enable);

	/* Current message transfer state info */
	size_t			len;
	void			*tx;
	void			*tx_end;
	spinlock_t		buf_lock;
	void			*rx;
	void			*rx_end;
	int			dma_mapped;
	u8			n_bytes;	/* current is a 1/2 bytes op */
	u32			dma_width;
	irqreturn_t		(*transfer_handler)(struct dw_spi *dws);
	u32			current_freq;	/* frequency in hz */

	/* DMA info */
	int			dma_inited;
	struct dma_chan		*txchan;
	struct dma_chan		*rxchan;
	unsigned long		dma_chan_busy;
	dma_addr_t		dma_addr; /* phy address of the Data register */
	const struct dw_spi_dma_ops *dma_ops;
	void			*dma_tx;
	void			*dma_rx;

	/* Bus interface info */
	void			*priv;
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
#endif
};

static inline u32 dw_readl(struct dw_spi *dws, u32 offset)
{
	return __raw_readl(dws->regs + offset);
}