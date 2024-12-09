	{
		struct ip_vs_timeout_user t;

		__ip_vs_get_timeouts(net, &t);
		if (copy_to_user(user, &t, sizeof(t)) != 0)
			ret = -EFAULT;
	}