	struct rtl_stats stats = {
		.signal = 0,
		.rate = 0,
	};
	int index = rtlpci->rx_ring[rx_queue_idx].idx;

	if (rtlpci->driver_is_goingto_unload)
		return;
	/*RX NORMAL PKT */
	while (count--) {
		/*rx descriptor */
		struct rtl_rx_desc *pdesc = &rtlpci->rx_ring[rx_queue_idx].desc[
				index];
		/*rx pkt */
		struct sk_buff *skb = rtlpci->rx_ring[rx_queue_idx].rx_buf[
				index];
		struct sk_buff *new_skb = NULL;

		own = (u8) rtlpriv->cfg->ops->get_desc((u8 *) pdesc,
						       false, HW_DESC_OWN);

		/*wait data to be filled by hardware */
		if (own)
			break;

		rtlpriv->cfg->ops->query_rx_desc(hw, &stats,
						 &rx_status,
						 (u8 *) pdesc, skb);

		if (stats.crc || stats.hwerror)
			goto done;

		new_skb = dev_alloc_skb(rtlpci->rxbuffersize);
		if (unlikely(!new_skb)) {
			RT_TRACE(rtlpriv, (COMP_INTR | COMP_RECV), DBG_DMESG,
				 "can't alloc skb for rx\n");
			goto done;
		}
		kmemleak_not_leak(new_skb);

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
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_ps_ctl *ppsc = rtl_psc(rtl_priv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	unsigned long flags;
	u8 RFInProgressTimeOut = 0;

	/*
	 *should be before disable interrupt&adapter
	 *and will do it immediately.
	 */
	set_hal_stop(rtlhal);

	rtlpci->driver_is_goingto_unload = true;
	rtlpriv->cfg->ops->disable_interrupt(hw);
	cancel_work_sync(&rtlpriv->works.lps_change_work);

	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flags);
	while (ppsc->rfchange_inprogress) {
		spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flags);
		if (RFInProgressTimeOut > 100) {
			spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flags);
			break;
		}
		mdelay(1);
		RFInProgressTimeOut++;
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flags);
	}
		if (RFInProgressTimeOut > 100) {
			spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flags);
			break;
		}
		mdelay(1);
		RFInProgressTimeOut++;
		spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flags);
	}
	ppsc->rfchange_inprogress = true;
	spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flags);

	rtlpriv->cfg->ops->hw_disable(hw);
	/* some things are not needed if firmware not available */
	if (!rtlpriv->max_fw_size)
		return;
	rtlpriv->cfg->ops->led_control(hw, LED_CTL_POWER_OFF);

	spin_lock_irqsave(&rtlpriv->locks.rf_ps_lock, flags);
	ppsc->rfchange_inprogress = false;
	spin_unlock_irqrestore(&rtlpriv->locks.rf_ps_lock, flags);

	rtl_pci_enable_aspm(hw);
}

