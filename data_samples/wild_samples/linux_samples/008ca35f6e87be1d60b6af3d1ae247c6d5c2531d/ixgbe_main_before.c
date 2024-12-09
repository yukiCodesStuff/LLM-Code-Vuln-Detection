		    ixgbe_vf_tx_pending(adapter)) {
			/* We've lost link, so the controller stops DMA,
			 * but we've got queued Tx work that's never going
			 * to get done, so reset controller to flush Tx.
			 * (Do the reset outside of interrupt context).
			 */
			e_warn(drv, "initiating reset to clear Tx work after link loss\n");
			set_bit(__IXGBE_RESET_REQUESTED, &adapter->state);
		}
	}
}

#ifdef CONFIG_PCI_IOV
static void ixgbe_check_for_bad_vf(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct pci_dev *pdev = adapter->pdev;
	unsigned int vf;
	u32 gpc;

	if (!(netif_carrier_ok(adapter->netdev)))
		return;

	gpc = IXGBE_READ_REG(hw, IXGBE_TXDGPC);
	if (gpc) /* If incrementing then no need for the check below */
		return;
	/* Check to see if a bad DMA write target from an errant or
	 * malicious VF has caused a PCIe error.  If so then we can
	 * issue a VFLR to the offending VF(s) and then resume without
	 * requesting a full slot reset.
	 */

	if (!pdev)
		return;

	/* check status reg for all VFs owned by this PF */
	for (vf = 0; vf < adapter->num_vfs; ++vf) {
		struct pci_dev *vfdev = adapter->vfinfo[vf].vfdev;
		u16 status_reg;

		if (!vfdev)
			continue;
		pci_read_config_word(vfdev, PCI_STATUS, &status_reg);
		if (status_reg != IXGBE_FAILED_READ_CFG_WORD &&
		    status_reg & PCI_STATUS_REC_MASTER_ABORT)
			pcie_flr(vfdev);
	}
}
	for (vf = 0; vf < adapter->num_vfs; ++vf) {
		struct pci_dev *vfdev = adapter->vfinfo[vf].vfdev;
		u16 status_reg;

		if (!vfdev)
			continue;
		pci_read_config_word(vfdev, PCI_STATUS, &status_reg);
		if (status_reg != IXGBE_FAILED_READ_CFG_WORD &&
		    status_reg & PCI_STATUS_REC_MASTER_ABORT)
			pcie_flr(vfdev);
	}
}

static void ixgbe_spoof_check(struct ixgbe_adapter *adapter)
{
	u32 ssvpc;

	/* Do not perform spoof check for 82598 or if not in IOV mode */
	if (adapter->hw.mac.type == ixgbe_mac_82598EB ||
	    adapter->num_vfs == 0)
		return;

	ssvpc = IXGBE_READ_REG(&adapter->hw, IXGBE_SSVPC);

	/*
	 * ssvpc register is cleared on read, if zero then no
	 * spoofed packets in the last interval.
	 */
	if (!ssvpc)
		return;

	e_warn(drv, "%u Spoofed packets detected\n", ssvpc);
}
#else
static void ixgbe_spoof_check(struct ixgbe_adapter __always_unused *adapter)
{
}

static void
ixgbe_check_for_bad_vf(struct ixgbe_adapter __always_unused *adapter)
{
}
#endif /* CONFIG_PCI_IOV */


/**
 * ixgbe_watchdog_subtask - check and bring link up
 * @adapter: pointer to the device adapter structure
 **/
static void ixgbe_watchdog_subtask(struct ixgbe_adapter *adapter)
{
	/* if interface is down, removing or resetting, do nothing */
	if (test_bit(__IXGBE_DOWN, &adapter->state) ||
	    test_bit(__IXGBE_REMOVING, &adapter->state) ||
	    test_bit(__IXGBE_RESETTING, &adapter->state))
		return;

	ixgbe_watchdog_update_link(adapter);

	if (adapter->link_up)
		ixgbe_watchdog_link_is_up(adapter);
	else
		ixgbe_watchdog_link_is_down(adapter);

	ixgbe_check_for_bad_vf(adapter);
	ixgbe_spoof_check(adapter);
	ixgbe_update_stats(adapter);

	ixgbe_watchdog_flush_tx(adapter);
}

/**
 * ixgbe_sfp_detection_subtask - poll for SFP+ cable
 * @adapter: the ixgbe adapter structure
 **/
