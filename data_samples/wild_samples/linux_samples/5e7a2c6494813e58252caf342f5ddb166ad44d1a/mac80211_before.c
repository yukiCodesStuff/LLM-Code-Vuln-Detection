{
	struct sk_buff *skb = phy->rx_amsdu[q].head;
	struct mt76_dev *dev = phy->dev;

	phy->rx_amsdu[q].head = NULL;
	phy->rx_amsdu[q].tail = NULL;
	__skb_queue_tail(&dev->rx_skb[q], skb);
}