/*
 * Routines to compress and uncompress tcp packets (for transmission
 * over low speed serial lines).
 *
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Van Jacobson (van@helios.ee.lbl.gov), Dec 31, 1989:
 *	- Initial distribution.
 *
 *
 * modified for KA9Q Internet Software Package by
 * Katie Stevens (dkstevens@ucdavis.edu)
 * University of California, Davis
 * Computing Services
 *	- 01-31-90	initial adaptation (from 1.19)
 *	PPP.05	02-15-90 [ks]
 *	PPP.08	05-02-90 [ks]	use PPP protocol field to signal compression
 *	PPP.15	09-90	 [ks]	improve mbuf handling
 *	PPP.16	11-02	 [karn]	substantially rewritten to use NOS facilities
 *
 *	- Feb 1991	Bill_Simpson@um.cc.umich.edu
 *			variable number of conversation slots
 *			allow zero or one slots
 *			separate routines
 *			status display
 *	- Jul 1994	Dmitry Gorodchanin
 *			Fixes for memory leaks.
 *      - Oct 1994      Dmitry Gorodchanin
 *                      Modularization.
 *	- Jan 1995	Bjorn Ekwall
 *			Use ip_fast_csum from ip.h
 *	- July 1995	Christos A. Polyzols
 *			Spotted bug in tcp option checking
 *
 *
 *	This module is a difficult issue. It's clearly inet code but it's also clearly
 *	driver code belonging close to PPP and SLIP
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <net/slhc_vj.h>

#ifdef CONFIG_INET
/* Entire module is for IP only */
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/termios.h>
#include <linux/in.h>
#include <linux/fcntl.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/icmp.h>
#include <net/tcp.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <net/checksum.h>
#include <asm/unaligned.h>

static unsigned char *encode(unsigned char *cp, unsigned short n);
static long decode(unsigned char **cpp);
static unsigned char * put16(unsigned char *cp, unsigned short x);
static unsigned short pull16(unsigned char **cpp);

/* Initialize compression data structure
 *	slots must be in range 0 to 255 (zero meaning no compression)
 */
struct slcompress *
slhc_init(int rslots, int tslots)
{
	register short i;
	register struct cstate *ts;
	struct slcompress *comp;

	comp = kzalloc(sizeof(struct slcompress), GFP_KERNEL);
	if (! comp)
		goto out_fail;

	if ( rslots > 0  &&  rslots < 256 ) {
		size_t rsize = rslots * sizeof(struct cstate);
		comp->rstate = kzalloc(rsize, GFP_KERNEL);
		if (! comp->rstate)
			goto out_free;
		comp->rslot_limit = rslots - 1;
	}

	if ( tslots > 0  &&  tslots < 256 ) {
		size_t tsize = tslots * sizeof(struct cstate);
		comp->tstate = kzalloc(tsize, GFP_KERNEL);
		if (! comp->tstate)
			goto out_free2;
		comp->tslot_limit = tslots - 1;
	}

	comp->xmit_oldest = 0;
	comp->xmit_current = 255;
	comp->recv_current = 255;
	/*
	 * don't accept any packets with implicit index until we get
	 * one with an explicit index.  Otherwise the uncompress code
	 * will try to use connection 255, which is almost certainly
	 * out of range
	 */
	comp->flags |= SLF_TOSS;

	if ( tslots > 0 ) {
		ts = comp->tstate;
		for(i = comp->tslot_limit; i > 0; --i){
			ts[i].cs_this = i;
			ts[i].next = &(ts[i - 1]);
		}
		ts[0].next = &(ts[comp->tslot_limit]);
		ts[0].cs_this = 0;
	}
	return comp;

out_free2:
	kfree(comp->rstate);
out_free:
	kfree(comp);
out_fail:
	return NULL;
}
{
	register short i;
	register struct cstate *ts;
	struct slcompress *comp;

	comp = kzalloc(sizeof(struct slcompress), GFP_KERNEL);
	if (! comp)
		goto out_fail;

	if ( rslots > 0  &&  rslots < 256 ) {
		size_t rsize = rslots * sizeof(struct cstate);
		comp->rstate = kzalloc(rsize, GFP_KERNEL);
		if (! comp->rstate)
			goto out_free;
		comp->rslot_limit = rslots - 1;
	}
		for(i = comp->tslot_limit; i > 0; --i){
			ts[i].cs_this = i;
			ts[i].next = &(ts[i - 1]);
		}
		ts[0].next = &(ts[comp->tslot_limit]);
		ts[0].cs_this = 0;
	}
	return comp;

out_free2:
	kfree(comp->rstate);
out_free:
	kfree(comp);
out_fail:
	return NULL;
}


/* Free a compression data structure */
void
slhc_free(struct slcompress *comp)
{
	if ( comp == NULLSLCOMPR )
		return;

	if ( comp->tstate != NULLSLSTATE )
		kfree( comp->tstate );

	if ( comp->rstate != NULLSLSTATE )
		kfree( comp->rstate );

	kfree( comp );
}


/* Put a short in host order into a char array in network order */
static inline unsigned char *
put16(unsigned char *cp, unsigned short x)
{
	*cp++ = x >> 8;
	*cp++ = x;

	return cp;
}


/* Encode a number */
static unsigned char *
encode(unsigned char *cp, unsigned short n)
{
	if(n >= 256 || n == 0){
		*cp++ = 0;
		cp = put16(cp,n);
	} else {
		*cp++ = n;
	}
	return cp;
}