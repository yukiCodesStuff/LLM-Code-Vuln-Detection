{
	int		i;
	uint32_t	count, salt, l, r0, r1, keybuf[2];
	u_char		*p, *q;

	if (!data->initialized)
		des_init_local(data);

	/*
	 * Copy the key, shifting each character up by one bit
	 * and padding with zeros.
	 */
	q = (u_char *) keybuf;
	while (q - (u_char *) keybuf < sizeof(keybuf)) {
		*q++ = *key << 1;
		if (*key)
			key++;
	}