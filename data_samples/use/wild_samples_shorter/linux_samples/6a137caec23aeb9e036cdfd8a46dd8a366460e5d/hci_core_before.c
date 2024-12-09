	} else {
		/* Init failed, cleanup */
		flush_work(&hdev->tx_work);
		flush_work(&hdev->cmd_work);
		flush_work(&hdev->rx_work);

		skb_queue_purge(&hdev->cmd_q);
		skb_queue_purge(&hdev->rx_q);
