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
					spin_unlock_irqrestore(
							      &target_list_lock,
							      flags);
					__netpoll_cleanup(&nt->np);
					spin_lock_irqsave(&target_list_lock,
							  flags);
					dev_put(nt->np.dev);
					nt->np.dev = NULL;
					netconsole_target_put(nt);
				}