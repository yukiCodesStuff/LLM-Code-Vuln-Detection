				 int depth, int pitch);
extern void btext_setup_display(int width, int height, int depth, int pitch,
				unsigned long address);
#ifdef CONFIG_PPC32
extern void btext_prepare_BAT(void);
#else
static inline void btext_prepare_BAT(void) { }
#endif
extern void btext_map(void);
extern void btext_unmap(void);

extern void btext_drawchar(char c);