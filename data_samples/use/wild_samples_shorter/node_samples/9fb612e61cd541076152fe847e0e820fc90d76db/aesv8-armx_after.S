#ifndef __ARMEB__
	rev	w8, w8
#endif
	orr	v1.16b,v0.16b,v0.16b
	add	w10, w8, #1
	orr	v18.16b,v0.16b,v0.16b
	add	w8, w8, #2
	orr	v6.16b,v0.16b,v0.16b
	rev	w10, w10
	mov	v1.s[3],w10
	b.ls	.Lctr32_tail
	rev	w12, w8
	sub	x2,x2,#3		// bias
	mov	v18.s[3],w12
	cmp	x2,#32
	b.lo	.Loop3x_ctr32

	add	w13,w8,#1
	add	w14,w8,#2
	aese	v1.16b,v16.16b
	aesmc	v5.16b,v1.16b
	ld1	{v2.16b},[x0],#16
	orr	v0.16b,v6.16b,v6.16b
	aese	v18.16b,v16.16b
	aesmc	v18.16b,v18.16b
	ld1	{v3.16b},[x0],#16
	orr	v1.16b,v6.16b,v6.16b
	aese	v4.16b,v17.16b
	aesmc	v4.16b,v4.16b
	aese	v5.16b,v17.16b
	aesmc	v5.16b,v5.16b
	mov	x7,x3
	aese	v18.16b,v17.16b
	aesmc	v17.16b,v18.16b
	orr	v18.16b,v6.16b,v6.16b
	add	w9,w8,#1
	aese	v4.16b,v20.16b
	aesmc	v4.16b,v4.16b
	aese	v5.16b,v20.16b
	aesmc	v5.16b,v5.16b
	aese	v5.16b,v21.16b
	aesmc	v5.16b,v5.16b
	eor	v19.16b,v19.16b,v7.16b
	rev	w9,w9
	aese	v17.16b,v21.16b
	aesmc	v17.16b,v17.16b
	mov	v0.s[3], w9
	rev	w10,w10
	aese	v4.16b,v22.16b
	aesmc	v4.16b,v4.16b
	aese	v5.16b,v22.16b
	aesmc	v5.16b,v5.16b
	mov	v1.s[3], w10
	rev	w12,w8
	aese	v17.16b,v22.16b
	aesmc	v17.16b,v17.16b
	mov	v18.s[3], w12
	subs	x2,x2,#3
	aese	v4.16b,v23.16b
	aese	v5.16b,v23.16b
	aese	v17.16b,v23.16b