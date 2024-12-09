
	mpu_irqs_cnt = _count_mpu_irqs(oh);
	for (i = 0; i < mpu_irqs_cnt; i++) {
		unsigned int irq;

		if (oh->xlate_irq)
			irq = oh->xlate_irq((oh->mpu_irqs + i)->irq);
		else
			irq = (oh->mpu_irqs + i)->irq;
		(res + r)->name = (oh->mpu_irqs + i)->name;
		(res + r)->start = irq;
		(res + r)->end = irq;
		(res + r)->flags = IORESOURCE_IRQ;
		r++;
	}
