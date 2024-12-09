#ifndef __ARMEB__
	rev	w8, w8
#endif
	add	w10, w8, #1
	orr	v6.16b,v0.16b,v0.16b
	rev	w10, w10
	mov	v6.s[3],w10
	add	w8, w8, #2
	orr	v1.16b,v6.16b,v6.16b
	b.ls	.Lctr32_tail
	rev	w12, w8
	mov	v6.s[3],w12
	sub	x2,x2,#3		// bias
	orr	v18.16b,v6.16b,v6.16b
	cmp	x2,#2
	b.lo	.Loop3x_ctr32

	add	w13,w8,#1
	add	w14,w8,#2
	aese	v1.16b,v16.16b
	aesmc	v5.16b,v1.16b
	ld1	{v2.16b},[x0],#16
	add	w9,w8,#1
	aese	v18.16b,v16.16b
	aesmc	v18.16b,v18.16b
	ld1	{v3.16b},[x0],#16
	rev	w9,w9
	aese	v4.16b,v17.16b
	aesmc	v4.16b,v4.16b
	aese	v5.16b,v17.16b
	aesmc	v5.16b,v5.16b
	mov	x7,x3
	aese	v18.16b,v17.16b
	aesmc	v17.16b,v18.16b
	aese	v4.16b,v20.16b
	aesmc	v4.16b,v4.16b
	aese	v5.16b,v20.16b
	aesmc	v5.16b,v5.16b
	aese	v5.16b,v21.16b
	aesmc	v5.16b,v5.16b
	eor	v19.16b,v19.16b,v7.16b
	mov	v6.s[3], w9
	aese	v17.16b,v21.16b
	aesmc	v17.16b,v17.16b
	orr	v0.16b,v6.16b,v6.16b
	rev	w10,w10
	aese	v4.16b,v22.16b
	aesmc	v4.16b,v4.16b
	mov	v6.s[3], w10
	rev	w12,w8
	aese	v5.16b,v22.16b
	aesmc	v5.16b,v5.16b
	orr	v1.16b,v6.16b,v6.16b
	mov	v6.s[3], w12
	aese	v17.16b,v22.16b
	aesmc	v17.16b,v17.16b
	orr	v18.16b,v6.16b,v6.16b
	subs	x2,x2,#3
	aese	v4.16b,v23.16b
	aese	v5.16b,v23.16b
	aese	v17.16b,v23.16b