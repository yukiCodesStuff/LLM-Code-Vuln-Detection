#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/if_vlan.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <asm/unaligned.h>
#include <trace/events/napi.h>
	 MAX_UDP_CHUNK)

static void zap_completion_queue(void);
static void netpoll_arp_reply(struct sk_buff *skb, struct netpoll_info *npinfo);

static unsigned int carrier_timeout = 4;
module_param(carrier_timeout, uint, 0644);

	struct napi_struct *napi;
	int budget = 16;

	WARN_ON_ONCE(!irqs_disabled());

	list_for_each_entry(napi, &dev->napi_list, dev_list) {
		local_irq_enable();
		if (napi->poll_owner != smp_processor_id() &&
		    spin_trylock(&napi->poll_lock)) {
			rcu_read_lock_bh();
			budget = poll_one_napi(rcu_dereference_bh(dev->npinfo),
					       napi, budget);
			rcu_read_unlock_bh();
			spin_unlock(&napi->poll_lock);

			if (!budget) {
				local_irq_disable();
				break;
			}
		}
		local_irq_disable();
	}
}

static void service_arp_queue(struct netpoll_info *npi)
		struct sk_buff *skb;

		while ((skb = skb_dequeue(&npi->arp_tx)))
			netpoll_arp_reply(skb, npi);
	}
}

static void netpoll_poll_dev(struct net_device *dev)
{
	const struct net_device_ops *ops;
	struct netpoll_info *ni = rcu_dereference_bh(dev->npinfo);

	if (!dev || !netif_running(dev))
		return;

	poll_napi(dev);

	if (dev->flags & IFF_SLAVE) {
		if (ni) {
			struct net_device *bond_dev = dev->master;
			struct sk_buff *skb;
			struct netpoll_info *bond_ni = rcu_dereference_bh(bond_dev->npinfo);
			while ((skb = skb_dequeue(&ni->arp_tx))) {
				skb->dev = bond_dev;
				skb_queue_tail(&bond_ni->arp_tx, skb);
			}
		}
	}

	service_arp_queue(ni);

	zap_completion_queue();
}

	return 0;
}

/* call with IRQ disabled */
void netpoll_send_skb_on_dev(struct netpoll *np, struct sk_buff *skb,
			     struct net_device *dev)
{
	int status = NETDEV_TX_BUSY;
	unsigned long tries;
	const struct net_device_ops *ops = dev->netdev_ops;
	/* It is up to the caller to keep npinfo alive. */
	struct netpoll_info *npinfo;

	WARN_ON_ONCE(!irqs_disabled());

