	.set_termios 		= line_set_termios,
	.throttle 		= line_throttle,
	.unthrottle 		= line_unthrottle,
	.hangup			= line_hangup,
};

static void uml_console_write(struct console *console, const char *string,