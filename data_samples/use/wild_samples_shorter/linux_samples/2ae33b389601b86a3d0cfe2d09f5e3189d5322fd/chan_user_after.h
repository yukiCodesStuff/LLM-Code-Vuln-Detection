			       unsigned short *cols_out);
extern void generic_free(void *data);

struct tty_port;
extern void register_winch(int fd,  struct tty_port *port);
extern void register_winch_irq(int fd, int tty_fd, int pid,
			       struct tty_port *port, unsigned long stack);

#define __channel_help(fn, prefix) \
__uml_help(fn, prefix "[0-9]*=<channel description>\n" \
"    Attach a console or serial line to a host channel.  See\n" \