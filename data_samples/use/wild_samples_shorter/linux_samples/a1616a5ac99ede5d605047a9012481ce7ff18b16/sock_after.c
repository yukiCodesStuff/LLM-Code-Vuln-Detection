			sockfd_put(csock);
			return err;
		}
		ca.name[sizeof(ca.name)-1] = 0;

		err = hidp_connection_add(&ca, csock, isock);
		if (!err && copy_to_user(argp, &ca, sizeof(ca)))
			err = -EFAULT;