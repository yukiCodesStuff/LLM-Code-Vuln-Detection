	f->y_map |= (p[5] & 0x20) << 6;
}

static void alps_process_touchpad_packet_v3(struct psmouse *psmouse)
{
	struct alps_data *priv = psmouse->private;
	unsigned char *packet = psmouse->packet;
	}

	/* Bytes 2 - pktsize should have 0 in the highest bit */
	if (psmouse->pktcnt >= 2 && psmouse->pktcnt <= psmouse->pktsize &&
	    (psmouse->packet[psmouse->pktcnt - 1] & 0x80)) {
		psmouse_dbg(psmouse, "refusing packet[%i] = %x\n",
			    psmouse->pktcnt - 1,
			    psmouse->packet[psmouse->pktcnt - 1]);
	return 0;
}

static int alps_enter_command_mode(struct psmouse *psmouse,
				   unsigned char *resp)
{
	unsigned char param[4];

	if (alps_rpt_cmd(psmouse, 0, PSMOUSE_CMD_RESET_WRAP, param)) {
		return -1;
	}

	if (param[0] != 0x88 || (param[1] != 0x07 && param[1] != 0x08)) {
		psmouse_dbg(psmouse,
			    "unknown response while entering command mode\n");
		return -1;
	}

	if (resp)
		*resp = param[2];
	return 0;
}

static inline int alps_exit_command_mode(struct psmouse *psmouse)
{
	int reg_val, ret = -1;

	if (alps_enter_command_mode(psmouse, NULL))
		return -1;

	reg_val = alps_command_mode_read_reg(psmouse, reg_base + 0x0008);
	if (reg_val == -1)
{
	int ret = -EIO, reg_val;

	if (alps_enter_command_mode(psmouse, NULL))
		goto error;

	reg_val = alps_command_mode_read_reg(psmouse, reg_base + 0x08);
	if (reg_val == -1)
		 * supported by this driver. If bit 1 isn't set the packet
		 * format is different.
		 */
		if (alps_enter_command_mode(psmouse, NULL) ||
		    alps_command_mode_write_reg(psmouse,
						reg_base + 0x08, 0x82) ||
		    alps_exit_command_mode(psmouse))
			ret = -EIO;
	    alps_setup_trackstick_v3(psmouse, ALPS_REG_BASE_PINNACLE) == -EIO)
		goto error;

	if (alps_enter_command_mode(psmouse, NULL) ||
	    alps_absolute_mode_v3(psmouse)) {
		psmouse_err(psmouse, "Failed to enter absolute mode\n");
		goto error;
	}
			priv->flags &= ~ALPS_DUALPOINT;
	}

	if (alps_enter_command_mode(psmouse, NULL) ||
	    alps_command_mode_read_reg(psmouse, 0xc2d9) == -1 ||
	    alps_command_mode_write_reg(psmouse, 0xc2cb, 0x00))
		goto error;

	struct ps2dev *ps2dev = &psmouse->ps2dev;
	unsigned char param[4];

	if (alps_enter_command_mode(psmouse, NULL))
		goto error;

	if (alps_absolute_mode_v4(psmouse)) {
		psmouse_err(psmouse, "Failed to enter absolute mode\n");
	return -1;
}

static void alps_set_defaults(struct alps_data *priv)
{
	priv->byte0 = 0x8f;
	priv->mask0 = 0x8f;
		priv->nibble_commands = alps_v4_nibble_commands;
		priv->addr_command = PSMOUSE_CMD_DISABLE;
		break;
	}
}

static int alps_match_table(struct psmouse *psmouse, struct alps_data *priv,
		return -EIO;

	if (alps_match_table(psmouse, priv, e7, ec) == 0) {
		return 0;
	} else if (ec[0] == 0x88 && ec[1] == 0x08) {
		priv->proto_version = ALPS_PROTO_V3;
		alps_set_defaults(priv);