static void ixgbe_sfp_detection_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	s32 err;

	/* not searching for SFP so there is nothing to do here */
	if (!(adapter->flags2 & IXGBE_FLAG2_SEARCH_FOR_SFP) &&
	    !(adapter->flags2 & IXGBE_FLAG2_SFP_NEEDS_RESET))
		return;

	if (adapter->sfp_poll_time &&
	    time_after(adapter->sfp_poll_time, jiffies))
		return; /* If not yet time to poll for SFP */

	/* someone else is in init, wait until next service event */
	if (test_and_set_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
		return;

	adapter->sfp_poll_time = jiffies + IXGBE_SFP_POLL_JIFFIES - 1;

	err = hw->phy.ops.identify_sfp(hw);
	if (err == IXGBE_ERR_SFP_NOT_SUPPORTED)
		goto sfp_out;

	if (err == IXGBE_ERR_SFP_NOT_PRESENT) {
		/* If no cable is present, then we need to reset
		 * the next time we find a good cable. */
		adapter->flags2 |= IXGBE_FLAG2_SFP_NEEDS_RESET;
	}

	/* exit on error */
	if (err)
		goto sfp_out;

	/* exit if reset not needed */
	if (!(adapter->flags2 & IXGBE_FLAG2_SFP_NEEDS_RESET))
		goto sfp_out;

	adapter->flags2 &= ~IXGBE_FLAG2_SFP_NEEDS_RESET;

	/*
	 * A module may be identified correctly, but the EEPROM may not have
	 * support for that module.  setup_sfp() will fail in that case, so
	 * we should not allow that module to load.
	 */
	if (hw->mac.type == ixgbe_mac_82598EB)
		err = hw->phy.ops.reset(hw);
	else
		err = hw->mac.ops.setup_sfp(hw);

	if (err == IXGBE_ERR_SFP_NOT_SUPPORTED)
		goto sfp_out;

	adapter->flags |= IXGBE_FLAG_NEED_LINK_CONFIG;
	e_info(probe, "detected SFP+: %d\n", hw->phy.sfp_type);

sfp_out:
	clear_bit(__IXGBE_IN_SFP_INIT, &adapter->state);

	if ((err == IXGBE_ERR_SFP_NOT_SUPPORTED) &&
	    (adapter->netdev->reg_state == NETREG_REGISTERED)) {
		e_dev_err("failed to initialize because an unsupported "
			  "SFP+ module type was detected.\n");
		e_dev_err("Reload the driver after installing a "
			  "supported module.\n");
		unregister_netdev(adapter->netdev);
	}
}

/**
 * ixgbe_sfp_link_config_subtask - set up link SFP after module install
 * @adapter: the ixgbe adapter structure
 **/
static void ixgbe_sfp_link_config_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 cap_speed;
	u32 speed;
	bool autoneg = false;

	if (!(adapter->flags & IXGBE_FLAG_NEED_LINK_CONFIG))
		return;

	/* someone else is in init, wait until next service event */
	if (test_and_set_bit(__IXGBE_IN_SFP_INIT, &adapter->state))
		return;

	adapter->flags &= ~IXGBE_FLAG_NEED_LINK_CONFIG;

	hw->mac.ops.get_link_capabilities(hw, &cap_speed, &autoneg);

	/* advertise highest capable link speed */
	if (!autoneg && (cap_speed & IXGBE_LINK_SPEED_10GB_FULL))
		speed = IXGBE_LINK_SPEED_10GB_FULL;
	else
		speed = cap_speed & (IXGBE_LINK_SPEED_10GB_FULL |
				     IXGBE_LINK_SPEED_1GB_FULL);

	if (hw->mac.ops.setup_link)
		hw->mac.ops.setup_link(hw, speed, true);

	adapter->flags |= IXGBE_FLAG_NEED_LINK_UPDATE;
	adapter->link_check_timeout = jiffies;
	clear_bit(__IXGBE_IN_SFP_INIT, &adapter->state);
}

/**
 * ixgbe_service_timer - Timer Call-back
 * @t: pointer to timer_list structure
 **/
static void ixgbe_service_timer(struct timer_list *t)
{
	struct ixgbe_adapter *adapter = from_timer(adapter, t, service_timer);
	unsigned long next_event_offset;

	/* poll faster when waiting for link */
	if (adapter->flags & IXGBE_FLAG_NEED_LINK_UPDATE)
		next_event_offset = HZ / 10;
	else
		next_event_offset = HZ * 2;

	/* Reset the timer */
	mod_timer(&adapter->service_timer, next_event_offset + jiffies);

	ixgbe_service_event_schedule(adapter);
}

