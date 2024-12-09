struct config_field_entry {
	struct list_head list;
	const struct config_field *field;
	unsigned int base_offset;
	void *data;
};

#define OFFSET(cfg_entry) ((cfg_entry)->base_offset+(cfg_entry)->field->offset)

/* Add fields to a device - the add_fields macro expects to get a pointer to
 * the first entry in an array (of which the ending is marked by size==0)
 */
int xen_pcibk_config_add_field_offset(struct pci_dev *dev,
				    const struct config_field *field,
				    unsigned int offset);

static inline int xen_pcibk_config_add_field(struct pci_dev *dev,
					   const struct config_field *field)
{
	return xen_pcibk_config_add_field_offset(dev, field, 0);
}