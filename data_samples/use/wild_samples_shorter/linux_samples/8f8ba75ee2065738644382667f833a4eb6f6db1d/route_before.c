		}
		dev_out = net->loopback_dev;
		fl4->flowi4_oif = dev_out->ifindex;
		res.fi = NULL;
		flags |= RTCF_LOCAL;
		goto make_route;
	}
