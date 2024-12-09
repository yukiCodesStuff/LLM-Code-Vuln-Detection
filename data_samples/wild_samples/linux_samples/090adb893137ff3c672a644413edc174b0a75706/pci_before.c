		if (unlikely(!new_skb)) {
			RT_TRACE(rtlpriv, (COMP_INTR | COMP_RECV), DBG_DMESG,
				 "can't alloc skb for rx\n");
			goto done;
		}

		pci_unmap_single(rtlpci->pdev,
				 *((dma_addr_t *) skb->cb),
				 rtlpci->rxbuffersize,
				 PCI_DMA_FROMDEVICE);

		skb_put(skb, rtlpriv->cfg->ops->get_desc((u8 *) pdesc, false,
			HW_DESC_RXPKT_LEN));
		skb_reserve(skb, stats.rx_drvinfo_size + stats.rx_bufshift);

		/*
		 * NOTICE This can not be use for mac80211,
		 * this is done in mac80211 code,
		 * if you done here sec DHCP will fail
		 * skb_trim(skb, skb->len - 4);
		 */

		_rtl_receive_one(hw, skb, rx_status);

		if (((rtlpriv->link_info.num_rx_inperiod +
		      rtlpriv->link_info.num_tx_inperiod) > 8) ||
		      (rtlpriv->link_info.num_rx_inperiod > 2)) {
			rtlpriv->enter_ps = false;
			schedule_work(&rtlpriv->works.lps_change_work);
		}

		dev_kfree_skb_any(skb);
		skb = new_skb;

		rtlpci->rx_ring[rx_queue_idx].rx_buf[index] = skb;
		*((dma_addr_t *) skb->cb) =
			    pci_map_single(rtlpci->pdev, skb_tail_pointer(skb),
					   rtlpci->rxbuffersize,
					   PCI_DMA_FROMDEVICE);

done:
		bufferaddress = (*((dma_addr_t *)skb->cb));
		if (pci_dma_mapping_error(rtlpci->pdev, bufferaddress))
			return;
		tmp_one = 1;
		rtlpriv->cfg->ops->set_desc((u8 *) pdesc, false,
					    HW_DESC_RXBUFF_ADDR,
					    (u8 *)&bufferaddress);
		rtlpriv->cfg->ops->set_desc((u8 *)pdesc, false,
					    HW_DESC_RXPKT_LEN,
					    (u8 *)&rtlpci->rxbuffersize);

		if (index == rtlpci->rxringcount - 1)
			rtlpriv->cfg->ops->set_desc((u8 *)pdesc, false,
						    HW_DESC_RXERO,
						    &tmp_one);

		rtlpriv->cfg->ops->set_desc((u8 *)pdesc, false, HW_DESC_RXOWN,
					    &tmp_one);

		index = (index + 1) % rtlpci->rxringcount;
	}

	rtlpci->rx_ring[rx_queue_idx].idx = index;
}