static void ixgbe_phy_interrupt_subtask(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 status;

	if (!(adapter->flags2 & IXGBE_FLAG2_PHY_INTERRUPT))
		return;

	adapter->flags2 &= ~IXGBE_FLAG2_PHY_INTERRUPT;

	if (!hw->phy.ops.handle_lasi)
		return;

	status = hw->phy.ops.handle_lasi(&adapter->hw);
	if (status != IXGBE_ERR_OVERTEMP)
		return;

	e_crit(drv, "%s\n", ixgbe_overheat_msg);
}

static void ixgbe_reset_subtask(struct ixgbe_adapter *adapter)
{
	if (!test_and_clear_bit(__IXGBE_RESET_REQUESTED, &adapter->state))
		return;

	rtnl_lock();
	/* If we're already down, removing or resetting, just bail */
	if (test_bit(__IXGBE_DOWN, &adapter->state) ||
	    test_bit(__IXGBE_REMOVING, &adapter->state) ||
	    test_bit(__IXGBE_RESETTING, &adapter->state)) {
		rtnl_unlock();
		return;
	}

	ixgbe_dump(adapter);
	netdev_err(adapter->netdev, "Reset adapter\n");
	adapter->tx_timeout_count++;

	ixgbe_reinit_locked(adapter);
	rtnl_unlock();
}

/**
 * ixgbe_check_fw_error - Check firmware for errors
 * @adapter: the adapter private structure
 *
 * Check firmware errors in register FWSM
 */
static bool ixgbe_check_fw_error(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	u32 fwsm;

	/* read fwsm.ext_err_ind register and log errors */
	fwsm = IXGBE_READ_REG(hw, IXGBE_FWSM(hw));

	if (fwsm & IXGBE_FWSM_EXT_ERR_IND_MASK ||
	    !(fwsm & IXGBE_FWSM_FW_VAL_BIT))
		e_dev_warn("Warning firmware error detected FWSM: 0x%08X\n",
			   fwsm);

	if (hw->mac.ops.fw_recovery_mode && hw->mac.ops.fw_recovery_mode(hw)) {
		e_dev_err("Firmware recovery mode detected. Limiting functionality. Refer to the Intel(R) Ethernet Adapters and Devices User Guide for details on firmware recovery mode.\n");
		return true;
	}

	return false;
}

/**
 * ixgbe_service_task - manages and runs subtasks
 * @work: pointer to work_struct containing our data
 **/
static void ixgbe_service_task(struct work_struct *work)
{
	struct ixgbe_adapter *adapter = container_of(work,
						     struct ixgbe_adapter,
						     service_task);
	if (ixgbe_removed(adapter->hw.hw_addr)) {
		if (!test_bit(__IXGBE_DOWN, &adapter->state)) {
			rtnl_lock();
			ixgbe_down(adapter);
			rtnl_unlock();
		}
	if (ixgbe_removed(hw->hw_addr)) {
		err = -EIO;
		goto err_ioremap;
	}
	/* If EEPROM is valid (bit 8 = 1), use default otherwise use bit bang */
	if (!(eec & BIT(8)))
		hw->eeprom.ops.read = &ixgbe_read_eeprom_bit_bang_generic;

	/* PHY */
	hw->phy.ops = *ii->phy_ops;
	hw->phy.sfp_type = ixgbe_sfp_type_unknown;
	/* ixgbe_identify_phy_generic will set prtad and mmds properly */
	hw->phy.mdio.prtad = MDIO_PRTAD_NONE;
	hw->phy.mdio.mmds = 0;
	hw->phy.mdio.mode_support = MDIO_SUPPORTS_C45 | MDIO_EMULATE_C22;
	hw->phy.mdio.dev = netdev;
	hw->phy.mdio.mdio_read = ixgbe_mdio_read;
	hw->phy.mdio.mdio_write = ixgbe_mdio_write;

	/* setup the private structure */
	err = ixgbe_sw_init(adapter, ii);
	if (err)
		goto err_sw_init;

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
		netdev->udp_tunnel_nic_info = &ixgbe_udp_tunnels_x550;
		break;
	case ixgbe_mac_x550em_a:
		netdev->udp_tunnel_nic_info = &ixgbe_udp_tunnels_x550em_a;
		break;
	default:
		break;
	}