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
static void ixgbe_bad_vf_abort(struct ixgbe_adapter *adapter, u32 vf)
{
	struct ixgbe_hw *hw = &adapter->hw;

	if (adapter->hw.mac.type == ixgbe_mac_82599EB &&
	    adapter->flags2 & IXGBE_FLAG2_AUTO_DISABLE_VF) {
		adapter->vfinfo[vf].primary_abort_count++;
		if (adapter->vfinfo[vf].primary_abort_count ==
		    IXGBE_PRIMARY_ABORT_LIMIT) {
			ixgbe_set_vf_link_state(adapter, vf,
						IFLA_VF_LINK_STATE_DISABLE);
			adapter->vfinfo[vf].primary_abort_count = 0;

			e_info(drv,
			       "Malicious Driver Detection event detected on PF %d VF %d MAC: %pM mdd-disable-vf=on",
			       hw->bus.func, vf,
			       adapter->vfinfo[vf].vf_mac_addresses);
		}
	}
}
	for (vf = 0; vf < adapter->num_vfs; ++vf) {
		struct pci_dev *vfdev = adapter->vfinfo[vf].vfdev;
		u16 status_reg;

		if (!vfdev)
			continue;
		pci_read_config_word(vfdev, PCI_STATUS, &status_reg);
		if (status_reg != IXGBE_FAILED_READ_CFG_WORD &&
		    status_reg & PCI_STATUS_REC_MASTER_ABORT) {
			ixgbe_bad_vf_abort(adapter, vf);
			pcie_flr(vfdev);
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

	if (adapter->hw.mac.type == ixgbe_mac_82599EB)
		adapter->flags2 |= IXGBE_FLAG2_AUTO_DISABLE_VF;

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