	npinfo = rcu_dereference_bh(np->dev->npinfo);
	if (!npinfo || !netif_running(dev) || !netif_device_present(dev)) {
		__kfree_skb(skb);
		return;
	}
	/* don't get messages out of order, and no recursion */
	if (skb_queue_len(&npinfo->txq) == 0 && !netpoll_owner_active(dev)) {
		struct netdev_queue *txq;

		txq = netdev_get_tx_queue(dev, skb_get_queue_mapping(skb));

		/* try until next clock tick */
		for (tries = jiffies_to_usecs(1)/USEC_PER_POLL;
		     tries > 0; --tries) {
			if (__netif_tx_trylock(txq)) {
				if (!netif_xmit_stopped(txq)) {
					if (vlan_tx_tag_present(skb) &&
					    !(netif_skb_features(skb) & NETIF_F_HW_VLAN_TX)) {
						skb = __vlan_put_tag(skb, vlan_tx_tag_get(skb));
						if (unlikely(!skb))
							break;
						skb->vlan_tci = 0;
					}

					status = ops->ndo_start_xmit(skb, dev);
					if (status == NETDEV_TX_OK)
						txq_trans_update(txq);
				}
		}

		WARN_ONCE(!irqs_disabled(),
			"netpoll_send_skb_on_dev(): %s enabled interrupts in poll (%pF)\n",
			dev->name, ops->ndo_start_xmit);

	}

	if (status != NETDEV_TX_OK) {
		skb_queue_tail(&npinfo->txq, skb);
}
EXPORT_SYMBOL(netpoll_send_udp);

static void netpoll_arp_reply(struct sk_buff *skb, struct netpoll_info *npinfo)
{
	struct arphdr *arp;
	unsigned char *arp_ptr;
	int size, type = ARPOP_REPLY, ptype = ETH_P_ARP;
	__be32 sip, tip;
	spin_unlock_irqrestore(&npinfo->rx_lock, flags);
}

int __netpoll_rx(struct sk_buff *skb, struct netpoll_info *npinfo)
{
	int proto, len, ulen;
	int hits = 0;
	const struct iphdr *iph;
	struct udphdr *uh;
	struct netpoll *np, *tmp;

	if (list_empty(&npinfo->rx_np))
		goto out;
		return 1;
	}

	if (skb->protocol == cpu_to_be16(ETH_P_8021Q)) {
		skb = vlan_untag(skb);
		if (unlikely(!skb))
			goto out;
	}

	proto = ntohs(eth_hdr(skb)->h_proto);
	if (proto != ETH_P_IP)
		goto out;
	if (skb->pkt_type == PACKET_OTHERHOST)
}
EXPORT_SYMBOL(netpoll_parse_options);

int __netpoll_setup(struct netpoll *np, struct net_device *ndev, gfp_t gfp)
{
	struct netpoll_info *npinfo;
	const struct net_device_ops *ops;
	unsigned long flags;
	}

	if (!ndev->npinfo) {
		npinfo = kmalloc(sizeof(*npinfo), gfp);
		if (!npinfo) {
			err = -ENOMEM;
			goto out;
		}

		ops = np->dev->netdev_ops;
		if (ops->ndo_netpoll_setup) {
			err = ops->ndo_netpoll_setup(ndev, npinfo, gfp);
			if (err)
				goto free_npinfo;
		}
	} else {
	refill_skbs();

	rtnl_lock();
	err = __netpoll_setup(np, ndev, GFP_KERNEL);
	rtnl_unlock();

	if (err)
		goto put;
}
core_initcall(netpoll_init);

static void rcu_cleanup_netpoll_info(struct rcu_head *rcu_head)
{
	struct netpoll_info *npinfo =
			container_of(rcu_head, struct netpoll_info, rcu);

	skb_queue_purge(&npinfo->arp_tx);
	skb_queue_purge(&npinfo->txq);

	/* we can't call cancel_delayed_work_sync here, as we are in softirq */
	cancel_delayed_work(&npinfo->tx_work);

	/* clean after last, unfinished work */
	__skb_queue_purge(&npinfo->txq);
	/* now cancel it again */
	cancel_delayed_work(&npinfo->tx_work);
	kfree(npinfo);
}

void __netpoll_cleanup(struct netpoll *np)
{
	struct netpoll_info *npinfo;
	unsigned long flags;
			ops->ndo_netpoll_cleanup(np->dev);

		RCU_INIT_POINTER(np->dev->npinfo, NULL);
		call_rcu_bh(&npinfo->rcu, rcu_cleanup_netpoll_info);
	}
}
EXPORT_SYMBOL_GPL(__netpoll_cleanup);

static void rcu_cleanup_netpoll(struct rcu_head *rcu_head)
{
	struct netpoll *np = container_of(rcu_head, struct netpoll, rcu);

	__netpoll_cleanup(np);
	kfree(np);
}

void __netpoll_free_rcu(struct netpoll *np)
{
	call_rcu_bh(&np->rcu, rcu_cleanup_netpoll);
}
EXPORT_SYMBOL_GPL(__netpoll_free_rcu);

void netpoll_cleanup(struct netpoll *np)
{
	if (!np->dev)