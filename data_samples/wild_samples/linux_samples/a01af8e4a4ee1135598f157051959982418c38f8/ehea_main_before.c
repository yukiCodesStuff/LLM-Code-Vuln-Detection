		if (!skb_arr_rq1[index]) {
			skb_arr_rq1[index] = netdev_alloc_skb(dev,
							      EHEA_L_PKT_SIZE);
			if (!skb_arr_rq1[index]) {
				pr->rq1_skba.os_skbs = fill_wqes - i;
				break;
			}
{
	struct sk_buff **skb_arr_rq1 = pr->rq1_skba.arr;
	struct net_device *dev = pr->port->netdev;
	int i;

	for (i = 0; i < pr->rq1_skba.len; i++) {
		skb_arr_rq1[i] = netdev_alloc_skb(dev, EHEA_L_PKT_SIZE);
		if (!skb_arr_rq1[i])
			break;
	}
				if (unlikely(!skb)) {
					if (netif_msg_rx_err(port))
						ehea_error("LL rq1: skb=NULL");

					skb = netdev_alloc_skb(dev,
							       EHEA_L_PKT_SIZE);
					if (!skb)
						break;
				}
				skb_copy_to_linear_data(skb, ((char *)cqe) + 64,
						 cqe->num_bytes_transfered - 4);
				ehea_fill_skb(dev, skb, cqe);
			} else if (rq == 2) {
				/* RQ2 */
				skb = get_skb_by_index(skb_arr_rq2,
						       skb_arr_rq2_len, cqe);
				if (unlikely(!skb)) {
					if (netif_msg_rx_err(port))
						ehea_error("rq2: skb=NULL");
					break;
				}