struct qtnf_pcie_bus_priv {
	struct pci_dev  *pdev;

	/* lock for irq configuration changes */
	spinlock_t irq_lock;

	/* lock for tx reclaim operations */
	spinlock_t tx_reclaim_lock;
	u8 msi_enabled;
	int mps;

	struct workqueue_struct *workqueue;
	struct tasklet_struct reclaim_tq;

	void __iomem *sysctl_bar;
	void __iomem *epmem_bar;
	void __iomem *dmareg_bar;

	struct qtnf_shm_ipc shm_ipc_ep_in;
	struct qtnf_shm_ipc shm_ipc_ep_out;

	struct qtnf_pcie_bda __iomem *bda;
	void __iomem *pcie_reg_base;

	u16 tx_bd_num;
	u16 rx_bd_num;

	struct sk_buff **tx_skb;
	struct sk_buff **rx_skb;

	struct qtnf_tx_bd *tx_bd_vbase;
	dma_addr_t tx_bd_pbase;

	struct qtnf_rx_bd *rx_bd_vbase;
	dma_addr_t rx_bd_pbase;

	dma_addr_t bd_table_paddr;
	void *bd_table_vaddr;
	u32 bd_table_len;

	u32 rx_bd_w_index;
	u32 rx_bd_r_index;

	u32 tx_bd_w_index;
	u32 tx_bd_r_index;

	u32 pcie_irq_mask;

	/* diagnostics stats */
	u32 pcie_irq_count;
	u32 pcie_irq_rx_count;
	u32 pcie_irq_tx_count;
	u32 pcie_irq_uf_count;
	u32 tx_full_count;
	u32 tx_done_count;
	u32 tx_reclaim_done;
	u32 tx_reclaim_req;
};

#endif /* _QTN_FMAC_PCIE_H_ */