static irqreturn_t _rtl_pci_interrupt(int irq, void *dev_id)
{
	struct ieee80211_hw *hw = dev_id;
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	unsigned long flags;
	u32 inta = 0;
	u32 intb = 0;
	irqreturn_t ret = IRQ_HANDLED;

	spin_lock_irqsave(&rtlpriv->locks.irq_th_lock, flags);

	/*read ISR: 4/8bytes */
	rtlpriv->cfg->ops->interrupt_recognized(hw, &inta, &intb);

	/*Shared IRQ or HW disappared */
	if (!inta || inta == 0xffff) {
		ret = IRQ_NONE;
		goto done;
	}

	/*<1> beacon related */
	if (inta & rtlpriv->cfg->maps[RTL_IMR_TBDOK]) {
		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "beacon ok interrupt!\n");
	}

	if (unlikely(inta & rtlpriv->cfg->maps[RTL_IMR_TBDER])) {
		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "beacon err interrupt!\n");
	}

	if (inta & rtlpriv->cfg->maps[RTL_IMR_BDOK]) {
		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE, "beacon interrupt!\n");
	}

	if (inta & rtlpriv->cfg->maps[RTL_IMR_BCNINT]) {
		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "prepare beacon for interrupt!\n");
		tasklet_schedule(&rtlpriv->works.irq_prepare_bcn_tasklet);
	}

	/*<3> Tx related */
	if (unlikely(inta & rtlpriv->cfg->maps[RTL_IMR_TXFOVW]))
		RT_TRACE(rtlpriv, COMP_ERR, DBG_WARNING, "IMR_TXFOVW!\n");

	if (inta & rtlpriv->cfg->maps[RTL_IMR_MGNTDOK]) {
		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "Manage ok interrupt!\n");
		_rtl_pci_tx_isr(hw, MGNT_QUEUE);
	}

	if (inta & rtlpriv->cfg->maps[RTL_IMR_HIGHDOK]) {
		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "HIGH_QUEUE ok interrupt!\n");
		_rtl_pci_tx_isr(hw, HIGH_QUEUE);
	}

	if (inta & rtlpriv->cfg->maps[RTL_IMR_BKDOK]) {
		rtlpriv->link_info.num_tx_inperiod++;

		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "BK Tx OK interrupt!\n");
		_rtl_pci_tx_isr(hw, BK_QUEUE);
	}

	if (inta & rtlpriv->cfg->maps[RTL_IMR_BEDOK]) {
		rtlpriv->link_info.num_tx_inperiod++;

		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "BE TX OK interrupt!\n");
		_rtl_pci_tx_isr(hw, BE_QUEUE);
	}

	if (inta & rtlpriv->cfg->maps[RTL_IMR_VIDOK]) {
		rtlpriv->link_info.num_tx_inperiod++;

		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "VI TX OK interrupt!\n");
		_rtl_pci_tx_isr(hw, VI_QUEUE);
	}

	if (inta & rtlpriv->cfg->maps[RTL_IMR_VODOK]) {
		rtlpriv->link_info.num_tx_inperiod++;

		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
			 "Vo TX OK interrupt!\n");
		_rtl_pci_tx_isr(hw, VO_QUEUE);
	}

	if (rtlhal->hw_type == HARDWARE_TYPE_RTL8192SE) {
		if (inta & rtlpriv->cfg->maps[RTL_IMR_COMDOK]) {
			rtlpriv->link_info.num_tx_inperiod++;

			RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
				 "CMD TX OK interrupt!\n");
			_rtl_pci_tx_isr(hw, TXCMD_QUEUE);
		}
	}

	/*<2> Rx related */
	if (inta & rtlpriv->cfg->maps[RTL_IMR_ROK]) {
		RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE, "Rx ok interrupt!\n");
		_rtl_pci_rx_interrupt(hw);
	}

	if (unlikely(inta & rtlpriv->cfg->maps[RTL_IMR_RDU])) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_WARNING,
			 "rx descriptor unavailable!\n");
		_rtl_pci_rx_interrupt(hw);
	}

	if (unlikely(inta & rtlpriv->cfg->maps[RTL_IMR_RXFOVW])) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_WARNING, "rx overflow !\n");
		_rtl_pci_rx_interrupt(hw);
	}

	/*fw related*/
	if (rtlhal->hw_type == HARDWARE_TYPE_RTL8723AE) {
		if (inta & rtlpriv->cfg->maps[RTL_IMR_C2HCMD]) {
			RT_TRACE(rtlpriv, COMP_INTR, DBG_TRACE,
				 "firmware interrupt!\n");
			queue_delayed_work(rtlpriv->works.rtl_wq,
					   &rtlpriv->works.fwevt_wq, 0);
		}
	}

	if (rtlpriv->rtlhal.earlymode_enable)
		tasklet_schedule(&rtlpriv->works.irq_tasklet);

done:
	spin_unlock_irqrestore(&rtlpriv->locks.irq_th_lock, flags);
	return ret;
}

static void _rtl_pci_irq_tasklet(struct ieee80211_hw *hw)
{
	_rtl_pci_tx_chk_waitq(hw);
}

