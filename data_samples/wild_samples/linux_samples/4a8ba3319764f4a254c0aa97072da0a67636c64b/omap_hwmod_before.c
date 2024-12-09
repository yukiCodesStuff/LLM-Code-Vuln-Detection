{
	struct omap_hwmod_ocp_if *os;
	struct list_head *p;
	int i, j, mpu_irqs_cnt, sdma_reqs_cnt, addr_cnt;
	int r = 0;

	/* For each IRQ, DMA, memory area, fill in array.*/

	mpu_irqs_cnt = _count_mpu_irqs(oh);
	for (i = 0; i < mpu_irqs_cnt; i++) {
		(res + r)->name = (oh->mpu_irqs + i)->name;
		(res + r)->start = (oh->mpu_irqs + i)->irq;
		(res + r)->end = (oh->mpu_irqs + i)->irq;
		(res + r)->flags = IORESOURCE_IRQ;
		r++;
	}