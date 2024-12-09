#include "pciback.h"
#include "conf_space.h"

struct pci_bar_info {
	u32 val;
	u32 len_val;
	int which;
#define is_enable_cmd(value) ((value)&(PCI_COMMAND_MEMORY|PCI_COMMAND_IO))
#define is_master_cmd(value) ((value)&PCI_COMMAND_MASTER)

static int command_read(struct pci_dev *dev, int offset, u16 *value, void *data)
{
	int i;
	int ret;

	ret = xen_pcibk_read_config_word(dev, offset, value, data);
	if (!pci_is_enabled(dev))
		return ret;

	for (i = 0; i < PCI_ROM_RESOURCE; i++) {
		if (dev->resource[i].flags & IORESOURCE_IO)
			*value |= PCI_COMMAND_IO;
		if (dev->resource[i].flags & IORESOURCE_MEM)
			*value |= PCI_COMMAND_MEMORY;
	}

	return ret;
}

static int command_write(struct pci_dev *dev, int offset, u16 value, void *data)
{
	struct xen_pcibk_dev_data *dev_data;
	int err;

	dev_data = pci_get_drvdata(dev);
	if (!pci_is_enabled(dev) && is_enable_cmd(value)) {
		if (unlikely(verbose_request))
		}
	}

	return pci_write_config_word(dev, offset, value);
}

static int rom_write(struct pci_dev *dev, int offset, u32 value, void *data)
	{
	 .offset    = PCI_COMMAND,
	 .size      = 2,
	 .u.w.read  = command_read,
	 .u.w.write = command_write,
	},
	{