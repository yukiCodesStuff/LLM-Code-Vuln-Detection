
#include "netlink.h"
#include "core.h"
#include "msg.h"
#include <net/genetlink.h>

#define MAX_MEDIA	3

#define TIPC_MEDIA_TYPE_IB	2
#define TIPC_MEDIA_TYPE_UDP	3

/* minimum bearer MTU */
#define TIPC_MIN_BEARER_MTU	(MAX_H_SIZE + INT_H_SIZE)

/**
 * struct tipc_media_addr - destination address used by TIPC bearers
 * @value: address info (format defined by media)
 * @media_id: TIPC media type identifier
void tipc_bearer_bc_xmit(struct net *net, u32 bearer_id,
			 struct sk_buff_head *xmitq);

/* check if device MTU is too low for tipc headers */
static inline bool tipc_mtu_bad(struct net_device *dev, unsigned int reserve)
{
	if (dev->mtu >= TIPC_MIN_BEARER_MTU + reserve)
		return false;
	netdev_warn(dev, "MTU too low for tipc bearer\n");
	return true;
}

#endif	/* _TIPC_BEARER_H */