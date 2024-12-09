{
	struct qtnf_pcie_bus_priv *priv = (void *)get_bus_priv(bus);
	dma_addr_t txbd_paddr, skb_paddr;
	struct qtnf_tx_bd *txbd;
	int len, i;
	u32 info;
	int ret = 0;

	if (!qtnf_tx_queue_ready(priv)) {
		if (skb->dev)
			netif_stop_queue(skb->dev);

		return NETDEV_TX_BUSY;
	}
	if (ret && skb) {
		pr_err_ratelimited("drop skb\n");
		if (skb->dev)
			skb->dev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
	}

	qtnf_pcie_data_tx_reclaim(priv);
	priv->tx_done_count++;

	return NETDEV_TX_OK;
}

static int qtnf_pcie_control_tx(struct qtnf_bus *bus, struct sk_buff *skb)
{
	struct qtnf_pcie_bus_priv *priv = (void *)get_bus_priv(bus);

	return qtnf_shm_ipc_send(&priv->shm_ipc_ep_in, skb->data, skb->len);
}

static irqreturn_t qtnf_interrupt(int irq, void *data)
{
	struct qtnf_bus *bus = (struct qtnf_bus *)data;
	struct qtnf_pcie_bus_priv *priv = (void *)get_bus_priv(bus);
	u32 status;

	priv->pcie_irq_count++;
	status = readl(PCIE_HDP_INT_STATUS(priv->pcie_reg_base));

	qtnf_shm_ipc_irq_handler(&priv->shm_ipc_ep_in);
	qtnf_shm_ipc_irq_handler(&priv->shm_ipc_ep_out);

	if (!(status & priv->pcie_irq_mask))
		goto irq_done;

	if (status & PCIE_HDP_INT_RX_BITS)
		priv->pcie_irq_rx_count++;

	if (status & PCIE_HDP_INT_TX_BITS)
		priv->pcie_irq_tx_count++;

	if (status & PCIE_HDP_INT_HHBM_UF)
		priv->pcie_irq_uf_count++;

	if (status & PCIE_HDP_INT_RX_BITS) {
		qtnf_dis_rxdone_irq(priv);
		napi_schedule(&bus->mux_napi);
	}

	if (status & PCIE_HDP_INT_TX_BITS) {
		qtnf_dis_txdone_irq(priv);
		tasklet_hi_schedule(&priv->reclaim_tq);
	}

irq_done:
	/* H/W workaround: clean all bits, not only enabled */
	qtnf_non_posted_write(~0U, PCIE_HDP_INT_STATUS(priv->pcie_reg_base));

	if (!priv->msi_enabled)
		qtnf_deassert_intx(priv);

	return IRQ_HANDLED;
}

static int qtnf_rx_data_ready(struct qtnf_pcie_bus_priv *priv)
{
	u16 index = priv->rx_bd_r_index;
	struct qtnf_rx_bd *rxbd;
	u32 descw;

	rxbd = &priv->rx_bd_vbase[index];
	descw = le32_to_cpu(rxbd->info);

	if (descw & QTN_TXDONE_MASK)
		return 1;

	return 0;
}

static int qtnf_rx_poll(struct napi_struct *napi, int budget)
{
	struct qtnf_bus *bus = container_of(napi, struct qtnf_bus, mux_napi);
	struct qtnf_pcie_bus_priv *priv = (void *)get_bus_priv(bus);
	struct net_device *ndev = NULL;
	struct sk_buff *skb = NULL;
	int processed = 0;
	struct qtnf_rx_bd *rxbd;
	dma_addr_t skb_paddr;
	int consume;
	u32 descw;
	u32 psize;
	u16 r_idx;
	u16 w_idx;
	int ret;

	while (processed < budget) {


		if (!qtnf_rx_data_ready(priv))
			goto rx_out;

		r_idx = priv->rx_bd_r_index;
		rxbd = &priv->rx_bd_vbase[r_idx];
		descw = le32_to_cpu(rxbd->info);

		skb = priv->rx_skb[r_idx];
		psize = QTN_GET_LEN(descw);
		consume = 1;

		if (!(descw & QTN_TXDONE_MASK)) {
			pr_warn("skip invalid rxbd[%d]\n", r_idx);
			consume = 0;
		}
	if (!bus) {
		ret = -ENOMEM;
		goto err_init;
	}

	pcie_priv = get_bus_priv(bus);

	pci_set_drvdata(pdev, bus);
	bus->bus_ops = &qtnf_pcie_bus_ops;
	bus->dev = &pdev->dev;
	bus->fw_state = QTNF_FW_STATE_RESET;
	pcie_priv->pdev = pdev;

	strcpy(bus->fwname, QTN_PCI_PEARL_FW_NAME);
	init_completion(&bus->request_firmware_complete);
	mutex_init(&bus->bus_lock);
	spin_lock_init(&pcie_priv->irq_lock);
	spin_lock_init(&pcie_priv->tx_reclaim_lock);

	/* init stats */
	pcie_priv->tx_full_count = 0;
	pcie_priv->tx_done_count = 0;
	pcie_priv->pcie_irq_count = 0;
	pcie_priv->pcie_irq_rx_count = 0;
	pcie_priv->pcie_irq_tx_count = 0;
	pcie_priv->pcie_irq_uf_count = 0;
	pcie_priv->tx_reclaim_done = 0;
	pcie_priv->tx_reclaim_req = 0;

	pcie_priv->workqueue = create_singlethread_workqueue("QTNF_PEARL_PCIE");
	if (!pcie_priv->workqueue) {
		pr_err("failed to alloc bus workqueue\n");
		ret = -ENODEV;
		goto err_priv;
	}