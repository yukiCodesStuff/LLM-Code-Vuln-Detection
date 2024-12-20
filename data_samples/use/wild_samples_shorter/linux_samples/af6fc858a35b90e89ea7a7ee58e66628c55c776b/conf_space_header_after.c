#include "pciback.h"
#include "conf_space.h"

struct pci_cmd_info {
	u16 val;
};

struct pci_bar_info {
	u32 val;
	u32 len_val;
	int which;
#define is_enable_cmd(value) ((value)&(PCI_COMMAND_MEMORY|PCI_COMMAND_IO))
#define is_master_cmd(value) ((value)&PCI_COMMAND_MASTER)

/* Bits guests are allowed to control in permissive mode. */
#define PCI_COMMAND_GUEST (PCI_COMMAND_MASTER|PCI_COMMAND_SPECIAL| \
			   PCI_COMMAND_INVALIDATE|PCI_COMMAND_VGA_PALETTE| \
			   PCI_COMMAND_WAIT|PCI_COMMAND_FAST_BACK)

static void *command_init(struct pci_dev *dev, int offset)
{
	struct pci_cmd_info *cmd = kmalloc(sizeof(*cmd), GFP_KERNEL);
	int err;

	if (!cmd)
		return ERR_PTR(-ENOMEM);

	err = pci_read_config_word(dev, PCI_COMMAND, &cmd->val);
	if (err) {
		kfree(cmd);
		return ERR_PTR(err);
	}

	return cmd;
}

static int command_read(struct pci_dev *dev, int offset, u16 *value, void *data)
{
	int ret = pci_read_config_word(dev, offset, value);
	const struct pci_cmd_info *cmd = data;

	*value &= PCI_COMMAND_GUEST;
	*value |= cmd->val & ~PCI_COMMAND_GUEST;

	return ret;
}

static int command_write(struct pci_dev *dev, int offset, u16 value, void *data)
{
	struct xen_pcibk_dev_data *dev_data;
	int err;
	u16 val;
	struct pci_cmd_info *cmd = data;

	dev_data = pci_get_drvdata(dev);
	if (!pci_is_enabled(dev) && is_enable_cmd(value)) {
		if (unlikely(verbose_request))
		}
	}

	cmd->val = value;

	if (!permissive && (!dev_data || !dev_data->permissive))
		return 0;

	/* Only allow the guest to control certain bits. */
	err = pci_read_config_word(dev, offset, &val);
	if (err || val == value)
		return err;

	value &= PCI_COMMAND_GUEST;
	value |= val & ~PCI_COMMAND_GUEST;

	return pci_write_config_word(dev, offset, value);
}

static int rom_write(struct pci_dev *dev, int offset, u32 value, void *data)
	{
	 .offset    = PCI_COMMAND,
	 .size      = 2,
	 .init      = command_init,
	 .release   = bar_release,
	 .u.w.read  = command_read,
	 .u.w.write = command_write,
	},
	{