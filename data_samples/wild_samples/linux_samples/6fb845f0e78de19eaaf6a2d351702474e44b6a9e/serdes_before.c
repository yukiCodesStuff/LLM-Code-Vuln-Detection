{
	if (port < 9)
		return 0;

	return mv88e6390_serdes_irq_setup(chip, port);
}

void mv88e6390x_serdes_irq_free(struct mv88e6xxx_chip *chip, int port)
{
	int lane = mv88e6390x_serdes_get_lane(chip, port);

	if (lane == -ENODEV)
		return;

	if (lane < 0)
		return;

	mv88e6390_serdes_irq_disable(chip, port, lane);

	/* Freeing the IRQ will trigger irq callbacks. So we cannot
	 * hold the reg_lock.
	 */
	mutex_unlock(&chip->reg_lock);
	free_irq(chip->ports[port].serdes_irq, &chip->ports[port]);
	mutex_lock(&chip->reg_lock);

	chip->ports[port].serdes_irq = 0;
}

void mv88e6390_serdes_irq_free(struct mv88e6xxx_chip *chip, int port)
{
	if (port < 9)
		return;

	mv88e6390x_serdes_irq_free(chip, port);
}

int mv88e6341_serdes_power(struct mv88e6xxx_chip *chip, int port, bool on)
{
	u8 cmode = chip->ports[port].cmode;

	if (port != 5)
		return 0;

	if (cmode == MV88E6XXX_PORT_STS_CMODE_1000BASE_X ||
	    cmode == MV88E6XXX_PORT_STS_CMODE_SGMII ||
	    cmode == MV88E6XXX_PORT_STS_CMODE_2500BASEX)
		return mv88e6390_serdes_power_sgmii(chip, MV88E6341_ADDR_SERDES,
						    on);

	return 0;
}