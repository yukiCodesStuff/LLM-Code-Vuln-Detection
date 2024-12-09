void (*mach_halt)(void);
void (*mach_power_off)(void);

#ifdef CONFIG_M68328
#define CPU_NAME	"MC68328"
#endif
#ifdef CONFIG_M68EZ328