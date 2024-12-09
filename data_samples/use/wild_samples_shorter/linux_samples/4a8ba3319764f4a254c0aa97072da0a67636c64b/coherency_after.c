	return type;
}

/*
 * As a precaution, we currently completely disable hardware I/O
 * coherency, until enough testing is done with automatic I/O
 * synchronization barriers to validate that it is a proper solution.
 */
int coherency_available(void)
{
	return false;
}

int __init coherency_init(void)
{