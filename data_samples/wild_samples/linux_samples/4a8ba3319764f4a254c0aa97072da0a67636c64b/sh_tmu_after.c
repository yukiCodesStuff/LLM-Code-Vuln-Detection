{
	struct clock_event_device *ced = &ch->ced;
	int ret;

	ced->name = name;
	ced->features = CLOCK_EVT_FEAT_PERIODIC;
	ced->features |= CLOCK_EVT_FEAT_ONESHOT;
	ced->rating = 200;
	ced->cpumask = cpu_possible_mask;
	ced->set_next_event = sh_tmu_clock_event_next;
	ced->set_mode = sh_tmu_clock_event_mode;
	ced->suspend = sh_tmu_clock_event_suspend;
	ced->resume = sh_tmu_clock_event_resume;

	dev_info(&ch->tmu->pdev->dev, "ch%u: used for clock events\n",
		 ch->index);

	clockevents_config_and_register(ced, 1, 0x300, 0xffffffff);

	ret = request_irq(ch->irq, sh_tmu_interrupt,
			  IRQF_TIMER | IRQF_IRQPOLL | IRQF_NOBALANCING,
			  dev_name(&ch->tmu->pdev->dev), ch);
	if (ret) {
		dev_err(&ch->tmu->pdev->dev, "ch%u: failed to request irq %d\n",
			ch->index, ch->irq);
		return;
	}