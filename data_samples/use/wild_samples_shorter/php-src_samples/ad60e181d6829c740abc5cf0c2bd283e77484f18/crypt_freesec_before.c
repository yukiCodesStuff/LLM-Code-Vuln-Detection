	 */
	q = (u_char *) keybuf;
	while (q - (u_char *) keybuf < sizeof(keybuf)) {
		if ((*q++ = *key << 1))
			key++;
	}
	if (des_setkey((u_char *) keybuf, data))
		return(NULL);