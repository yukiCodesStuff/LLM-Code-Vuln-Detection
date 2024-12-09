static const struct tty_operations ssl_ops = {
	.open 	 		= line_open,
	.close 	 		= line_close,
	.write 	 		= line_write,
	.put_char 		= line_put_char,
	.write_room		= line_write_room,
	.chars_in_buffer 	= line_chars_in_buffer,
	.flush_buffer 		= line_flush_buffer,
	.flush_chars 		= line_flush_chars,
	.set_termios 		= line_set_termios,
	.throttle 		= line_throttle,
	.unthrottle 		= line_unthrottle,
	.install		= ssl_install,
	.hangup			= line_hangup,
};

/* Changed by ssl_init and referenced by ssl_exit, which are both serialized
 * by being an initcall and exitcall, respectively.
 */
static int ssl_init_done = 0;

static void ssl_console_write(struct console *c, const char *string,
			      unsigned len)
{
	struct line *line = &serial_lines[c->index];
	unsigned long flags;

	spin_lock_irqsave(&line->lock, flags);
	console_write_chan(line->chan_out, string, len);
	spin_unlock_irqrestore(&line->lock, flags);
}