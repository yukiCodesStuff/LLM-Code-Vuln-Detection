};

void setup_rfi_flush(enum l1d_flush_type, bool enable);
void setup_entry_flush(bool enable);
void setup_uaccess_flush(bool enable);
void do_rfi_flush_fixups(enum l1d_flush_type types);
#ifdef CONFIG_PPC_BARRIER_NOSPEC
void setup_barrier_nospec(void);
#else
static inline void setup_barrier_nospec(void) { };
#endif
void do_uaccess_flush_fixups(enum l1d_flush_type types);
void do_entry_flush_fixups(enum l1d_flush_type types);
void do_barrier_nospec_fixups(bool enable);
extern bool barrier_nospec_enabled;

#ifdef CONFIG_PPC_BARRIER_NOSPEC