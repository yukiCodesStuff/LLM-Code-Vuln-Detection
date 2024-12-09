{
	struct mv88e6xxx_chip *chip = dev_id;
	struct mv88e6xxx_atu_entry entry;
	int err;
	u16 val;

	mutex_lock(&chip->reg_lock);
	if (err)
		goto out;

	if (val & MV88E6XXX_G1_ATU_OP_AGE_OUT_VIOLATION) {
		dev_err_ratelimited(chip->dev,
				    "ATU age out violation for %pM\n",
				    entry.mac);

	if (val & MV88E6XXX_G1_ATU_OP_MEMBER_VIOLATION) {
		dev_err_ratelimited(chip->dev,
				    "ATU member violation for %pM portvec %x\n",
				    entry.mac, entry.portvec);
		chip->ports[entry.portvec].atu_member_violation++;
	}

	if (val & MV88E6XXX_G1_ATU_OP_MISS_VIOLATION) {
		dev_err_ratelimited(chip->dev,
				    "ATU miss violation for %pM portvec %x\n",
				    entry.mac, entry.portvec);
		chip->ports[entry.portvec].atu_miss_violation++;
	}

	if (val & MV88E6XXX_G1_ATU_OP_FULL_VIOLATION) {
		dev_err_ratelimited(chip->dev,
				    "ATU full violation for %pM portvec %x\n",
				    entry.mac, entry.portvec);
		chip->ports[entry.portvec].atu_full_violation++;
	}
	mutex_unlock(&chip->reg_lock);

	return IRQ_HANDLED;