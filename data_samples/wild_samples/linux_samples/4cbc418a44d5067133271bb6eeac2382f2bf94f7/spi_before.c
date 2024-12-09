{
	struct spi_controller *ctlr =
		container_of(work, struct spi_controller, pump_messages);

	__spi_pump_messages(ctlr, true);
}

/**
 * spi_take_timestamp_pre - helper for drivers to collect the beginning of the
 *			    TX timestamp for the requested byte from the SPI
 *			    transfer. The frequency with which this function
 *			    must be called (once per word, once for the whole
 *			    transfer, once per batch of words etc) is arbitrary
 *			    as long as the @tx buffer offset is greater than or
 *			    equal to the requested byte at the time of the
 *			    call. The timestamp is only taken once, at the
 *			    first such call. It is assumed that the driver
 *			    advances its @tx buffer pointer monotonically.
 * @ctlr: Pointer to the spi_controller structure of the driver
 * @xfer: Pointer to the transfer being timestamped
 * @tx: Pointer to the current word within the xfer->tx_buf that the driver is
 *	preparing to transmit right now.
 * @irqs_off: If true, will disable IRQs and preemption for the duration of the
 *	      transfer, for less jitter in time measurement. Only compatible
 *	      with PIO drivers. If true, must follow up with
 *	      spi_take_timestamp_post or otherwise system will crash.
 *	      WARNING: for fully predictable results, the CPU frequency must
 *	      also be under control (governor).
 */
void spi_take_timestamp_pre(struct spi_controller *ctlr,
			    struct spi_transfer *xfer,
			    const void *tx, bool irqs_off)
{
	u8 bytes_per_word = DIV_ROUND_UP(xfer->bits_per_word, 8);

	if (!xfer->ptp_sts)
		return;

	if (xfer->timestamped_pre)
		return;

	if (tx < (xfer->tx_buf + xfer->ptp_sts_word_pre * bytes_per_word))
		return;

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_pre = (tx - xfer->tx_buf) / bytes_per_word;

	xfer->timestamped_pre = true;

	if (irqs_off) {
		local_irq_save(ctlr->irq_flags);
		preempt_disable();
	}

	ptp_read_system_prets(xfer->ptp_sts);
}
{
	struct spi_controller *ctlr =
		container_of(work, struct spi_controller, pump_messages);

	__spi_pump_messages(ctlr, true);
}

/**
 * spi_take_timestamp_pre - helper for drivers to collect the beginning of the
 *			    TX timestamp for the requested byte from the SPI
 *			    transfer. The frequency with which this function
 *			    must be called (once per word, once for the whole
 *			    transfer, once per batch of words etc) is arbitrary
 *			    as long as the @tx buffer offset is greater than or
 *			    equal to the requested byte at the time of the
 *			    call. The timestamp is only taken once, at the
 *			    first such call. It is assumed that the driver
 *			    advances its @tx buffer pointer monotonically.
 * @ctlr: Pointer to the spi_controller structure of the driver
 * @xfer: Pointer to the transfer being timestamped
 * @tx: Pointer to the current word within the xfer->tx_buf that the driver is
 *	preparing to transmit right now.
 * @irqs_off: If true, will disable IRQs and preemption for the duration of the
 *	      transfer, for less jitter in time measurement. Only compatible
 *	      with PIO drivers. If true, must follow up with
 *	      spi_take_timestamp_post or otherwise system will crash.
 *	      WARNING: for fully predictable results, the CPU frequency must
 *	      also be under control (governor).
 */
void spi_take_timestamp_pre(struct spi_controller *ctlr,
			    struct spi_transfer *xfer,
			    const void *tx, bool irqs_off)
{
	u8 bytes_per_word = DIV_ROUND_UP(xfer->bits_per_word, 8);

	if (!xfer->ptp_sts)
		return;

	if (xfer->timestamped_pre)
		return;

	if (tx < (xfer->tx_buf + xfer->ptp_sts_word_pre * bytes_per_word))
		return;

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_pre = (tx - xfer->tx_buf) / bytes_per_word;

	xfer->timestamped_pre = true;

	if (irqs_off) {
		local_irq_save(ctlr->irq_flags);
		preempt_disable();
	}

	ptp_read_system_prets(xfer->ptp_sts);
}
	if (irqs_off) {
		local_irq_save(ctlr->irq_flags);
		preempt_disable();
	}

	ptp_read_system_prets(xfer->ptp_sts);
}
EXPORT_SYMBOL_GPL(spi_take_timestamp_pre);

/**
 * spi_take_timestamp_post - helper for drivers to collect the end of the
 *			     TX timestamp for the requested byte from the SPI
 *			     transfer. Can be called with an arbitrary
 *			     frequency: only the first call where @tx exceeds
 *			     or is equal to the requested word will be
 *			     timestamped.
 * @ctlr: Pointer to the spi_controller structure of the driver
 * @xfer: Pointer to the transfer being timestamped
 * @tx: Pointer to the current word within the xfer->tx_buf that the driver has
 *	just transmitted.
 * @irqs_off: If true, will re-enable IRQs and preemption for the local CPU.
 */
void spi_take_timestamp_post(struct spi_controller *ctlr,
			     struct spi_transfer *xfer,
			     const void *tx, bool irqs_off)
{
	u8 bytes_per_word = DIV_ROUND_UP(xfer->bits_per_word, 8);

	if (!xfer->ptp_sts)
		return;

	if (xfer->timestamped_post)
		return;

	if (tx < (xfer->tx_buf + xfer->ptp_sts_word_post * bytes_per_word))
		return;

	ptp_read_system_postts(xfer->ptp_sts);

	if (irqs_off) {
		local_irq_restore(ctlr->irq_flags);
		preempt_enable();
	}

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_post = (tx - xfer->tx_buf) / bytes_per_word;

	xfer->timestamped_post = true;
}
	if (irqs_off) {
		local_irq_restore(ctlr->irq_flags);
		preempt_enable();
	}

	/* Capture the resolution of the timestamp */
	xfer->ptp_sts_word_post = (tx - xfer->tx_buf) / bytes_per_word;

	xfer->timestamped_post = true;
}
EXPORT_SYMBOL_GPL(spi_take_timestamp_post);

/**
 * spi_set_thread_rt - set the controller to pump at realtime priority
 * @ctlr: controller to boost priority of
 *
 * This can be called because the controller requested realtime priority
 * (by setting the ->rt value before calling spi_register_controller()) or
 * because a device on the bus said that its transfers needed realtime
 * priority.
 *
 * NOTE: at the moment if any device on a bus says it needs realtime then
 * the thread will be at realtime priority for all transfers on that
 * controller.  If this eventually becomes a problem we may see if we can
 * find a way to boost the priority only temporarily during relevant
 * transfers.
 */
static void spi_set_thread_rt(struct spi_controller *ctlr)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO / 2 };

	dev_info(&ctlr->dev,
		"will run message pump with realtime priority\n");
	sched_setscheduler(ctlr->kworker_task, SCHED_FIFO, &param);
}

static int spi_init_queue(struct spi_controller *ctlr)
{
	ctlr->running = false;
	ctlr->busy = false;

	kthread_init_worker(&ctlr->kworker);
	ctlr->kworker_task = kthread_run(kthread_worker_fn, &ctlr->kworker,
					 "%s", dev_name(&ctlr->dev));
	if (IS_ERR(ctlr->kworker_task)) {
		dev_err(&ctlr->dev, "failed to create message pump task\n");
		return PTR_ERR(ctlr->kworker_task);
	}