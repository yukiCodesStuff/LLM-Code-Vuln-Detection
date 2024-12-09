#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <asm/unaligned.h>
#include <trace/events/napi.h>
	 MAX_UDP_CHUNK)

static void zap_completion_queue(void);
static void arp_reply(struct sk_buff *skb);

static unsigned int carrier_timeout = 4;
module_param(carrier_timeout, uint, 0644);

	struct napi_struct *napi;
	int budget = 16;

	list_for_each_entry(napi, &dev->napi_list, dev_list) {
		if (napi->poll_owner != smp_processor_id() &&
		    spin_trylock(&napi->poll_lock)) {
			budget = poll_one_napi(dev->npinfo, napi, budget);
			spin_unlock(&napi->poll_lock);

			if (!budget)
				break;
		}
	}
}

static void service_arp_queue(struct netpoll_info *npi)
		struct sk_buff *skb;

		while ((skb = skb_dequeue(&npi->arp_tx)))
			arp_reply(skb);
	}
}

static void netpoll_poll_dev(struct net_device *dev)
{
	const struct net_device_ops *ops;

	if (!dev || !netif_running(dev))
		return;

	poll_napi(dev);

	if (dev->flags & IFF_SLAVE) {
		if (dev->npinfo) {
			struct net_device *bond_dev = dev->master;
			struct sk_buff *skb;
			while ((skb = skb_dequeue(&dev->npinfo->arp_tx))) {
				skb->dev = bond_dev;
				skb_queue_tail(&bond_dev->npinfo->arp_tx, skb);
			}
		}
	}

	service_arp_queue(dev->npinfo);

	zap_completion_queue();
}

	return 0;
}

void netpoll_send_skb_on_dev(struct netpoll *np, struct sk_buff *skb,
			     struct net_device *dev)
{
	int status = NETDEV_TX_BUSY;
	unsigned long tries;
	const struct net_device_ops *ops = dev->netdev_ops;
	/* It is up to the caller to keep npinfo alive. */
	struct netpoll_info *npinfo = np->dev->npinfo;

	if (!npinfo || !netif_running(dev) || !netif_device_present(dev)) {
		__kfree_skb(skb);
		return;
	}
	/* don't get messages out of order, and no recursion */
	if (skb_queue_len(&npinfo->txq) == 0 && !netpoll_owner_active(dev)) {
		struct netdev_queue *txq;
		unsigned long flags;

		txq = netdev_get_tx_queue(dev, skb_get_queue_mapping(skb));

		local_irq_save(flags);
		/* try until next clock tick */
		for (tries = jiffies_to_usecs(1)/USEC_PER_POLL;
		     tries > 0; --tries) {
			if (__netif_tx_trylock(txq)) {
				if (!netif_xmit_stopped(txq)) {
					status = ops->ndo_start_xmit(skb, dev);
					if (status == NETDEV_TX_OK)
						txq_trans_update(txq);
				}
		}

		WARN_ONCE(!irqs_disabled(),
			"netpoll_send_skb(): %s enabled interrupts in poll (%pF)\n",
			dev->name, ops->ndo_start_xmit);

		local_irq_restore(flags);
	}

	if (status != NETDEV_TX_OK) {
		skb_queue_tail(&npinfo->txq, skb);
}
EXPORT_SYMBOL(netpoll_send_udp);

static void arp_reply(struct sk_buff *skb)
{
	struct netpoll_info *npinfo = skb->dev->npinfo;
	struct arphdr *arp;
	unsigned char *arp_ptr;
	int size, type = ARPOP_REPLY, ptype = ETH_P_ARP;
	__be32 sip, tip;
	spin_unlock_irqrestore(&npinfo->rx_lock, flags);
}

int __netpoll_rx(struct sk_buff *skb)
{
	int proto, len, ulen;
	int hits = 0;
	const struct iphdr *iph;
	struct udphdr *uh;
	struct netpoll_info *npinfo = skb->dev->npinfo;
	struct netpoll *np, *tmp;

	if (list_empty(&npinfo->rx_np))
		goto out;
		return 1;
	}

	proto = ntohs(eth_hdr(skb)->h_proto);
	if (proto != ETH_P_IP)
		goto out;
	if (skb->pkt_type == PACKET_OTHERHOST)
}
EXPORT_SYMBOL(netpoll_parse_options);

int __netpoll_setup(struct netpoll *np, struct net_device *ndev)
{
	struct netpoll_info *npinfo;
	const struct net_device_ops *ops;
	unsigned long flags;
	}

	if (!ndev->npinfo) {
		npinfo = kmalloc(sizeof(*npinfo), GFP_KERNEL);
		if (!npinfo) {
			err = -ENOMEM;
			goto out;
		}

		ops = np->dev->netdev_ops;
		if (ops->ndo_netpoll_setup) {
			err = ops->ndo_netpoll_setup(ndev, npinfo);
			if (err)
				goto free_npinfo;
		}
	} else {
	refill_skbs();

	rtnl_lock();
	err = __netpoll_setup(np, ndev);
	rtnl_unlock();

	if (err)
		goto put;
}
core_initcall(netpoll_init);

void __netpoll_cleanup(struct netpoll *np)
{
	struct netpoll_info *npinfo;
	unsigned long flags;
			ops->ndo_netpoll_cleanup(np->dev);

		RCU_INIT_POINTER(np->dev->npinfo, NULL);

		/* avoid racing with NAPI reading npinfo */
		synchronize_rcu_bh();

		skb_queue_purge(&npinfo->arp_tx);
		skb_queue_purge(&npinfo->txq);
		cancel_delayed_work_sync(&npinfo->tx_work);

		/* clean after last, unfinished work */
		__skb_queue_purge(&npinfo->txq);
		kfree(npinfo);
	}
}
EXPORT_SYMBOL_GPL(__netpoll_cleanup);

void netpoll_cleanup(struct netpoll *np)
{
	if (!np->dev)