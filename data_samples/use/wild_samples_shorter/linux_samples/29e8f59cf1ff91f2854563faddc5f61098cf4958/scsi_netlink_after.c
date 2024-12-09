			goto next_msg;
		}

		if (!netlink_capable(skb, CAP_SYS_ADMIN)) {
			err = -EPERM;
			goto next_msg;
		}
