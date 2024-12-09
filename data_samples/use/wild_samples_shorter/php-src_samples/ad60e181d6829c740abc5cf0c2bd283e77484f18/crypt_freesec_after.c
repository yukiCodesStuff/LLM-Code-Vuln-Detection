	 */
	q = (u_char *) keybuf;
	while (q - (u_char *) keybuf < sizeof(keybuf)) {
		*q++ = *key << 1;
		if (*key)
			key++;
	}
	if (des_setkey((u_char *) keybuf, data))
		return(NULL);