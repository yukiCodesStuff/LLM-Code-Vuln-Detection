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

static void ixgbe_check_for_bad_vf(struct ixgbe_adapter *adapter)
{
	struct ixgbe_hw *hw = &adapter->hw;
	struct pci_dev *pdev = adapter->pdev;
			continue;
		pci_read_config_word(vfdev, PCI_STATUS, &status_reg);
		if (status_reg != IXGBE_FAILED_READ_CFG_WORD &&
		    status_reg & PCI_STATUS_REC_MASTER_ABORT) {
			ixgbe_bad_vf_abort(adapter, vf);
			pcie_flr(vfdev);
		}
	}
}

static void ixgbe_spoof_check(struct ixgbe_adapter *adapter)
	if (err)
		goto err_sw_init;

	if (adapter->hw.mac.type == ixgbe_mac_82599EB)
		adapter->flags2 |= IXGBE_FLAG2_AUTO_DISABLE_VF;

	switch (adapter->hw.mac.type) {
	case ixgbe_mac_X550:
	case ixgbe_mac_X550EM_x:
		netdev->udp_tunnel_nic_info = &ixgbe_udp_tunnels_x550;