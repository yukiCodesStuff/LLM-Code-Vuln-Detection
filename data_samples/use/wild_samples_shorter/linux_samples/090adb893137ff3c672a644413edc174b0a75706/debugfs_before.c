static struct dentry *mwifiex_dfs_dir;

static char *bss_modes[] = {
	"Unknown",
	"Ad-hoc",
	"Managed",
	"Auto"
};

/* size/addr for mwifiex_debug_info */
#define item_size(n)		(FIELD_SIZEOF(struct mwifiex_debug_info, n))
	p += sprintf(p, "driver_version = %s", fmt);
	p += sprintf(p, "\nverext = %s", priv->version_str);
	p += sprintf(p, "\ninterface_name=\"%s\"\n", netdev->name);
	p += sprintf(p, "bss_mode=\"%s\"\n", bss_modes[info.bss_mode]);
	p += sprintf(p, "media_state=\"%s\"\n",
		     (!priv->media_connected ? "Disconnected" : "Connected"));
	p += sprintf(p, "mac_address=\"%pM\"\n", netdev->dev_addr);
