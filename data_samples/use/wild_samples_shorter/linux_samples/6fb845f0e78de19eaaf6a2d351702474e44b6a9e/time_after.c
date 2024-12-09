		container_of(evt, struct ccount_timer, evt);

	if (timer->irq_enabled) {
		disable_irq_nosync(evt->irq);
		timer->irq_enabled = 0;
	}
	return 0;
}