static bool _rtl_pci_find_adapter(struct pci_dev *pdev,
		struct ieee80211_hw *hw)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_pci_priv *pcipriv = rtl_pcipriv(hw);
	struct rtl_hal *rtlhal = rtl_hal(rtl_priv(hw));
	struct pci_dev *bridge_pdev = pdev->bus->self;
	u16 venderid;
	u16 deviceid;
	u8 revisionid;
	u16 irqline;
	u8 tmp;

	pcipriv->ndis_adapter.pcibridge_vendor = PCI_BRIDGE_VENDOR_UNKNOWN;
	venderid = pdev->vendor;
	deviceid = pdev->device;
	pci_read_config_byte(pdev, 0x8, &revisionid);
	pci_read_config_word(pdev, 0x3C, &irqline);

	/* PCI ID 0x10ec:0x8192 occurs for both RTL8192E, which uses
	 * r8192e_pci, and RTL8192SE, which uses this driver. If the
	 * revision ID is RTL_PCI_REVISION_ID_8192PCIE (0x01), then
	 * the correct driver is r8192e_pci, thus this routine should
	 * return false.
	 */
	if (deviceid == RTL_PCI_8192SE_DID &&
	    revisionid == RTL_PCI_REVISION_ID_8192PCIE)
		return false;

	if (deviceid == RTL_PCI_8192_DID ||
	    deviceid == RTL_PCI_0044_DID ||
	    deviceid == RTL_PCI_0047_DID ||
	    deviceid == RTL_PCI_8192SE_DID ||
	    deviceid == RTL_PCI_8174_DID ||
	    deviceid == RTL_PCI_8173_DID ||
	    deviceid == RTL_PCI_8172_DID ||
	    deviceid == RTL_PCI_8171_DID) {
		switch (revisionid) {
		case RTL_PCI_REVISION_ID_8192PCIE:
			RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
				 "8192 PCI-E is found - vid/did=%x/%x\n",
				 venderid, deviceid);
			rtlhal->hw_type = HARDWARE_TYPE_RTL8192E;
			return false;
		case RTL_PCI_REVISION_ID_8192SE:
			RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
				 "8192SE is found - vid/did=%x/%x\n",
				 venderid, deviceid);
			rtlhal->hw_type = HARDWARE_TYPE_RTL8192SE;
			break;
		default:
			RT_TRACE(rtlpriv, COMP_ERR, DBG_WARNING,
				 "Err: Unknown device - vid/did=%x/%x\n",
				 venderid, deviceid);
			rtlhal->hw_type = HARDWARE_TYPE_RTL8192SE;
			break;

		}
	} else if (deviceid == RTL_PCI_8723AE_DID) {
		rtlhal->hw_type = HARDWARE_TYPE_RTL8723AE;
		RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
			 "8723AE PCI-E is found - "
			 "vid/did=%x/%x\n", venderid, deviceid);
	} else if (deviceid == RTL_PCI_8192CET_DID ||
		   deviceid == RTL_PCI_8192CE_DID ||
		   deviceid == RTL_PCI_8191CE_DID ||
		   deviceid == RTL_PCI_8188CE_DID) {
		rtlhal->hw_type = HARDWARE_TYPE_RTL8192CE;
		RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
			 "8192C PCI-E is found - vid/did=%x/%x\n",
			 venderid, deviceid);
	} else if (deviceid == RTL_PCI_8192DE_DID ||
		   deviceid == RTL_PCI_8192DE_DID2) {
		rtlhal->hw_type = HARDWARE_TYPE_RTL8192DE;
		RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
			 "8192D PCI-E is found - vid/did=%x/%x\n",
			 venderid, deviceid);
	} else if (deviceid == RTL_PCI_8188EE_DID) {
		rtlhal->hw_type = HARDWARE_TYPE_RTL8188EE;
		RT_TRACE(rtlpriv, COMP_INIT, DBG_LOUD,
			 "Find adapter, Hardware type is 8188EE\n");
	} else {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_WARNING,
			 "Err: Unknown device - vid/did=%x/%x\n",
			 venderid, deviceid);

		rtlhal->hw_type = RTL_DEFAULT_HARDWARE_TYPE;
	}

	if (rtlhal->hw_type == HARDWARE_TYPE_RTL8192DE) {
		if (revisionid == 0 || revisionid == 1) {
			if (revisionid == 0) {
				RT_TRACE(rtlpriv, COMP_INIT, DBG_LOUD,
					 "Find 92DE MAC0\n");
				rtlhal->interfaceindex = 0;
			} else if (revisionid == 1) {
				RT_TRACE(rtlpriv, COMP_INIT, DBG_LOUD,
					 "Find 92DE MAC1\n");
				rtlhal->interfaceindex = 1;
			}
		} else {
			RT_TRACE(rtlpriv, COMP_INIT, DBG_LOUD,
				 "Unknown device - VendorID/DeviceID=%x/%x, Revision=%x\n",
				 venderid, deviceid, revisionid);
			rtlhal->interfaceindex = 0;
		}
	}
	/*find bus info */
	pcipriv->ndis_adapter.busnumber = pdev->bus->number;
	pcipriv->ndis_adapter.devnumber = PCI_SLOT(pdev->devfn);
	pcipriv->ndis_adapter.funcnumber = PCI_FUNC(pdev->devfn);

	/* some ARM have no bridge_pdev and will crash here
	 * so we should check if bridge_pdev is NULL
	 */
	if (bridge_pdev) {
		/*find bridge info if available */
		pcipriv->ndis_adapter.pcibridge_vendorid = bridge_pdev->vendor;
		for (tmp = 0; tmp < PCI_BRIDGE_VENDOR_MAX; tmp++) {
			if (bridge_pdev->vendor == pcibridge_vendors[tmp]) {
				pcipriv->ndis_adapter.pcibridge_vendor = tmp;
				RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
					 "Pci Bridge Vendor is found index: %d\n",
					 tmp);
				break;
			}
		}
	}

	if (pcipriv->ndis_adapter.pcibridge_vendor !=
		PCI_BRIDGE_VENDOR_UNKNOWN) {
		pcipriv->ndis_adapter.pcibridge_busnum =
		    bridge_pdev->bus->number;
		pcipriv->ndis_adapter.pcibridge_devnum =
		    PCI_SLOT(bridge_pdev->devfn);
		pcipriv->ndis_adapter.pcibridge_funcnum =
		    PCI_FUNC(bridge_pdev->devfn);
		pcipriv->ndis_adapter.pcibridge_pciehdr_offset =
		    pci_pcie_cap(bridge_pdev);
		pcipriv->ndis_adapter.num4bytes =
		    (pcipriv->ndis_adapter.pcibridge_pciehdr_offset + 0x10) / 4;

		rtl_pci_get_linkcontrol_field(hw);

		if (pcipriv->ndis_adapter.pcibridge_vendor ==
		    PCI_BRIDGE_VENDOR_AMD) {
			pcipriv->ndis_adapter.amd_l1_patch =
			    rtl_pci_get_amd_l1_patch(hw);
		}
	}

	RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
		 "pcidev busnumber:devnumber:funcnumber:vendor:link_ctl %d:%d:%d:%x:%x\n",
		 pcipriv->ndis_adapter.busnumber,
		 pcipriv->ndis_adapter.devnumber,
		 pcipriv->ndis_adapter.funcnumber,
		 pdev->vendor, pcipriv->ndis_adapter.linkctrl_reg);

	RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
		 "pci_bridge busnumber:devnumber:funcnumber:vendor:pcie_cap:link_ctl_reg:amd %d:%d:%d:%x:%x:%x:%x\n",
		 pcipriv->ndis_adapter.pcibridge_busnum,
		 pcipriv->ndis_adapter.pcibridge_devnum,
		 pcipriv->ndis_adapter.pcibridge_funcnum,
		 pcibridge_vendors[pcipriv->ndis_adapter.pcibridge_vendor],
		 pcipriv->ndis_adapter.pcibridge_pciehdr_offset,
		 pcipriv->ndis_adapter.pcibridge_linkctrlreg,
		 pcipriv->ndis_adapter.amd_l1_patch);

	rtl_pci_parse_configuration(pdev, hw);
	list_add_tail(&rtlpriv->list, &rtlpriv->glb_var->glb_priv_list);

	return true;
}

