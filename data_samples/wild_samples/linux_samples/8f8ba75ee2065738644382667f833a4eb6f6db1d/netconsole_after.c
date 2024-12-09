			switch (event) {
			case NETDEV_CHANGENAME:
				strlcpy(nt->np.dev_name, dev->name, IFNAMSIZ);
				break;
			case NETDEV_RELEASE:
			case NETDEV_JOIN:
			case NETDEV_UNREGISTER:
				/*
				 * rtnl_lock already held
				 */
				if (nt->np.dev) {
					__netpoll_cleanup(&nt->np);
					dev_put(nt->np.dev);
					nt->np.dev = NULL;
				}