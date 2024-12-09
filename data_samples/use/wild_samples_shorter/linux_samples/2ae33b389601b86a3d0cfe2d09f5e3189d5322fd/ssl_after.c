	.throttle 		= line_throttle,
	.unthrottle 		= line_unthrottle,
	.install		= ssl_install,
	.hangup			= line_hangup,
};

/* Changed by ssl_init and referenced by ssl_exit, which are both serialized