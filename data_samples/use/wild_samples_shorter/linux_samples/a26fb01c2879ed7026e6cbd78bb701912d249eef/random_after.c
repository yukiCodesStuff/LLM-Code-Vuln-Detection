write_pool(struct entropy_store *r, const char __user *buffer, size_t count)
{
	size_t bytes;
	__u32 t, buf[16];
	const char __user *p = buffer;

	while (count > 0) {
		int b, i = 0;

		bytes = min(count, sizeof(buf));
		if (copy_from_user(&buf, p, bytes))
			return -EFAULT;

		for (b = bytes ; b > 0 ; b -= sizeof(__u32), i++) {
			if (!arch_get_random_int(&t))
				break;
			buf[i] ^= t;
		}

		count -= bytes;
		p += bytes;

		mix_pool_bytes(r, buf, bytes);