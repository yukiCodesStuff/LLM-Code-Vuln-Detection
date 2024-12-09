			.err  = "using default of "
				__MODULE_STRING(PCH_GBE_DEFAULT_TXD),
			.def  = PCH_GBE_DEFAULT_TXD,
			.arg  = { .r = { .min = PCH_GBE_MIN_TXD,
					 .max = PCH_GBE_MAX_TXD } }
		};
		struct pch_gbe_tx_ring *tx_ring = adapter->tx_ring;
		tx_ring->count = TxDescriptors;
		pch_gbe_validate_option(&tx_ring->count, &opt, adapter);
			.err  = "using default of "
				__MODULE_STRING(PCH_GBE_DEFAULT_RXD),
			.def  = PCH_GBE_DEFAULT_RXD,
			.arg  = { .r = { .min = PCH_GBE_MIN_RXD,
					 .max = PCH_GBE_MAX_RXD } }
		};
		struct pch_gbe_rx_ring *rx_ring = adapter->rx_ring;
		rx_ring->count = RxDescriptors;
		pch_gbe_validate_option(&rx_ring->count, &opt, adapter);