	.id_table	= snbep_uncore_pci_ids,
};

/*
 * build pci bus to socket mapping
 */
static int snbep_pci2phy_map_init(int devid, int nodeid_loc, int idmap_loc, bool reverse)
		err = pci_read_config_dword(ubox_dev, nodeid_loc, &config);
		if (err)
			break;
		nodeid = config;
		/* get the Node ID mapping */
		err = pci_read_config_dword(ubox_dev, idmap_loc, &config);
		if (err)
			break;