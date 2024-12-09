extern int console_open_chan(struct line *line, struct console *co);
extern void deactivate_chan(struct chan *chan, int irq);
extern void reactivate_chan(struct chan *chan, int irq);
extern void chan_enable_winch(struct chan *chan, struct tty_port *port);
extern int enable_chan(struct line *line);
extern void close_chan(struct line *line);
extern int chan_window_size(struct line *line, 
			     unsigned short *rows_out, 