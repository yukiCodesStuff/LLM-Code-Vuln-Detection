	f->y_map |= (p[5] & 0x20) << 6;
}

static void alps_decode_dolphin(struct alps_fields *f, unsigned char *p)
{
	f->first_mp = !!(p[0] & 0x02);
	f->is_mp = !!(p[0] & 0x20);

	f->fingers = ((p[0] & 0x6) >> 1 |
		     (p[0] & 0x10) >> 2);
	f->x_map = ((p[2] & 0x60) >> 5) |
		   ((p[4] & 0x7f) << 2) |
		   ((p[5] & 0x7f) << 9) |
		   ((p[3] & 0x07) << 16) |
		   ((p[3] & 0x70) << 15) |
		   ((p[0] & 0x01) << 22);
	f->y_map = (p[1] & 0x7f) |
		   ((p[2] & 0x1f) << 7);

	f->x = ((p[1] & 0x7f) | ((p[4] & 0x0f) << 7));
	f->y = ((p[2] & 0x7f) | ((p[4] & 0xf0) << 3));
	f->z = (p[0] & 4) ? 0 : p[5] & 0x7f;

	alps_decode_buttons_v3(f, p);
}

static void alps_process_touchpad_packet_v3(struct psmouse *psmouse)
{
	struct alps_data *priv = psmouse->private;
	unsigned char *packet = psmouse->packet;
	}

	/* Bytes 2 - pktsize should have 0 in the highest bit */
	if (priv->proto_version != ALPS_PROTO_V5 &&
	    psmouse->pktcnt >= 2 && psmouse->pktcnt <= psmouse->pktsize &&
	    (psmouse->packet[psmouse->pktcnt - 1] & 0x80)) {
		psmouse_dbg(psmouse, "refusing packet[%i] = %x\n",
			    psmouse->pktcnt - 1,
			    psmouse->packet[psmouse->pktcnt - 1]);
	return 0;
}

static int alps_enter_command_mode(struct psmouse *psmouse)
{
	unsigned char param[4];

	if (alps_rpt_cmd(psmouse, 0, PSMOUSE_CMD_RESET_WRAP, param)) {
		return -1;
	}

	if ((param[0] != 0x88 || (param[1] != 0x07 && param[1] != 0x08)) &&
	    param[0] != 0x73) {
		psmouse_dbg(psmouse,
			    "unknown response while entering command mode\n");
		return -1;
	}
	return 0;
}

static inline int alps_exit_command_mode(struct psmouse *psmouse)
{
	int reg_val, ret = -1;

	if (alps_enter_command_mode(psmouse))
		return -1;

	reg_val = alps_command_mode_read_reg(psmouse, reg_base + 0x0008);
	if (reg_val == -1)
{
	int ret = -EIO, reg_val;

	if (alps_enter_command_mode(psmouse))
		goto error;

	reg_val = alps_command_mode_read_reg(psmouse, reg_base + 0x08);
	if (reg_val == -1)
		 * supported by this driver. If bit 1 isn't set the packet
		 * format is different.
		 */
		if (alps_enter_command_mode(psmouse) ||
		    alps_command_mode_write_reg(psmouse,
						reg_base + 0x08, 0x82) ||
		    alps_exit_command_mode(psmouse))
			ret = -EIO;
	    alps_setup_trackstick_v3(psmouse, ALPS_REG_BASE_PINNACLE) == -EIO)
		goto error;

	if (alps_enter_command_mode(psmouse) ||
	    alps_absolute_mode_v3(psmouse)) {
		psmouse_err(psmouse, "Failed to enter absolute mode\n");
		goto error;
	}
			priv->flags &= ~ALPS_DUALPOINT;
	}

	if (alps_enter_command_mode(psmouse) ||
	    alps_command_mode_read_reg(psmouse, 0xc2d9) == -1 ||
	    alps_command_mode_write_reg(psmouse, 0xc2cb, 0x00))
		goto error;

	struct ps2dev *ps2dev = &psmouse->ps2dev;
	unsigned char param[4];

	if (alps_enter_command_mode(psmouse))
		goto error;

	if (alps_absolute_mode_v4(psmouse)) {
		psmouse_err(psmouse, "Failed to enter absolute mode\n");
	return -1;
}

static int alps_hw_init_dolphin_v1(struct psmouse *psmouse)
{
	struct ps2dev *ps2dev = &psmouse->ps2dev;
	unsigned char param[2];

	/* This is dolphin "v1" as empirically defined by florin9doi */
	param[0] = 0x64;
	param[1] = 0x28;

	if (ps2_command(ps2dev, NULL, PSMOUSE_CMD_SETSTREAM) ||
	    ps2_command(ps2dev, &param[0], PSMOUSE_CMD_SETRATE) ||
	    ps2_command(ps2dev, &param[1], PSMOUSE_CMD_SETRATE))
		return -1;

	return 0;
}

static void alps_set_defaults(struct alps_data *priv)
{
	priv->byte0 = 0x8f;
	priv->mask0 = 0x8f;
		priv->nibble_commands = alps_v4_nibble_commands;
		priv->addr_command = PSMOUSE_CMD_DISABLE;
		break;
	case ALPS_PROTO_V5:
		priv->hw_init = alps_hw_init_dolphin_v1;
		priv->process_packet = alps_process_packet_v3;
		priv->decode_fields = alps_decode_dolphin;
		priv->set_abs_params = alps_set_abs_params_mt;
		priv->nibble_commands = alps_v3_nibble_commands;
		priv->addr_command = PSMOUSE_CMD_RESET_WRAP;
		priv->byte0 = 0xc8;
		priv->mask0 = 0xc8;
		priv->flags = 0;
		priv->x_max = 1360;
		priv->y_max = 660;
		priv->x_bits = 23;
		priv->y_bits = 12;
		break;
	}
}

static int alps_match_table(struct psmouse *psmouse, struct alps_data *priv,
		return -EIO;

	if (alps_match_table(psmouse, priv, e7, ec) == 0) {
		return 0;
	} else if (e7[0] == 0x73 && e7[1] == 0x03 && e7[2] == 0x50 &&
		   ec[0] == 0x73 && ec[1] == 0x01) {
		priv->proto_version = ALPS_PROTO_V5;
		alps_set_defaults(priv);

		return 0;
	} else if (ec[0] == 0x88 && ec[1] == 0x08) {
		priv->proto_version = ALPS_PROTO_V3;
		alps_set_defaults(priv);