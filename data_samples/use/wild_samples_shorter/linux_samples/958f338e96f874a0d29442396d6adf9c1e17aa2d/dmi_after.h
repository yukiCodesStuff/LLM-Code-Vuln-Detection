
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/io.h>

#include <asm/setup.h>

static __always_inline __init void *dmi_alloc(unsigned len)
{