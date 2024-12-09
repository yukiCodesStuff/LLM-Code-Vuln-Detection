				 int depth, int pitch);
extern void btext_setup_display(int width, int height, int depth, int pitch,
				unsigned long address);
extern void btext_prepare_BAT(void);
extern void btext_map(void);
extern void btext_unmap(void);

extern void btext_drawchar(char c);