static void _rtl_pci_prepare_bcn_tasklet(struct ieee80211_hw *hw)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	struct rtl8192_tx_ring *ring = NULL;
	struct ieee80211_hdr *hdr = NULL;
	struct ieee80211_tx_info *info = NULL;
	struct sk_buff *pskb = NULL;
	struct rtl_tx_desc *pdesc = NULL;
	struct rtl_tcb_desc tcb_desc;
	u8 temp_one = 1;

	memset(&tcb_desc, 0, sizeof(struct rtl_tcb_desc));
	ring = &rtlpci->tx_ring[BEACON_QUEUE];
	pskb = __skb_dequeue(&ring->queue);
	if (pskb) {
		struct rtl_tx_desc *entry = &ring->desc[ring->idx];
		pci_unmap_single(rtlpci->pdev, rtlpriv->cfg->ops->get_desc(
				 (u8 *) entry, true, HW_DESC_TXBUFF_ADDR),
				 pskb->len, PCI_DMA_TODEVICE);
		kfree_skb(pskb);
	}

	/*NB: the beacon data buffer must be 32-bit aligned. */
	pskb = ieee80211_beacon_get(hw, mac->vif);
	if (pskb == NULL)
		return;
	hdr = rtl_get_hdr(pskb);
	info = IEEE80211_SKB_CB(pskb);
	pdesc = &ring->desc[0];
	rtlpriv->cfg->ops->fill_tx_desc(hw, hdr, (u8 *) pdesc,
		info, NULL, pskb, BEACON_QUEUE, &tcb_desc);

	__skb_queue_tail(&ring->queue, pskb);

	rtlpriv->cfg->ops->set_desc((u8 *) pdesc, true, HW_DESC_OWN,
				    &temp_one);

	return;
}

static void rtl_lps_change_work_callback(struct work_struct *work)
{
	struct rtl_works *rtlworks =
	    container_of(work, struct rtl_works, lps_change_work);
	struct ieee80211_hw *hw = rtlworks->hw;
	struct rtl_priv *rtlpriv = rtl_priv(hw);

	if (rtlpriv->enter_ps)
		rtl_lps_enter(hw);
	else
		rtl_lps_leave(hw);
}

static void _rtl_pci_init_trx_var(struct ieee80211_hw *hw)
{
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	u8 i;

	for (i = 0; i < RTL_PCI_MAX_TX_QUEUE_COUNT; i++)
		rtlpci->txringcount[i] = RT_TXDESC_NUM;

	/*
	 *we just alloc 2 desc for beacon queue,
	 *because we just need first desc in hw beacon.
	 */
	rtlpci->txringcount[BEACON_QUEUE] = 2;

	/*
	 *BE queue need more descriptor for performance
	 *consideration or, No more tx desc will happen,
	 *and may cause mac80211 mem leakage.
	 */
	rtlpci->txringcount[BE_QUEUE] = RT_TXDESC_NUM_BE_QUEUE;

	rtlpci->rxbuffersize = 9100;	/*2048/1024; */
	rtlpci->rxringcount = RTL_PCI_MAX_RX_COUNT;	/*64; */
}

static void _rtl_pci_init_struct(struct ieee80211_hw *hw,
		struct pci_dev *pdev)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));

	rtlpci->up_first_time = true;
	rtlpci->being_init_adapter = false;

	rtlhal->hw = hw;
	rtlpci->pdev = pdev;

	/*Tx/Rx related var */
	_rtl_pci_init_trx_var(hw);

	/*IBSS*/ mac->beacon_interval = 100;

	/*AMPDU*/
	mac->min_space_cfg = 0;
	mac->max_mss_density = 0;
	/*set sane AMPDU defaults */
	mac->current_ampdu_density = 7;
	mac->current_ampdu_factor = 3;

	/*QOS*/
	rtlpci->acm_method = eAcmWay2_SW;

	/*task */
	tasklet_init(&rtlpriv->works.irq_tasklet,
		     (void (*)(unsigned long))_rtl_pci_irq_tasklet,
		     (unsigned long)hw);
	tasklet_init(&rtlpriv->works.irq_prepare_bcn_tasklet,
		     (void (*)(unsigned long))_rtl_pci_prepare_bcn_tasklet,
		     (unsigned long)hw);
	INIT_WORK(&rtlpriv->works.lps_change_work,
		  rtl_lps_change_work_callback);
}

