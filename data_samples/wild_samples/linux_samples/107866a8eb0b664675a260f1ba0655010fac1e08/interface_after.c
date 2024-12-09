	if (queue->task) {
		kthread_stop(queue->task);
		put_task_struct(queue->task);
		queue->task = NULL;
	}
{
	struct xenbus_device *dev = xenvif_to_xenbus_device(queue->vif);
	struct task_struct *task;
	int err;

	BUG_ON(queue->tx_irq);
	BUG_ON(queue->task);
	BUG_ON(queue->dealloc_task);

	err = xenvif_map_frontend_data_rings(queue, tx_ring_ref,
					     rx_ring_ref);
	if (err < 0)
		goto err;

	init_waitqueue_head(&queue->wq);
	init_waitqueue_head(&queue->dealloc_wq);
	atomic_set(&queue->inflight_packets, 0);

	netif_napi_add(queue->vif->dev, &queue->napi, xenvif_poll,
			XENVIF_NAPI_WEIGHT);

	queue->stalled = true;

	task = kthread_run(xenvif_kthread_guest_rx, queue,
			   "%s-guest-rx", queue->name);
	if (IS_ERR(task))
		goto kthread_err;
	queue->task = task;
	/*
	 * Take a reference to the task in order to prevent it from being freed
	 * if the thread function returns before kthread_stop is called.
	 */
	get_task_struct(task);

	task = kthread_run(xenvif_dealloc_kthread, queue,
			   "%s-dealloc", queue->name);
	if (IS_ERR(task))
		goto kthread_err;
	queue->dealloc_task = task;

	if (tx_evtchn == rx_evtchn) {
		/* feature-split-event-channels == 0 */
		err = bind_interdomain_evtchn_to_irqhandler_lateeoi(
			dev, tx_evtchn, xenvif_interrupt, 0,
			queue->name, queue);
		if (err < 0)
			goto err;
		queue->tx_irq = queue->rx_irq = err;
		disable_irq(queue->tx_irq);
	} else {
		/* feature-split-event-channels == 1 */
		snprintf(queue->tx_irq_name, sizeof(queue->tx_irq_name),
			 "%s-tx", queue->name);
		err = bind_interdomain_evtchn_to_irqhandler_lateeoi(
			dev, tx_evtchn, xenvif_tx_interrupt, 0,
			queue->tx_irq_name, queue);
		if (err < 0)
			goto err;
		queue->tx_irq = err;
		disable_irq(queue->tx_irq);

		snprintf(queue->rx_irq_name, sizeof(queue->rx_irq_name),
			 "%s-rx", queue->name);
		err = bind_interdomain_evtchn_to_irqhandler_lateeoi(
			dev, rx_evtchn, xenvif_rx_interrupt, 0,
			queue->rx_irq_name, queue);
		if (err < 0)
			goto err;
		queue->rx_irq = err;
		disable_irq(queue->rx_irq);
	}

	return 0;

kthread_err:
	pr_warn("Could not allocate kthread for %s\n", queue->name);
	err = PTR_ERR(task);
err:
	xenvif_disconnect_queue(queue);
	return err;
}