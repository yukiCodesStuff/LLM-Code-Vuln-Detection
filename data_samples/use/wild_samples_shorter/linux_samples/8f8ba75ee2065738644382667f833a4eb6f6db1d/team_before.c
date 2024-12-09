}

#ifdef CONFIG_NET_POLL_CONTROLLER
static int team_port_enable_netpoll(struct team *team, struct team_port *port)
{
	struct netpoll *np;
	int err;

	np = kzalloc(sizeof(*np), GFP_KERNEL);
	if (!np)
		return -ENOMEM;

	err = __netpoll_setup(np, port->dev);
	if (err) {
		kfree(np);
		return err;
	}
}

#else
static int team_port_enable_netpoll(struct team *team, struct team_port *port)
{
	return 0;
}
static void team_port_disable_netpoll(struct team_port *port)
	}

	if (team_netpoll_info(team)) {
		err = team_port_enable_netpoll(team, port);
		if (err) {
			netdev_err(dev, "Failed to enable netpoll on device %s\n",
				   portname);
			goto err_enable_netpoll;
}

static int team_netpoll_setup(struct net_device *dev,
			      struct netpoll_info *npifo)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;
	int err = 0;

	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		err = team_port_enable_netpoll(team, port);
		if (err) {
			__team_netpoll_cleanup(team);
			break;
		}