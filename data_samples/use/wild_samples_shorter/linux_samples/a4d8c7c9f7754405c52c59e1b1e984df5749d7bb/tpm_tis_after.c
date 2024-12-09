{
	u32 vendor, intfcaps, intmask;
	int rc, i, irq_s, irq_e, probe;
	int irq_r = -1;
	struct tpm_chip *chip;
	struct priv_data *priv;

	priv = devm_kzalloc(dev, sizeof(struct priv_data), GFP_KERNEL);
		irq_s =
		    ioread8(chip->vendor.iobase +
			    TPM_INT_VECTOR(chip->vendor.locality));
		irq_r = irq_s;
		if (irq_s) {
			irq_e = irq_s;
		} else {
			irq_s = 3;
			iowrite32(intmask,
				  chip->vendor.iobase +
				  TPM_INT_ENABLE(chip->vendor.locality));

			devm_free_irq(dev, i, chip);
		}
	}
	if (chip->vendor.irq) {
		iowrite8(chip->vendor.irq,
				  chip->vendor.iobase +
				  TPM_INT_ENABLE(chip->vendor.locality));
		}
	} else if (irq_r != -1)
		iowrite8(irq_r, chip->vendor.iobase +
			 TPM_INT_VECTOR(chip->vendor.locality));

	if (chip->flags & TPM_CHIP_FLAG_TPM2) {
		chip->vendor.timeout_a = msecs_to_jiffies(TPM2_TIMEOUT_A);
		chip->vendor.timeout_b = msecs_to_jiffies(TPM2_TIMEOUT_B);