		if (ret) {
			dev_err(&bus->dev, "MDIO bus is busy\n");
			return ret;
		}

		hns_mdio_cmd_write(mdio_dev, is_c45,
				   MDIO_C45_WRITE_ADDR, phy_id, devad);
	}

	/* Step 5: waitting for MDIO_COMMAND_REG 's mdio_start==0,*/
	/* check for read or write opt is finished */
	ret = hns_mdio_wait_ready(bus);
	if (ret) {
		dev_err(&bus->dev, "MDIO bus is busy\n");
		return ret;
	}

	reg_val = MDIO_GET_REG_BIT(mdio_dev, MDIO_STA_REG, MDIO_STATE_STA_B);
	if (reg_val) {
		dev_err(&bus->dev, " ERROR! MDIO Read failed!\n");
		return -EBUSY;
	}

	/* Step 6; get out data*/
	reg_val = (u16)MDIO_GET_REG_FIELD(mdio_dev, MDIO_RDATA_REG,
					  MDIO_RDATA_DATA_M, MDIO_RDATA_DATA_S);

	return reg_val;
}

/**
 * hns_mdio_reset - reset mdio bus
 * @bus: mdio bus
 *
 * Return 0 on success, negative on failure
 */
static int hns_mdio_reset(struct mii_bus *bus)
{
	struct hns_mdio_device *mdio_dev = (struct hns_mdio_device *)bus->priv;
	const struct hns_mdio_sc_reg *sc_reg;
	int ret;

	if (dev_of_node(bus->parent)) {
		if (!mdio_dev->subctrl_vbase) {
			dev_err(&bus->dev, "mdio sys ctl reg has not maped\n");
			return -ENODEV;
		}