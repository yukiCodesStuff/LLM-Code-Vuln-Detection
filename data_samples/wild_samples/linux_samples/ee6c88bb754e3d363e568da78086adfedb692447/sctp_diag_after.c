{
	struct sctp_sockaddr_entry *laddr;
	int addrlen = sizeof(struct sockaddr_storage);
	int addrcnt = 0;
	struct nlattr *attr;
	void *info = NULL;

	list_for_each_entry_rcu(laddr, address_list, list)
		addrcnt++;

	attr = nla_reserve(skb, INET_DIAG_LOCALS, addrlen * addrcnt);
	if (!attr)
		return -EMSGSIZE;

	info = nla_data(attr);
	list_for_each_entry_rcu(laddr, address_list, list) {
		memcpy(info, &laddr->a, sizeof(laddr->a));
		memset(info + sizeof(laddr->a), 0, addrlen - sizeof(laddr->a));
		info += addrlen;
	}
{
	int addrlen = sizeof(struct sockaddr_storage);
	struct sctp_transport *from;
	struct nlattr *attr;
	void *info = NULL;

	attr = nla_reserve(skb, INET_DIAG_PEERS,
			   addrlen * asoc->peer.transport_count);
	if (!attr)
		return -EMSGSIZE;

	info = nla_data(attr);
	list_for_each_entry(from, &asoc->peer.transport_addr_list,
			    transports) {
		memcpy(info, &from->ipaddr, sizeof(from->ipaddr));
		memset(info + sizeof(from->ipaddr), 0,
		       addrlen - sizeof(from->ipaddr));
		info += addrlen;
	}