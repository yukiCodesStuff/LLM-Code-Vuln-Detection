		static const struct pch_gbe_option opt = {
			.type = range_option,
			.name = "Transmit Descriptors",
			.err  = "using default of "
				__MODULE_STRING(PCH_GBE_DEFAULT_TXD),
			.def  = PCH_GBE_DEFAULT_TXD,
			.arg  = { .r = { .min = PCH_GBE_MIN_TXD } },
			.arg  = { .r = { .max = PCH_GBE_MAX_TXD } }
		};
		struct pch_gbe_tx_ring *tx_ring = adapter->tx_ring;
		tx_ring->count = TxDescriptors;
		pch_gbe_validate_option(&tx_ring->count, &opt, adapter);
		tx_ring->count = roundup(tx_ring->count,
					PCH_GBE_TX_DESC_MULTIPLE);
	}
	{ /* Receive Descriptor Count */
		static const struct pch_gbe_option opt = {
			.type = range_option,
			.name = "Receive Descriptors",
			.err  = "using default of "
				__MODULE_STRING(PCH_GBE_DEFAULT_RXD),
			.def  = PCH_GBE_DEFAULT_RXD,
			.arg  = { .r = { .min = PCH_GBE_MIN_RXD } },
			.arg  = { .r = { .max = PCH_GBE_MAX_RXD } }
		};
		struct pch_gbe_rx_ring *rx_ring = adapter->rx_ring;
		rx_ring->count = RxDescriptors;
		pch_gbe_validate_option(&rx_ring->count, &opt, adapter);
		rx_ring->count = roundup(rx_ring->count,
				PCH_GBE_RX_DESC_MULTIPLE);
	}
	{ /* Checksum Offload Enable/Disable */
		static const struct pch_gbe_option opt = {
			.type = enable_option,
			.name = "Checksum Offload",
			.err  = "defaulting to Enabled",
			.def  = PCH_GBE_DEFAULT_RX_CSUM
		};
		adapter->rx_csum = XsumRX;
		pch_gbe_validate_option((int *)(&adapter->rx_csum),
					&opt, adapter);
	}
	{ /* Checksum Offload Enable/Disable */
		static const struct pch_gbe_option opt = {
			.type = enable_option,
			.name = "Checksum Offload",
			.err  = "defaulting to Enabled",
			.def  = PCH_GBE_DEFAULT_TX_CSUM
		};
		adapter->tx_csum = XsumTX;
		pch_gbe_validate_option((int *)(&adapter->tx_csum),
						&opt, adapter);
	}
	{ /* Flow Control */
		static const struct pch_gbe_option opt = {
			.type = list_option,
			.name = "Flow Control",
			.err  = "reading default settings from EEPROM",
			.def  = PCH_GBE_FC_DEFAULT,
			.arg  = { .l = { .nr = (int)ARRAY_SIZE(fc_list),
					 .p = fc_list } }
		};
		hw->mac.fc = FlowControl;
		pch_gbe_validate_option((int *)(&hw->mac.fc),
						&opt, adapter);
	}

	pch_gbe_check_copper_options(adapter);
}
		static const struct pch_gbe_option opt = {
			.type = range_option,
			.name = "Receive Descriptors",
			.err  = "using default of "
				__MODULE_STRING(PCH_GBE_DEFAULT_RXD),
			.def  = PCH_GBE_DEFAULT_RXD,
			.arg  = { .r = { .min = PCH_GBE_MIN_RXD } },
			.arg  = { .r = { .max = PCH_GBE_MAX_RXD } }
		};
		struct pch_gbe_rx_ring *rx_ring = adapter->rx_ring;
		rx_ring->count = RxDescriptors;
		pch_gbe_validate_option(&rx_ring->count, &opt, adapter);
		rx_ring->count = roundup(rx_ring->count,
				PCH_GBE_RX_DESC_MULTIPLE);
	}
	{ /* Checksum Offload Enable/Disable */
		static const struct pch_gbe_option opt = {
			.type = enable_option,
			.name = "Checksum Offload",
			.err  = "defaulting to Enabled",
			.def  = PCH_GBE_DEFAULT_RX_CSUM
		};
		adapter->rx_csum = XsumRX;
		pch_gbe_validate_option((int *)(&adapter->rx_csum),
					&opt, adapter);
	}
	{ /* Checksum Offload Enable/Disable */
		static const struct pch_gbe_option opt = {
			.type = enable_option,
			.name = "Checksum Offload",
			.err  = "defaulting to Enabled",
			.def  = PCH_GBE_DEFAULT_TX_CSUM
		};
		adapter->tx_csum = XsumTX;
		pch_gbe_validate_option((int *)(&adapter->tx_csum),
						&opt, adapter);
	}
	{ /* Flow Control */
		static const struct pch_gbe_option opt = {
			.type = list_option,
			.name = "Flow Control",
			.err  = "reading default settings from EEPROM",
			.def  = PCH_GBE_FC_DEFAULT,
			.arg  = { .l = { .nr = (int)ARRAY_SIZE(fc_list),
					 .p = fc_list } }
		};
		hw->mac.fc = FlowControl;
		pch_gbe_validate_option((int *)(&hw->mac.fc),
						&opt, adapter);
	}

	pch_gbe_check_copper_options(adapter);
}