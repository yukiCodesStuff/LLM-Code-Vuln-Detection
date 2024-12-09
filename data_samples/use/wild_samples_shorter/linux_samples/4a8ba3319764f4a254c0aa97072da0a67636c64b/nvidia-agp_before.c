/*
 * Nvidia AGPGART routines.
 * Based upon a 2.4 agpgart diff by the folks from NVIDIA, and hacked up
 * to work in 2.5 by Dave Jones <davej@redhat.com>
 */

#include <linux/module.h>
#include <linux/pci.h>