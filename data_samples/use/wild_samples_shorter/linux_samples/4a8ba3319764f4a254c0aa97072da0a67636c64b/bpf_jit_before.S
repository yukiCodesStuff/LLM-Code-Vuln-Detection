 * skb_copy_bits takes 4 parameters:
 *   %r2 = skb pointer
 *   %r3 = offset into skb data
 *   %r4 = length to copy
 *   %r5 = pointer to temp buffer
 */
#define SKBDATA	%r8

	/* A = *(u32 *) (skb->data+K+X) */

sk_load_word_slow:
	lgr	%r9,%r2			# save %r2
	lhi	%r4,4			# 4 bytes
	la	%r5,160(%r15)		# pointer to temp buffer
	brasl	%r14,skb_copy_bits	# get data from skb
	l	%r5,160(%r15)		# load result from temp buffer
	ltgr	%r2,%r2			# set cc to (%r2 != 0)
	lgr	%r2,%r9			# restore %r2

sk_load_half_slow:
	lgr	%r9,%r2			# save %r2
	lhi	%r4,2			# 2 bytes
	la	%r5,162(%r15)		# pointer to temp buffer
	brasl	%r14,skb_copy_bits	# get data from skb
	xc	160(2,%r15),160(%r15)
	l	%r5,160(%r15)		# load result from temp buffer
	ltgr	%r2,%r2			# set cc to (%r2 != 0)

sk_load_byte_slow:
	lgr	%r9,%r2			# save %r2
	lhi	%r4,1			# 1 bytes
	la	%r5,163(%r15)		# pointer to temp buffer
	brasl	%r14,skb_copy_bits	# get data from skb
	xc	160(3,%r15),160(%r15)
	l	%r5,160(%r15)		# load result from temp buffer
	ltgr	%r2,%r2			# set cc to (%r2 != 0)
	lgr	%r2,%r9			# restore %r2
	br	%r8

	/* A = (*(u8 *)(skb->data+K) & 0xf) << 2 */
ENTRY(sk_load_byte_msh)
	llgfr	%r1,%r3			# extend offset
	clr	%r11,%r3		# hlen < offset ?
	jle	sk_load_byte_slow
	lhi	%r12,0
	ic	%r12,0(%r1,%r10)	# get byte from skb
	nill	%r12,0x0f
	sll	%r12,2

sk_load_byte_msh_slow:
	lgr	%r9,%r2			# save %r2
	lhi	%r4,2			# 2 bytes
	la	%r5,162(%r15)		# pointer to temp buffer
	brasl	%r14,skb_copy_bits	# get data from skb
	xc	160(3,%r15),160(%r15)
	l	%r12,160(%r15)		# load result from temp buffer
	nill	%r12,0x0f