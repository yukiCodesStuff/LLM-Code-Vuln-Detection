			skb_arr_rq1[index] = netdev_alloc_skb(dev,
							      EHEA_L_PKT_SIZE);
			if (!skb_arr_rq1[index]) {
				pr->rq1_skba.os_skbs = fill_wqes - i;
				break;
			}
		}
	struct net_device *dev = pr->port->netdev;
	int i;

	for (i = 0; i < pr->rq1_skba.len; i++) {
		skb_arr_rq1[i] = netdev_alloc_skb(dev, EHEA_L_PKT_SIZE);
		if (!skb_arr_rq1[i])
			break;
	}
	/* Ring doorbell */
	ehea_update_rq1a(pr->qp, nr_rq1a);
}

static int ehea_refill_rq_def(struct ehea_port_res *pr,
			      struct ehea_q_skb_arr *q_skba, int rq_nr,

					skb = netdev_alloc_skb(dev,
							       EHEA_L_PKT_SIZE);
					if (!skb)
						break;
				}
				skb_copy_to_linear_data(skb, ((char *)cqe) + 64,
						 cqe->num_bytes_transfered - 4);
				ehea_fill_skb(dev, skb, cqe);