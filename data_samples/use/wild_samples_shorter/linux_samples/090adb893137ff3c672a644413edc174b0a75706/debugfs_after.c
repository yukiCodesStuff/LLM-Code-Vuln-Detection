static struct dentry *mwifiex_dfs_dir;

static char *bss_modes[] = {
	"UNSPECIFIED",
	"ADHOC",
	"STATION",
	"AP",
	"AP_VLAN",
	"WDS",
	"MONITOR",
	"MESH_POINT",
	"P2P_CLIENT",
	"P2P_GO",
	"P2P_DEVICE",
};

/* size/addr for mwifiex_debug_info */
#define item_size(n)		(FIELD_SIZEOF(struct mwifiex_debug_info, n))
	p += sprintf(p, "driver_version = %s", fmt);
	p += sprintf(p, "\nverext = %s", priv->version_str);
	p += sprintf(p, "\ninterface_name=\"%s\"\n", netdev->name);

	if (info.bss_mode >= ARRAY_SIZE(bss_modes))
		p += sprintf(p, "bss_mode=\"%d\"\n", info.bss_mode);
	else
		p += sprintf(p, "bss_mode=\"%s\"\n", bss_modes[info.bss_mode]);

	p += sprintf(p, "media_state=\"%s\"\n",
		     (!priv->media_connected ? "Disconnected" : "Connected"));
	p += sprintf(p, "mac_address=\"%pM\"\n", netdev->dev_addr);
