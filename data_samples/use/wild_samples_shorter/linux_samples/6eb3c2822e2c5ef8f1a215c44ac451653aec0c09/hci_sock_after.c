	bt_cb(skb)->pkt_type = *((unsigned char *) skb->data);
	skb_pull(skb, 1);

	if (hci_pi(sk)->channel == HCI_CHANNEL_USER) {
		/* No permission check is needed for user channel
		 * since that gets enforced when binding the socket.
		 *
		 * However check that the packet type is valid.
		 */
		if (bt_cb(skb)->pkt_type != HCI_COMMAND_PKT &&
		    bt_cb(skb)->pkt_type != HCI_ACLDATA_PKT &&
		    bt_cb(skb)->pkt_type != HCI_SCODATA_PKT) {
			err = -EINVAL;
			goto drop;
		}

		skb_queue_tail(&hdev->raw_q, skb);
		queue_work(hdev->workqueue, &hdev->tx_work);
	} else if (bt_cb(skb)->pkt_type == HCI_COMMAND_PKT) {
		u16 opcode = get_unaligned_le16(skb->data);
		u16 ogf = hci_opcode_ogf(opcode);
		u16 ocf = hci_opcode_ocf(opcode);

			goto drop;
		}

		skb_queue_tail(&hdev->raw_q, skb);
		queue_work(hdev->workqueue, &hdev->tx_work);
	}
