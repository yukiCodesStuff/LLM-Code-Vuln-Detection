{
	struct mv88e6xxx_chip *chip = dev_id;
	struct mv88e6xxx_atu_entry entry;
	int spid;
	int err;
	u16 val;

	mutex_lock(&chip->reg_lock);

	err = mv88e6xxx_g1_atu_op(chip, 0,
				  MV88E6XXX_G1_ATU_OP_GET_CLR_VIOLATION);
	if (err)
		goto out;

	err = mv88e6xxx_g1_read(chip, MV88E6XXX_G1_ATU_OP, &val);
	if (err)
		goto out;

	err = mv88e6xxx_g1_atu_data_read(chip, &entry);
	if (err)
		goto out;

	err = mv88e6xxx_g1_atu_mac_read(chip, &entry);
	if (err)
		goto out;

	spid = entry.state;

	if (val & MV88E6XXX_G1_ATU_OP_AGE_OUT_VIOLATION) {
		dev_err_ratelimited(chip->dev,
				    "ATU age out violation for %pM\n",
				    entry.mac);
	}
{
	struct mv88e6xxx_chip *chip = dev_id;
	struct mv88e6xxx_atu_entry entry;
	int spid;
	int err;
	u16 val;

	mutex_lock(&chip->reg_lock);

	err = mv88e6xxx_g1_atu_op(chip, 0,
				  MV88E6XXX_G1_ATU_OP_GET_CLR_VIOLATION);
	if (err)
		goto out;

	err = mv88e6xxx_g1_read(chip, MV88E6XXX_G1_ATU_OP, &val);
	if (err)
		goto out;

	err = mv88e6xxx_g1_atu_data_read(chip, &entry);
	if (err)
		goto out;

	err = mv88e6xxx_g1_atu_mac_read(chip, &entry);
	if (err)
		goto out;

	spid = entry.state;

	if (val & MV88E6XXX_G1_ATU_OP_AGE_OUT_VIOLATION) {
		dev_err_ratelimited(chip->dev,
				    "ATU age out violation for %pM\n",
				    entry.mac);
	}
	if (val & MV88E6XXX_G1_ATU_OP_MEMBER_VIOLATION) {
		dev_err_ratelimited(chip->dev,
				    "ATU member violation for %pM portvec %x spid %d\n",
				    entry.mac, entry.portvec, spid);
		chip->ports[spid].atu_member_violation++;
	}