	kfree(rsi_dev->tx_buffer);

fail_eps:
	kfree(rsi_dev);

	return status;
}