int rtl_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *id)
{
	struct ieee80211_hw *hw = NULL;

	struct rtl_priv *rtlpriv = NULL;
	struct rtl_pci_priv *pcipriv = NULL;
	struct rtl_pci *rtlpci;
	unsigned long pmem_start, pmem_len, pmem_flags;
	int err;

	err = pci_enable_device(pdev);
	if (err) {
		RT_ASSERT(false, "%s : Cannot enable new PCI device\n",
			  pci_name(pdev));
		return err;
	}

	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
		if (pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32))) {
			RT_ASSERT(false,
				  "Unable to obtain 32bit DMA for consistent allocations\n");
			err = -ENOMEM;
			goto fail1;
		}
	}

	pci_set_master(pdev);

	hw = ieee80211_alloc_hw(sizeof(struct rtl_pci_priv) +
				sizeof(struct rtl_priv), &rtl_ops);
	if (!hw) {
		RT_ASSERT(false,
			  "%s : ieee80211 alloc failed\n", pci_name(pdev));
		err = -ENOMEM;
		goto fail1;
	}

	SET_IEEE80211_DEV(hw, &pdev->dev);
	pci_set_drvdata(pdev, hw);

	rtlpriv = hw->priv;
	rtlpriv->hw = hw;
	pcipriv = (void *)rtlpriv->priv;
	pcipriv->dev.pdev = pdev;
	init_completion(&rtlpriv->firmware_loading_complete);

	/* init cfg & intf_ops */
	rtlpriv->rtlhal.interface = INTF_PCI;
	rtlpriv->cfg = (struct rtl_hal_cfg *)(id->driver_data);
	rtlpriv->intf_ops = &rtl_pci_ops;
	rtlpriv->glb_var = &rtl_global_var;

	/*
	 *init dbgp flags before all
	 *other functions, because we will
	 *use it in other funtions like
	 *RT_TRACE/RT_PRINT/RTL_PRINT_DATA
	 *you can not use these macro
	 *before this
	 */
	rtl_dbgp_flag_init(hw);

	/* MEM map */
	err = pci_request_regions(pdev, KBUILD_MODNAME);
	if (err) {
		RT_ASSERT(false, "Can't obtain PCI resources\n");
		goto fail1;
	}

	pmem_start = pci_resource_start(pdev, rtlpriv->cfg->bar_id);
	pmem_len = pci_resource_len(pdev, rtlpriv->cfg->bar_id);
	pmem_flags = pci_resource_flags(pdev, rtlpriv->cfg->bar_id);

	/*shared mem start */
	rtlpriv->io.pci_mem_start =
			(unsigned long)pci_iomap(pdev,
			rtlpriv->cfg->bar_id, pmem_len);
	if (rtlpriv->io.pci_mem_start == 0) {
		RT_ASSERT(false, "Can't map PCI mem\n");
		err = -ENOMEM;
		goto fail2;
	}

	RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
		 "mem mapped space: start: 0x%08lx len:%08lx flags:%08lx, after map:0x%08lx\n",
		 pmem_start, pmem_len, pmem_flags,
		 rtlpriv->io.pci_mem_start);

	/* Disable Clk Request */
	pci_write_config_byte(pdev, 0x81, 0);
	/* leave D3 mode */
	pci_write_config_byte(pdev, 0x44, 0);
	pci_write_config_byte(pdev, 0x04, 0x06);
	pci_write_config_byte(pdev, 0x04, 0x07);

	/* find adapter */
	if (!_rtl_pci_find_adapter(pdev, hw)) {
		err = -ENODEV;
		goto fail3;
	}

	/* Init IO handler */
	_rtl_pci_io_handler_init(&pdev->dev, hw);

	/*like read eeprom and so on */
	rtlpriv->cfg->ops->read_eeprom_info(hw);

	/*aspm */
	rtl_pci_init_aspm(hw);

	/* Init mac80211 sw */
	err = rtl_init_core(hw);
	if (err) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
			 "Can't allocate sw for mac80211\n");
		goto fail3;
	}

	/* Init PCI sw */
	err = rtl_pci_init(hw, pdev);
	if (err) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG, "Failed to init PCI\n");
		goto fail3;
	}

	if (rtlpriv->cfg->ops->init_sw_vars(hw)) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG, "Can't init_sw_vars\n");
		err = -ENODEV;
		goto fail3;
	}

	rtlpriv->cfg->ops->init_sw_leds(hw);

	err = sysfs_create_group(&pdev->dev.kobj, &rtl_attribute_group);
	if (err) {
		RT_TRACE(rtlpriv, COMP_ERR, DBG_EMERG,
			 "failed to create sysfs device attributes\n");
		goto fail3;
	}

	rtlpci = rtl_pcidev(pcipriv);
	err = request_irq(rtlpci->pdev->irq, &_rtl_pci_interrupt,
			  IRQF_SHARED, KBUILD_MODNAME, hw);
	if (err) {
		RT_TRACE(rtlpriv, COMP_INIT, DBG_DMESG,
			 "%s: failed to register IRQ handler\n",
			 wiphy_name(hw->wiphy));
		goto fail3;
	}
	rtlpci->irq_alloc = 1;

	return 0;

