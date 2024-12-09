/*
 * llc_input.c - Minimal input path for LLC
 *
 * Copyright (c) 1997 by Procom Technology, Inc.
 * 		 2001-2003 by Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *
 * This program can be redistributed or modified under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * This program is distributed without any warranty or implied warranty
 * of merchantability or fitness for a particular purpose.
 *
 * See the GNU General Public License for more details.
 */
#include <linux/netdevice.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/net_namespace.h>
#include <net/llc.h>
#include <net/llc_pdu.h>
#include <net/llc_sap.h>

#if 0
#define dprintk(args...) printk(KERN_DEBUG args)
#else
#define dprintk(args...)
#endif

/*
 * Packet handler for the station, registerable because in the minimal
 * LLC core that is taking shape only the very minimal subset of LLC that
 * is needed for things like IPX, Appletalk, etc will stay, with all the
 * rest in the llc1 and llc2 modules.
 */
static void (*llc_station_handler)(struct sk_buff *skb);

/*
 * Packet handlers for LLC_DEST_SAP and LLC_DEST_CONN.
 */
static void (*llc_type_handlers[2])(struct llc_sap *sap,
				    struct sk_buff *skb);

void llc_add_pack(int type, void (*handler)(struct llc_sap *sap,
					    struct sk_buff *skb))
{
	if (type == LLC_DEST_SAP || type == LLC_DEST_CONN)
		llc_type_handlers[type - 1] = handler;
}
{
	if (type == LLC_DEST_SAP || type == LLC_DEST_CONN)
		llc_type_handlers[type - 1] = NULL;
}

void llc_set_station_handler(void (*handler)(struct sk_buff *skb))
{
	llc_station_handler = handler;
}

/**
 *	llc_pdu_type - returns which LLC component must handle for PDU
 *	@skb: input skb
 *
 *	This function returns which LLC component must handle this PDU.
 */
static __inline__ int llc_pdu_type(struct sk_buff *skb)
{
	int type = LLC_DEST_CONN; /* I-PDU or S-PDU type */
	struct llc_pdu_sn *pdu = llc_pdu_sn_hdr(skb);

	if ((pdu->ctrl_1 & LLC_PDU_TYPE_MASK) != LLC_PDU_TYPE_U)
		goto out;
	switch (LLC_U_PDU_CMD(pdu)) {
	case LLC_1_PDU_CMD_XID:
	case LLC_1_PDU_CMD_UI:
	case LLC_1_PDU_CMD_TEST:
		type = LLC_DEST_SAP;
		break;
	case LLC_2_PDU_CMD_SABME:
	case LLC_2_PDU_CMD_DISC:
	case LLC_2_PDU_RSP_UA:
	case LLC_2_PDU_RSP_DM:
	case LLC_2_PDU_RSP_FRMR:
		break;
	default:
		type = LLC_DEST_INVALID;
		break;
	}
{
	struct llc_sap *sap;
	struct llc_pdu_sn *pdu;
	int dest;
	int (*rcv)(struct sk_buff *, struct net_device *,
		   struct packet_type *, struct net_device *);

	if (!net_eq(dev_net(dev), &init_net))
		goto drop;

	/*
	 * When the interface is in promisc. mode, drop all the crap that it
	 * receives, do not try to analyse it.
	 */
	if (unlikely(skb->pkt_type == PACKET_OTHERHOST)) {
		dprintk("%s: PACKET_OTHERHOST\n", __func__);
		goto drop;
	}
	if (unlikely(skb->pkt_type == PACKET_OTHERHOST)) {
		dprintk("%s: PACKET_OTHERHOST\n", __func__);
		goto drop;
	}
	skb = skb_share_check(skb, GFP_ATOMIC);
	if (unlikely(!skb))
		goto out;
	if (unlikely(!llc_fixup_skb(skb)))
		goto drop;
	pdu = llc_pdu_sn_hdr(skb);
	if (unlikely(!pdu->dsap)) /* NULL DSAP, refer to station */
	       goto handle_station;
	sap = llc_sap_find(pdu->dsap);
	if (unlikely(!sap)) {/* unknown SAP */
		dprintk("%s: llc_sap_find(%02X) failed!\n", __func__,
			pdu->dsap);
		goto drop;
	}
	/*
	 * First the upper layer protocols that don't need the full
	 * LLC functionality
	 */
	rcv = rcu_dereference(sap->rcv_func);
	dest = llc_pdu_type(skb);
	if (unlikely(!dest || !llc_type_handlers[dest - 1])) {
		if (rcv)
			rcv(skb, dev, pt, orig_dev);
		else
			kfree_skb(skb);
	} else {
		if (rcv) {
			struct sk_buff *cskb = skb_clone(skb, GFP_ATOMIC);
			if (cskb)
				rcv(cskb, dev, pt, orig_dev);
		}
		llc_type_handlers[dest - 1](sap, skb);
	}
	llc_sap_put(sap);
out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
handle_station:
	if (!llc_station_handler)
		goto drop;
	llc_station_handler(skb);
	goto out;
}
		if (rcv) {
			struct sk_buff *cskb = skb_clone(skb, GFP_ATOMIC);
			if (cskb)
				rcv(cskb, dev, pt, orig_dev);
		}
		llc_type_handlers[dest - 1](sap, skb);
	}
	llc_sap_put(sap);
out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
handle_station:
	if (!llc_station_handler)
		goto drop;
	llc_station_handler(skb);
	goto out;
}

EXPORT_SYMBOL(llc_add_pack);
EXPORT_SYMBOL(llc_remove_pack);
EXPORT_SYMBOL(llc_set_station_handler);
		if (rcv) {
			struct sk_buff *cskb = skb_clone(skb, GFP_ATOMIC);
			if (cskb)
				rcv(cskb, dev, pt, orig_dev);
		}
		llc_type_handlers[dest - 1](sap, skb);
	}
	llc_sap_put(sap);
out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
handle_station:
	if (!llc_station_handler)
		goto drop;
	llc_station_handler(skb);
	goto out;
}

EXPORT_SYMBOL(llc_add_pack);
EXPORT_SYMBOL(llc_remove_pack);
EXPORT_SYMBOL(llc_set_station_handler);