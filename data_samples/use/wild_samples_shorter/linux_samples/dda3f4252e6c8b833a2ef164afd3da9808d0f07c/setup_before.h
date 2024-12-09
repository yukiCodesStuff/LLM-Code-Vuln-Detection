};

void setup_rfi_flush(enum l1d_flush_type, bool enable);
void do_rfi_flush_fixups(enum l1d_flush_type types);
#ifdef CONFIG_PPC_BARRIER_NOSPEC
void setup_barrier_nospec(void);
#else
static inline void setup_barrier_nospec(void) { };
#endif
void do_barrier_nospec_fixups(bool enable);
extern bool barrier_nospec_enabled;

#ifdef CONFIG_PPC_BARRIER_NOSPEC