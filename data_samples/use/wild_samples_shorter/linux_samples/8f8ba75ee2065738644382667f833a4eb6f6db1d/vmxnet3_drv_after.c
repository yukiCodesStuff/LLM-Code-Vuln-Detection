	netdev->watchdog_timeo = 5 * HZ;

	INIT_WORK(&adapter->work, vmxnet3_reset_work);
	set_bit(VMXNET3_STATE_BIT_QUIESCED, &adapter->state);

	if (adapter->intr.type == VMXNET3_IT_MSIX) {
		int i;
		for (i = 0; i < adapter->num_rx_queues; i++) {
		goto err_register;
	}

	vmxnet3_check_link(adapter, false);
	atomic_inc(&devices_found);
	return 0;
