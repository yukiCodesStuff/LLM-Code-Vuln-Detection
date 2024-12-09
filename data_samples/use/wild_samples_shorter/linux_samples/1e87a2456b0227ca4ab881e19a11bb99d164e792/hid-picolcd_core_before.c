		buf += 10;
		cnt -= 10;
	}
	if (!report)
		return -EINVAL;

	while (cnt > 0 && (buf[cnt-1] == '\n' || buf[cnt-1] == '\r'))
		cnt--;