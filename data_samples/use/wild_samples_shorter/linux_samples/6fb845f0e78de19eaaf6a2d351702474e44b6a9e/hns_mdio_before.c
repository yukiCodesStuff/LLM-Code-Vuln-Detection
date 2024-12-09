		}

		hns_mdio_cmd_write(mdio_dev, is_c45,
				   MDIO_C45_WRITE_ADDR, phy_id, devad);
	}

	/* Step 5: waitting for MDIO_COMMAND_REG 's mdio_start==0,*/
	/* check for read or write opt is finished */