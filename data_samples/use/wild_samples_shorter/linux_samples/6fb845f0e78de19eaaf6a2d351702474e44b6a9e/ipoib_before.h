	struct list_head     list;
	struct net_device   *dev;
	struct ipoib_neigh  *neigh;
	struct ipoib_path   *path;
	struct ipoib_tx_buf *tx_ring;
	unsigned int	     tx_head;
	unsigned int	     tx_tail;
	unsigned long	     flags;