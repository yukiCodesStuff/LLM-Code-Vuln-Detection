	void *data;
};

#define OFFSET(cfg_entry) ((cfg_entry)->base_offset+(cfg_entry)->field->offset)

/* Add fields to a device - the add_fields macro expects to get a pointer to
 * the first entry in an array (of which the ending is marked by size==0)