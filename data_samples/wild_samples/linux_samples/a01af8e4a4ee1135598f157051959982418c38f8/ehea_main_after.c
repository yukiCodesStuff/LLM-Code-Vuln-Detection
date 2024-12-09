		if (!skb_arr_rq1[index]) {
			skb_arr_rq1[index] = netdev_alloc_skb(dev,
							      EHEA_L_PKT_SIZE);
			if (!skb_arr_rq1[index]) {
				ehea_info("Unable to allocate enough skb in the array\n");
				pr->rq1_skba.os_skbs = fill_wqes - i;
				break;
			}
{
	struct sk_buff **skb_arr_rq1 = pr->rq1_skba.arr;
	struct net_device *dev = pr->port->netdev;
	int i;

	if (nr_rq1a > pr->rq1_skba.len) {
		ehea_error("NR_RQ1A bigger than skb array len\n");
		return;
	}
				if (unlikely(!skb)) {
					if (netif_msg_rx_err(port))
						ehea_error("LL rq1: skb=NULL");

					skb = netdev_alloc_skb(dev,
							       EHEA_L_PKT_SIZE);
					if (!skb) {
						ehea_info("Not enough memory to allocate skb\n");
						break;
					}