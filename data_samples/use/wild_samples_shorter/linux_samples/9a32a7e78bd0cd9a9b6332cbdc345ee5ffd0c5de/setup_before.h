#else
static inline void setup_barrier_nospec(void) { };
#endif
void do_entry_flush_fixups(enum l1d_flush_type types);
void do_barrier_nospec_fixups(bool enable);
extern bool barrier_nospec_enabled;
