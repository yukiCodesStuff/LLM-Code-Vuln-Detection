	} else {
		/* Init failed, cleanup */
		flush_work(&hdev->tx_work);

		/* Since hci_rx_work() is possible to awake new cmd_work
		 * it should be flushed first to avoid unexpected call of
		 * hci_cmd_work()
		 */
		flush_work(&hdev->rx_work);
		flush_work(&hdev->cmd_work);

		skb_queue_purge(&hdev->cmd_q);
		skb_queue_purge(&hdev->rx_q);
