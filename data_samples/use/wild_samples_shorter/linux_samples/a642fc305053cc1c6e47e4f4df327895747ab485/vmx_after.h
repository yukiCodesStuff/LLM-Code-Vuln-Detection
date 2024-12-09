#define EXIT_REASON_EPT_MISCONFIG       49
#define EXIT_REASON_INVEPT              50
#define EXIT_REASON_PREEMPTION_TIMER    52
#define EXIT_REASON_INVVPID             53
#define EXIT_REASON_WBINVD              54
#define EXIT_REASON_XSETBV              55
#define EXIT_REASON_APIC_WRITE          56
#define EXIT_REASON_INVPCID             58
	{ EXIT_REASON_EOI_INDUCED,           "EOI_INDUCED" }, \
	{ EXIT_REASON_INVALID_STATE,         "INVALID_STATE" }, \
	{ EXIT_REASON_INVD,                  "INVD" }, \
	{ EXIT_REASON_INVVPID,               "INVVPID" }, \
	{ EXIT_REASON_INVPCID,               "INVPCID" }

#endif /* _UAPIVMX_H */