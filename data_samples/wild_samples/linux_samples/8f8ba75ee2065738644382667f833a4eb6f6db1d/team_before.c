{
	if (team->ops.port_leave)
		team->ops.port_leave(team, port);
	port->dev->priv_flags &= ~IFF_TEAM_PORT;
	dev_put(team->dev);
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
	port->np = np;
	return err;
}
{
	return team->dev->npinfo;
}

#else
static int team_port_enable_netpoll(struct team *team, struct team_port *port)
{
	return 0;
}
	if (err) {
		netdev_err(dev, "Failed to add vlan ids to device %s\n",
				portname);
		goto err_vids_add;
	}

	if (team_netpoll_info(team)) {
		err = team_port_enable_netpoll(team, port);
		if (err) {
			netdev_err(dev, "Failed to enable netpoll on device %s\n",
				   portname);
			goto err_enable_netpoll;
		}
	}
{
	struct team *team = netdev_priv(dev);

	mutex_lock(&team->lock);
	__team_netpoll_cleanup(team);
	mutex_unlock(&team->lock);
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
	}
	mutex_unlock(&team->lock);
	return err;
}