			       unsigned short *cols_out);
extern void generic_free(void *data);

struct tty_struct;
extern void register_winch(int fd,  struct tty_struct *tty);
extern void register_winch_irq(int fd, int tty_fd, int pid,
			       struct tty_struct *tty, unsigned long stack);

#define __channel_help(fn, prefix) \
__uml_help(fn, prefix "[0-9]*=<channel description>\n" \
"    Attach a console or serial line to a host channel.  See\n" \