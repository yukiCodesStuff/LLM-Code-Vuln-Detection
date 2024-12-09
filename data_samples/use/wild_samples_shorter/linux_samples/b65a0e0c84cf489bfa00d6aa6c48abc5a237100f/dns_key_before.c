	size_t result_len = 0;
	const char *data = _data, *end, *opt;

	kenter("%%%d,%s,'%s',%zu",
	       key->serial, key->description, data, datalen);

	if (datalen <= 1 || !data || data[datalen - 1] != '\0')
		return -EINVAL;
	datalen--;
		seq_printf(m, ": %u", key->datalen);
}

struct key_type key_type_dns_resolver = {
	.name		= "dns_resolver",
	.instantiate	= dns_resolver_instantiate,
	.match		= dns_resolver_match,
	.revoke		= user_revoke,
	.destroy	= user_destroy,
	.describe	= dns_resolver_describe,
	.read		= user_read,
};

static int __init init_dns_resolver(void)
{