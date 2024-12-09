	if (port < 9)
		return 0;

	return mv88e6390x_serdes_irq_setup(chip, port);
}

void mv88e6390x_serdes_irq_free(struct mv88e6xxx_chip *chip, int port)
{