 * we put the segment information here.
 */
typedef struct {
	void *ldt;
	int size;

#ifdef CONFIG_X86_64
	/* True if mm supports a task running in 32 bit compatibility mode. */
	unsigned short ia32_compat;