static int _rtl_pci_init_tx_ring(struct ieee80211_hw *hw,
				 unsigned int prio, unsigned int entries)
{
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_tx_desc *ring;
	dma_addr_t dma;
	u32 nextdescaddress;
	int i;

	ring = pci_alloc_consistent(rtlpci->pdev,
				    sizeof(*ring) * entries, &dma);

	if (!ring || (unsigned long)ring & 0xFF) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
			 "Cannot allocate TX ring (prio = %d)\n", prio);
		return -ENOMEM;
	}

	memset(ring, 0, sizeof(*ring) * entries);
	rtlpci->tx_ring[prio].desc = ring;
	rtlpci->tx_ring[prio].dma = dma;
	rtlpci->tx_ring[prio].idx = 0;
	rtlpci->tx_ring[prio].entries = entries;
	skb_queue_head_init(&rtlpci->tx_ring[prio].queue);

	RT_TRACE(rtlpriv, COMP_INIT, DBG_LOUD, "queue:%d, ring_addr:%p\n",
		 prio, ring);

	for (i = 0; i < entries; i++) {
		nextdescaddress = (u32) dma +
					      ((i + 1) % entries) *
					      sizeof(*ring);

		rtlpriv->cfg->ops->set_desc((u8 *)&(ring[i]),
					    true, HW_DESC_TX_NEXTDESC_ADDR,
					    (u8 *)&nextdescaddress);
	}

	return 0;
}

static int _rtl_pci_init_rx_ring(struct ieee80211_hw *hw)
{
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_rx_desc *entry = NULL;
	int i, rx_queue_idx;
	u8 tmp_one = 1;

	/*
	 *rx_queue_idx 0:RX_MPDU_QUEUE
	 *rx_queue_idx 1:RX_CMD_QUEUE
	 */
	for (rx_queue_idx = 0; rx_queue_idx < RTL_PCI_MAX_RX_QUEUE;
	     rx_queue_idx++) {
		rtlpci->rx_ring[rx_queue_idx].desc =
		    pci_alloc_consistent(rtlpci->pdev,
					 sizeof(*rtlpci->rx_ring[rx_queue_idx].
						desc) * rtlpci->rxringcount,
					 &rtlpci->rx_ring[rx_queue_idx].dma);

		if (!rtlpci->rx_ring[rx_queue_idx].desc ||
		    (unsigned long)rtlpci->rx_ring[rx_queue_idx].desc & 0xFF) {
			RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
				 "Cannot allocate RX ring\n");
			return -ENOMEM;
		}

		memset(rtlpci->rx_ring[rx_queue_idx].desc, 0,
		       sizeof(*rtlpci->rx_ring[rx_queue_idx].desc) *
		       rtlpci->rxringcount);

		rtlpci->rx_ring[rx_queue_idx].idx = 0;

		/* If amsdu_8k is disabled, set buffersize to 4096. This
		 * change will reduce memory fragmentation.
		 */
		if (rtlpci->rxbuffersize > 4096 &&
		    rtlpriv->rtlhal.disable_amsdu_8k)
			rtlpci->rxbuffersize = 4096;

		for (i = 0; i < rtlpci->rxringcount; i++) {
			struct sk_buff *skb =
			    dev_alloc_skb(rtlpci->rxbuffersize);
			u32 bufferaddress;
			if (!skb)
				return 0;
			kmemleak_not_leak(skb);
			entry = &rtlpci->rx_ring[rx_queue_idx].desc[i];

			/*skb->dev = dev; */

			rtlpci->rx_ring[rx_queue_idx].rx_buf[i] = skb;

			/*
			 *just set skb->cb to mapping addr
			 *for pci_unmap_single use
			 */
			*((dma_addr_t *) skb->cb) =
			    pci_map_single(rtlpci->pdev, skb_tail_pointer(skb),
					   rtlpci->rxbuffersize,
					   PCI_DMA_FROMDEVICE);

			bufferaddress = (*((dma_addr_t *)skb->cb));
			if (pci_dma_mapping_error(rtlpci->pdev, bufferaddress)) {
				dev_kfree_skb_any(skb);
				return 1;
			}