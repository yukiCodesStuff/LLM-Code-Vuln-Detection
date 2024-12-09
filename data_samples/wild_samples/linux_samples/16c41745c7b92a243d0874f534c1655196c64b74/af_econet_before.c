	switch (cmd) {
	case SIOCSIFADDR:
		edev = dev->ec_ptr;
		if (edev == NULL) {
			/* Magic up a new one. */
			edev = kzalloc(sizeof(struct ec_device), GFP_KERNEL);
			if (edev == NULL) {
				err = -ENOMEM;
				break;
			}
			dev->ec_ptr = edev;
		} else
			net2dev_map[edev->net] = NULL;
		edev->station = sec->addr.station;
		edev->net = sec->addr.net;
		net2dev_map[sec->addr.net] = dev;
		if (!net2dev_map[0])
			net2dev_map[0] = dev;
		break;

	case SIOCGIFADDR:
		edev = dev->ec_ptr;
		if (edev == NULL) {
			err = -ENODEV;
			break;
		}
		memset(sec, 0, sizeof(struct sockaddr_ec));
		sec->addr.station = edev->station;
		sec->addr.net = edev->net;
		sec->sec_family = AF_ECONET;
		dev_put(dev);
		if (copy_to_user(arg, &ifr, sizeof(struct ifreq)))
			err = -EFAULT;
		break;

	default:
		err = -EINVAL;
		break;
	}

	mutex_unlock(&econet_mutex);

	dev_put(dev);

	return err;
}