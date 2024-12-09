	cytp->fw_version = param[2] & FW_VERSION_MASX;
	cytp->tp_metrics_supported = (param[2] & TP_METRICS_MASK) ? 1 : 0;

	/*
	 * Trackpad fw_version 11 (in Dell XPS12) yields a bogus response to
	 * CYTP_CMD_READ_TP_METRICS so do not try to use it. LP: #1103594.
	 */
	if (cytp->fw_version >= 11)
		cytp->tp_metrics_supported = 0;

	psmouse_dbg(psmouse, "cytp->fw_version = %d\n", cytp->fw_version);
	psmouse_dbg(psmouse, "cytp->tp_metrics_supported = %d\n",
		 cytp->tp_metrics_supported);

	cytp->tp_res_x = cytp->tp_max_abs_x / cytp->tp_width;
	cytp->tp_res_y = cytp->tp_max_abs_y / cytp->tp_high;

	if (!cytp->tp_metrics_supported)
		return 0;

	memset(param, 0, sizeof(param));
	if (cypress_send_ext_cmd(psmouse, CYTP_CMD_READ_TP_METRICS, param) == 0) {
		/* Update trackpad parameters. */
		cytp->tp_max_abs_x = (param[1] << 8) | param[0];

static int cypress_query_hardware(struct psmouse *psmouse)
{
	int ret;

	ret = cypress_read_fw_version(psmouse);
	if (ret)
		return ret;

	ret = cypress_read_tp_metrics(psmouse);
	if (ret)
		return ret;

	return 0;
}
