
	.if (klen == KEY_128)
		.if (load_keys)
			vmovdqa	3*16(p_keys), xkey4
		.endif
	.else
		vmovdqa	3*16(p_keys), xkeyA
	.endif
	add	$(16*by), p_in

	.if (klen == KEY_128)
		vmovdqa	4*16(p_keys), xkeyB
	.else
		.if (load_keys)
			vmovdqa	4*16(p_keys), xkey4
		.endif
	.set i, 0
	.rept by
		club XDATA, i
		/* key 3 */
		.if (klen == KEY_128)
			vaesenc	xkey4, var_xdata, var_xdata
		.else
			vaesenc	xkeyA, var_xdata, var_xdata
		.endif
		.set i, (i +1)
	.endr

	vmovdqa	5*16(p_keys), xkeyA
	.set i, 0
	.rept by
		club XDATA, i
		/* key 4 */
		.if (klen == KEY_128)
			vaesenc	xkeyB, var_xdata, var_xdata
		.else
			vaesenc	xkey4, var_xdata, var_xdata
		.endif
		.set i, (i +1)
	.endr

	.if (klen == KEY_128)
		.if (load_keys)
			vmovdqa	6*16(p_keys), xkey8
		.endif
	.else
		vmovdqa	6*16(p_keys), xkeyB
	.endif
	.set i, 0
	.rept by
		club XDATA, i
		/* key 6 */
		.if (klen == KEY_128)
			vaesenc	xkey8, var_xdata, var_xdata
		.else
			vaesenc	xkeyB, var_xdata, var_xdata
		.endif
		.set i, (i +1)
	.endr

	.if (klen == KEY_128)
		vmovdqa	8*16(p_keys), xkeyB
	.else
		.if (load_keys)
			vmovdqa	8*16(p_keys), xkey8
		.endif

	.if (klen == KEY_128)
		.if (load_keys)
			vmovdqa	9*16(p_keys), xkey12
		.endif
	.else
		vmovdqa	9*16(p_keys), xkeyA
	.endif
	.set i, 0
	.rept by
		club XDATA, i
		/* key 8 */
		.if (klen == KEY_128)
			vaesenc	xkeyB, var_xdata, var_xdata
		.else
			vaesenc	xkey8, var_xdata, var_xdata
		.endif
		.set i, (i +1)
	.endr

	vmovdqa	10*16(p_keys), xkeyB
	.set i, 0
	.rept by
		club XDATA, i
		/* key 9 */
		.if (klen == KEY_128)
			vaesenc	xkey12, var_xdata, var_xdata
		.else
			vaesenc	xkeyA, var_xdata, var_xdata
		.endif
		.set i, (i +1)
	.endr

	.if (klen != KEY_128)
/* main body of aes ctr load */

.macro do_aes_ctrmain key_len
	cmp	$16, num_bytes
	jb	.Ldo_return2\key_len

	vmovdqa	byteswap_const(%rip), xbyteswap