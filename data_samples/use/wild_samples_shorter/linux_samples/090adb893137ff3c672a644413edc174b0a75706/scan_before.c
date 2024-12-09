	memcpy(cmd->channels_2, cmd_channels->channels_2,
	       sizeof(cmd->channels_2));
	memcpy(cmd->channels_5, cmd_channels->channels_5,
	       sizeof(cmd->channels_2));
	/* channels_4 are not supported, so no need to copy them */
}

static int wl18xx_scan_send(struct wl1271 *wl, struct wl12xx_vif *wlvif,