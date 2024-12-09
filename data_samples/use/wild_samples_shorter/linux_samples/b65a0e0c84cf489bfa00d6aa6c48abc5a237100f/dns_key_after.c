	size_t result_len = 0;
	const char *data = _data, *end, *opt;

	kenter("%%%d,%s,'%*.*s',%zu",
	       key->serial, key->description,
	       (int)datalen, (int)datalen, data, datalen);

	if (datalen <= 1 || !data || data[datalen - 1] != '\0')
		return -EINVAL;
	datalen--;
		seq_printf(m, ": %u", key->datalen);
}

/*
 * read the DNS data
 * - the key's semaphore is read-locked
 */
static long dns_resolver_read(const struct key *key,
			      char __user *buffer, size_t buflen)
{
	if (key->type_data.x[0])
		return key->type_data.x[0];

	return user_read(key, buffer, buflen);
}

struct key_type key_type_dns_resolver = {
	.name		= "dns_resolver",
	.instantiate	= dns_resolver_instantiate,
	.match		= dns_resolver_match,
	.revoke		= user_revoke,
	.destroy	= user_destroy,
	.describe	= dns_resolver_describe,
	.read		= dns_resolver_read,
};

static int __init init_dns_resolver(void)
{