fail3:
	rtl_deinit_core(hw);

	if (rtlpriv->io.pci_mem_start != 0)
		pci_iounmap(pdev, (void __iomem *)rtlpriv->io.pci_mem_start);

fail2:
	pci_release_regions(pdev);
	complete(&rtlpriv->firmware_loading_complete);

fail1:
	if (hw)
		ieee80211_free_hw(hw);
	pci_disable_device(pdev);

	return err;

}
EXPORT_SYMBOL(rtl_pci_probe);

void rtl_pci_disconnect(struct pci_dev *pdev)
{
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct rtl_pci_priv *pcipriv = rtl_pcipriv(hw);
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_pci *rtlpci = rtl_pcidev(pcipriv);
	struct rtl_mac *rtlmac = rtl_mac(rtlpriv);

	/* just in case driver is removed before firmware callback */
	wait_for_completion(&rtlpriv->firmware_loading_complete);
	clear_bit(RTL_STATUS_INTERFACE_START, &rtlpriv->status);

	sysfs_remove_group(&pdev->dev.kobj, &rtl_attribute_group);

	/*ieee80211_unregister_hw will call ops_stop */
	if (rtlmac->mac80211_registered == 1) {
		ieee80211_unregister_hw(hw);
		rtlmac->mac80211_registered = 0;
	} else {
		rtl_deinit_deferred_work(hw);
		rtlpriv->intf_ops->adapter_stop(hw);
	}
	rtlpriv->cfg->ops->disable_interrupt(hw);

	/*deinit rfkill */
	rtl_deinit_rfkill(hw);

	rtl_pci_deinit(hw);
	rtl_deinit_core(hw);
	rtlpriv->cfg->ops->deinit_sw_vars(hw);

	if (rtlpci->irq_alloc) {
		synchronize_irq(rtlpci->pdev->irq);
		free_irq(rtlpci->pdev->irq, hw);
		rtlpci->irq_alloc = 0;
	}

	list_del(&rtlpriv->list);
	if (rtlpriv->io.pci_mem_start != 0) {
		pci_iounmap(pdev, (void __iomem *)rtlpriv->io.pci_mem_start);
		pci_release_regions(pdev);
	}

	pci_disable_device(pdev);

	rtl_pci_disable_aspm(hw);

	ieee80211_free_hw(hw);
}
EXPORT_SYMBOL(rtl_pci_disconnect);

