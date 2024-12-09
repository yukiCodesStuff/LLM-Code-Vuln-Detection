/*
 * PCI Backend - Functions for creating a virtual configuration space for
 *               exported PCI Devices.
 *               It's dangerous to allow PCI Driver Domains to change their
 *               device's resources (memory, i/o ports, interrupts). We need to
 *               restrict changes to certain PCI Configuration registers:
 *               BARs, INTERRUPT_PIN, most registers in the header...
 *
 * Author: Ryan Wilson <hap9@epoch.ncsc.mil>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include "pciback.h"
#include "conf_space.h"
#include "conf_space_quirks.h"

static bool permissive;
module_param(permissive, bool, 0644);

/* This is where xen_pcibk_read_config_byte, xen_pcibk_read_config_word,
 * xen_pcibk_write_config_word, and xen_pcibk_write_config_byte are created. */
#define DEFINE_PCI_CONFIG(op, size, type)			\
int xen_pcibk_##op##_config_##size				\
(struct pci_dev *dev, int offset, type value, void *data)	\
{								\
	return pci_##op##_config_##size(dev, offset, value);	\
}

DEFINE_PCI_CONFIG(read, byte, u8 *)
DEFINE_PCI_CONFIG(read, word, u16 *)
DEFINE_PCI_CONFIG(read, dword, u32 *)

DEFINE_PCI_CONFIG(write, byte, u8)
DEFINE_PCI_CONFIG(write, word, u16)
DEFINE_PCI_CONFIG(write, dword, u32)

static int conf_space_read(struct pci_dev *dev,
			   const struct config_field_entry *entry,
			   int offset, u32 *value)
{
	int ret = 0;
	const struct config_field *field = entry->field;

	*value = 0;

	switch (field->size) {
	case 1:
		if (field->u.b.read)
			ret = field->u.b.read(dev, offset, (u8 *) value,
					      entry->data);
		break;
	case 2:
		if (field->u.w.read)
			ret = field->u.w.read(dev, offset, (u16 *) value,
					      entry->data);
		break;
	case 4:
		if (field->u.dw.read)
			ret = field->u.dw.read(dev, offset, value, entry->data);
		break;
	}