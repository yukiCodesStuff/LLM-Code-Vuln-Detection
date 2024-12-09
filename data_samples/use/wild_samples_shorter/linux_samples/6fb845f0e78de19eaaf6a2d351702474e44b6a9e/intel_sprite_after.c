
	keymax = (key->max_value & 0xffffff) | PLANE_KEYMAX_ALPHA(alpha);

	keymsk = key->channel_mask & 0x7ffffff;
	if (alpha < 0xff)
		keymsk |= PLANE_KEYMSK_ALPHA_ENABLE;

	/* The scaler will handle the output position */