#ifdef CONFIG_PM_SLEEP
/***************************************
kernel pci power state define:
PCI_D0         ((pci_power_t __force) 0)
PCI_D1         ((pci_power_t __force) 1)
PCI_D2         ((pci_power_t __force) 2)
PCI_D3hot      ((pci_power_t __force) 3)
PCI_D3cold     ((pci_power_t __force) 4)
PCI_UNKNOWN    ((pci_power_t __force) 5)

This function is called when system
goes into suspend state mac80211 will
call rtl_mac_stop() from the mac80211
suspend function first, So there is
no need to call hw_disable here.
****************************************/
int rtl_pci_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct rtl_priv *rtlpriv = rtl_priv(hw);

	rtlpriv->cfg->ops->hw_suspend(hw);
	rtl_deinit_rfkill(hw);

	return 0;
}
EXPORT_SYMBOL(rtl_pci_suspend);

int rtl_pci_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct rtl_priv *rtlpriv = rtl_priv(hw);

	rtlpriv->cfg->ops->hw_resume(hw);
	rtl_init_rfkill(hw);
	return 0;
}
EXPORT_SYMBOL(rtl_pci_resume);
#endif /* CONFIG_PM_SLEEP */

struct rtl_intf_ops rtl_pci_ops = {
	.read_efuse_byte = read_efuse_byte,
	.adapter_start = rtl_pci_start,
	.adapter_stop = rtl_pci_stop,
	.check_buddy_priv = rtl_pci_check_buddy_priv,
	.adapter_tx = rtl_pci_tx,
	.flush = rtl_pci_flush,
	.reset_trx_ring = rtl_pci_reset_trx_ring,
	.waitq_insert = rtl_pci_tx_chk_waitq_insert,

	.disable_aspm = rtl_pci_disable_aspm,
	.enable_aspm = rtl_pci_enable_aspm,
};