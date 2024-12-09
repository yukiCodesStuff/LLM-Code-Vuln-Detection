#include <asm/intel-mid.h>
#include <asm/intel_scu_ipc.h>
#include <asm/io_apic.h>
#include <asm/hw_irq.h>

#define TANGIER_EXT_TIMER0_MSI 12

static struct platform_device wdt_dev = {