	.irqs			= omap4_prcm_irqs,
	.nr_irqs		= ARRAY_SIZE(omap4_prcm_irqs),
	.irq			= 11 + OMAP44XX_IRQ_GIC_START,
	.read_pending_irqs	= &omap44xx_prm_read_pending_irqs,
	.ocp_barrier		= &omap44xx_prm_ocp_barrier,
	.save_and_clear_irqen	= &omap44xx_prm_save_and_clear_irqen,
	.restore_irqen		= &omap44xx_prm_restore_irqen,
		}

		/* Once OMAP4 DT is filled as well */
		if (irq_num >= 0)
			omap4_prcm_irq_setup.irq = irq_num;
	}

	omap44xx_prm_enable_io_wakeup();
