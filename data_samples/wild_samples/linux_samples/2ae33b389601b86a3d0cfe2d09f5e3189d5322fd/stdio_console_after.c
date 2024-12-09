static const struct tty_operations console_ops = {
	.open 	 		= line_open,
	.install		= con_install,
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
	.hangup			= line_hangup,
};

static void uml_console_write(struct console *console, const char *string,
			      unsigned len)
{
	struct line *line = &vts[console->index];
	unsigned long flags;

	spin_lock_irqsave(&line->lock, flags);
	console_write_chan(line->chan_out, string, len);
	spin_unlock_irqrestore(&line->lock, flags);
}