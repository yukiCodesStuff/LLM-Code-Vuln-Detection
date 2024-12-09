			goto next_msg;
		}

		if (!capable(CAP_SYS_ADMIN)) {
			err = -EPERM;
			goto next_msg;
		}
