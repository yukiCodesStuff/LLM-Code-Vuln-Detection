
#include "netlink.h"
#include "core.h"
#include <net/genetlink.h>

#define MAX_MEDIA	3

#define TIPC_MEDIA_TYPE_IB	2
#define TIPC_MEDIA_TYPE_UDP	3

/**
 * struct tipc_media_addr - destination address used by TIPC bearers
 * @value: address info (format defined by media)
 * @media_id: TIPC media type identifier
void tipc_bearer_bc_xmit(struct net *net, u32 bearer_id,
			 struct sk_buff_head *xmitq);

#endif	/* _TIPC_BEARER_H */