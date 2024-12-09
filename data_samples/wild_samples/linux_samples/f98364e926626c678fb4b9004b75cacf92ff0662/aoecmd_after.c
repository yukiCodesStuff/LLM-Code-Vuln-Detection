	for_each_netdev_rcu(&init_net, ifp) {
		dev_hold(ifp);
		if (!is_aoe_netif(ifp)) {
			dev_put(ifp);
			continue;
		}

		skb = new_skb(sizeof *h + sizeof *ch);
		if (skb == NULL) {
			printk(KERN_INFO "aoe: skb alloc failure\n");
			dev_put(ifp);
			continue;
		}
		skb_put(skb, sizeof *h + sizeof *ch);
		skb->dev = ifp;
		__skb_queue_tail(queue, skb);
		h = (struct aoe_hdr *) skb_mac_header(skb);
		memset(h, 0, sizeof *h + sizeof *ch);

		memset(h->dst, 0xff, sizeof h->dst);
		memcpy(h->src, ifp->dev_addr, sizeof h->src);
		h->type = __constant_cpu_to_be16(ETH_P_AOE);
		h->verfl = AOE_HVER;
		h->major = cpu_to_be16(aoemajor);
		h->minor = aoeminor;
		h->cmd = AOECMD_CFG;
	}
		if (skb == NULL) {
			printk(KERN_INFO "aoe: skb alloc failure\n");
			dev_put(ifp);
			continue;
		}
		skb_put(skb, sizeof *h + sizeof *ch);
		skb->dev = ifp;
		__skb_queue_tail(queue, skb);
		h = (struct aoe_hdr *) skb_mac_header(skb);
		memset(h, 0, sizeof *h + sizeof *ch);

		memset(h->dst, 0xff, sizeof h->dst);
		memcpy(h->src, ifp->dev_addr, sizeof h->src);
		h->type = __constant_cpu_to_be16(ETH_P_AOE);
		h->verfl = AOE_HVER;
		h->major = cpu_to_be16(aoemajor);
		h->minor = aoeminor;
		h->cmd = AOECMD_CFG;
	}
	rcu_read_unlock();
}

static void
resend(struct aoedev *d, struct frame *f)
{
	struct sk_buff *skb;
	struct sk_buff_head queue;
	struct aoe_hdr *h;
	struct aoetgt *t;
	char buf[128];
	u32 n;

	t = f->t;
	n = newtag(d);
	skb = f->skb;
	if (ifrotate(t) == NULL) {
		/* probably can't happen, but set it up to fail anyway */
		pr_info("aoe: resend: no interfaces to rotate to.\n");
		ktcomplete(f, NULL);
		return;
	}
	h = (struct aoe_hdr *) skb_mac_header(skb);

	if (!(f->flags & FFL_PROBE)) {
		snprintf(buf, sizeof(buf),
			"%15s e%ld.%d oldtag=%08x@%08lx newtag=%08x s=%pm d=%pm nout=%d\n",
			"retransmit", d->aoemajor, d->aoeminor,
			f->tag, jiffies, n,
			h->src, h->dst, t->nout);
		aoechr_error(buf);
	}

	f->tag = n;
	fhash(f);
	h->tag = cpu_to_be32(n);
	memcpy(h->dst, t->addr, sizeof h->dst);
	memcpy(h->src, t->ifp->nd->dev_addr, sizeof h->src);

	skb->dev = t->ifp->nd;
	skb = skb_clone(skb, GFP_ATOMIC);
	if (skb == NULL)
		return;
	f->sent = ktime_get();
	__skb_queue_head_init(&queue);
	__skb_queue_tail(&queue, skb);
	aoenet_xmit(&queue);
}

static int
tsince_hr(struct frame *f)
{
	u64 delta = ktime_to_ns(ktime_sub(ktime_get(), f->sent));

	/* delta is normally under 4.2 seconds, avoid 64-bit division */
	if (likely(delta <= UINT_MAX))
		return (u32)delta / NSEC_PER_USEC;

	/* avoid overflow after 71 minutes */
	if (delta > ((u64)INT_MAX * NSEC_PER_USEC))
		return INT_MAX;

	return div_u64(delta, NSEC_PER_USEC);
}

static int
tsince(u32 tag)
{
	int n;

	n = jiffies & 0xffff;
	n -= tag & 0xffff;
	if (n < 0)
		n += 1<<16;
	return jiffies_to_usecs(n + 1);
}

static struct aoeif *
getif(struct aoetgt *t, struct net_device *nd)
{
	struct aoeif *p, *e;

	p = t->ifs;
	e = p + NAOEIFS;
	for (; p < e; p++)
		if (p->nd == nd)
			return p;
	return NULL;
}

static void
ejectif(struct aoetgt *t, struct aoeif *ifp)
{
	struct aoeif *e;
	struct net_device *nd;
	ulong n;

	nd = ifp->nd;
	e = t->ifs + NAOEIFS - 1;
	n = (e - ifp) * sizeof *ifp;
	memmove(ifp, ifp+1, n);
	e->nd = NULL;
	dev_put(nd);
}

static struct frame *
reassign_frame(struct frame *f)
{
	struct frame *nf;
	struct sk_buff *skb;

	nf = newframe(f->t->d);
	if (!nf)
		return NULL;
	if (nf->t == f->t) {
		aoe_freetframe(nf);
		return NULL;
	}

	skb = nf->skb;
	nf->skb = f->skb;
	nf->buf = f->buf;
	nf->iter = f->iter;
	nf->waited = 0;
	nf->waited_total = f->waited_total;
	nf->sent = f->sent;
	f->skb = skb;

	return nf;
}

static void
probe(struct aoetgt *t)
{
	struct aoedev *d;
	struct frame *f;
	struct sk_buff *skb;
	struct sk_buff_head queue;
	size_t n, m;
	int frag;

	d = t->d;
	f = newtframe(d, t);
	if (!f) {
		pr_err("%s %pm for e%ld.%d: %s\n",
			"aoe: cannot probe remote address",
			t->addr,
			(long) d->aoemajor, d->aoeminor,
			"no frame available");
		return;
	}
	f->flags |= FFL_PROBE;
	ifrotate(t);
	f->iter.bi_size = t->d->maxbcnt ? t->d->maxbcnt : DEFAULTBCNT;
	ata_rw_frameinit(f);
	skb = f->skb;
	for (frag = 0, n = f->iter.bi_size; n > 0; ++frag, n -= m) {
		if (n < PAGE_SIZE)
			m = n;
		else
			m = PAGE_SIZE;
		skb_fill_page_desc(skb, frag, empty_page, 0, m);
	}
	skb->len += f->iter.bi_size;
	skb->data_len = f->iter.bi_size;
	skb->truesize += f->iter.bi_size;

	skb = skb_clone(f->skb, GFP_ATOMIC);
	if (skb) {
		f->sent = ktime_get();
		__skb_queue_head_init(&queue);
		__skb_queue_tail(&queue, skb);
		aoenet_xmit(&queue);
	}
}

static long
rto(struct aoedev *d)
{
	long t;

	t = 2 * d->rttavg >> RTTSCALE;
	t += 8 * d->rttdev >> RTTDSCALE;
	if (t == 0)
		t = 1;

	return t;
}

static void
rexmit_deferred(struct aoedev *d)
{
	struct aoetgt *t;
	struct frame *f;
	struct frame *nf;
	struct list_head *pos, *nx, *head;
	int since;
	int untainted;

	count_targets(d, &untainted);

	head = &d->rexmitq;
	list_for_each_safe(pos, nx, head) {
		f = list_entry(pos, struct frame, head);
		t = f->t;
		if (t->taint) {
			if (!(f->flags & FFL_PROBE)) {
				nf = reassign_frame(f);
				if (nf) {
					if (t->nout_probes == 0
					&& untainted > 0) {
						probe(t);
						t->nout_probes++;
					}
					list_replace(&f->head, &nf->head);
					pos = &nf->head;
					aoe_freetframe(f);
					f = nf;
					t = f->t;
				}
			} else if (untainted < 1) {
				/* don't probe w/o other untainted aoetgts */
				goto stop_probe;
			} else if (tsince_hr(f) < t->taint * rto(d)) {
				/* reprobe slowly when taint is high */
				continue;
			}
		} else if (f->flags & FFL_PROBE) {
stop_probe:		/* don't probe untainted aoetgts */
			list_del(pos);
			aoe_freetframe(f);
			/* leaving d->kicked, because this is routine */
			f->t->d->flags |= DEVFL_KICKME;
			continue;
		}
		if (t->nout >= t->maxout)
			continue;
		list_del(pos);
		t->nout++;
		if (f->flags & FFL_PROBE)
			t->nout_probes++;
		since = tsince_hr(f);
		f->waited += since;
		f->waited_total += since;
		resend(d, f);
	}
}

/* An aoetgt accumulates demerits quickly, and successful
 * probing redeems the aoetgt slowly.
 */
static void
scorn(struct aoetgt *t)
{
	int n;

	n = t->taint++;
	t->taint += t->taint * 2;
	if (n > t->taint)
		t->taint = n;
	if (t->taint > MAX_TAINT)
		t->taint = MAX_TAINT;
}

static int
count_targets(struct aoedev *d, int *untainted)
{
	int i, good;

	for (i = good = 0; i < d->ntargets && d->targets[i]; ++i)
		if (d->targets[i]->taint == 0)
			good++;

	if (untainted)
		*untainted = good;
	return i;
}

static void
rexmit_timer(struct timer_list *timer)
{
	struct aoedev *d;
	struct aoetgt *t;
	struct aoeif *ifp;
	struct frame *f;
	struct list_head *head, *pos, *nx;
	LIST_HEAD(flist);
	register long timeout;
	ulong flags, n;
	int i;
	int utgts;	/* number of aoetgt descriptors (not slots) */
	int since;

	d = from_timer(d, timer, timer);

	spin_lock_irqsave(&d->lock, flags);

	/* timeout based on observed timings and variations */
	timeout = rto(d);

	utgts = count_targets(d, NULL);

	if (d->flags & DEVFL_TKILL) {
		spin_unlock_irqrestore(&d->lock, flags);
		return;
	}

	/* collect all frames to rexmit into flist */
	for (i = 0; i < NFACTIVE; i++) {
		head = &d->factive[i];
		list_for_each_safe(pos, nx, head) {
			f = list_entry(pos, struct frame, head);
			if (tsince_hr(f) < timeout)
				break;	/* end of expired frames */
			/* move to flist for later processing */
			list_move_tail(pos, &flist);
		}
	}

	/* process expired frames */
	while (!list_empty(&flist)) {
		pos = flist.next;
		f = list_entry(pos, struct frame, head);
		since = tsince_hr(f);
		n = f->waited_total + since;
		n /= USEC_PER_SEC;
		if (aoe_deadsecs
		&& n > aoe_deadsecs
		&& !(f->flags & FFL_PROBE)) {
			/* Waited too long.  Device failure.
			 * Hang all frames on first hash bucket for downdev
			 * to clean up.
			 */
			list_splice(&flist, &d->factive[0]);
			aoedev_downdev(d);
			goto out;
		}

		t = f->t;
		n = f->waited + since;
		n /= USEC_PER_SEC;
		if (aoe_deadsecs && utgts > 0
		&& (n > aoe_deadsecs / utgts || n > HARD_SCORN_SECS))
			scorn(t); /* avoid this target */

		if (t->maxout != 1) {
			t->ssthresh = t->maxout / 2;
			t->maxout = 1;
		}

		if (f->flags & FFL_PROBE) {
			t->nout_probes--;
		} else {
			ifp = getif(t, f->skb->dev);
			if (ifp && ++ifp->lost > (t->nframes << 1)
			&& (ifp != t->ifs || t->ifs[1].nd)) {
				ejectif(t, ifp);
				ifp = NULL;
			}
		}
		list_move_tail(pos, &d->rexmitq);
		t->nout--;
	}
	rexmit_deferred(d);

out:
	if ((d->flags & DEVFL_KICKME) && d->blkq) {
		d->flags &= ~DEVFL_KICKME;
		blk_mq_run_hw_queues(d->blkq, true);
	}

	d->timer.expires = jiffies + TIMERTICK;
	add_timer(&d->timer);

	spin_unlock_irqrestore(&d->lock, flags);
}

static void
bufinit(struct buf *buf, struct request *rq, struct bio *bio)
{
	memset(buf, 0, sizeof(*buf));
	buf->rq = rq;
	buf->bio = bio;
	buf->iter = bio->bi_iter;
}

static struct buf *
nextbuf(struct aoedev *d)
{
	struct request *rq;
	struct request_queue *q;
	struct aoe_req *req;
	struct buf *buf;
	struct bio *bio;

	q = d->blkq;
	if (q == NULL)
		return NULL;	/* initializing */
	if (d->ip.buf)
		return d->ip.buf;
	rq = d->ip.rq;
	if (rq == NULL) {
		rq = list_first_entry_or_null(&d->rq_list, struct request,
						queuelist);
		if (rq == NULL)
			return NULL;
		list_del_init(&rq->queuelist);
		blk_mq_start_request(rq);
		d->ip.rq = rq;
		d->ip.nxbio = rq->bio;

		req = blk_mq_rq_to_pdu(rq);
		req->nr_bios = 0;
		__rq_for_each_bio(bio, rq)
			req->nr_bios++;
	}
	buf = mempool_alloc(d->bufpool, GFP_ATOMIC);
	if (buf == NULL) {
		pr_err("aoe: nextbuf: unable to mempool_alloc!\n");
		return NULL;
	}
	bio = d->ip.nxbio;
	bufinit(buf, rq, bio);
	bio = bio->bi_next;
	d->ip.nxbio = bio;
	if (bio == NULL)
		d->ip.rq = NULL;
	return d->ip.buf = buf;
}

/* enters with d->lock held */
void
aoecmd_work(struct aoedev *d)
{
	rexmit_deferred(d);
	while (aoecmd_ata_rw(d))
		;
}

/* this function performs work that has been deferred until sleeping is OK
 */
void
aoecmd_sleepwork(struct work_struct *work)
{
	struct aoedev *d = container_of(work, struct aoedev, work);

	if (d->flags & DEVFL_GDALLOC)
		aoeblk_gdalloc(d);

	if (d->flags & DEVFL_NEWSIZE) {
		set_capacity_and_notify(d->gd, d->ssize);

		spin_lock_irq(&d->lock);
		d->flags |= DEVFL_UP;
		d->flags &= ~DEVFL_NEWSIZE;
		spin_unlock_irq(&d->lock);
	}
}

static void
ata_ident_fixstring(u16 *id, int ns)
{
	u16 s;

	while (ns-- > 0) {
		s = *id;
		*id++ = s >> 8 | s << 8;
	}
}

static void
ataid_complete(struct aoedev *d, struct aoetgt *t, unsigned char *id)
{
	u64 ssize;
	u16 n;

	/* word 83: command set supported */
	n = get_unaligned_le16(&id[83 << 1]);

	/* word 86: command set/feature enabled */
	n |= get_unaligned_le16(&id[86 << 1]);

	if (n & (1<<10)) {	/* bit 10: LBA 48 */
		d->flags |= DEVFL_EXT;

		/* word 100: number lba48 sectors */
		ssize = get_unaligned_le64(&id[100 << 1]);

		/* set as in ide-disk.c:init_idedisk_capacity */
		d->geo.cylinders = ssize;
		d->geo.cylinders /= (255 * 63);
		d->geo.heads = 255;
		d->geo.sectors = 63;
	} else {
		d->flags &= ~DEVFL_EXT;

		/* number lba28 sectors */
		ssize = get_unaligned_le32(&id[60 << 1]);

		/* NOTE: obsolete in ATA 6 */
		d->geo.cylinders = get_unaligned_le16(&id[54 << 1]);
		d->geo.heads = get_unaligned_le16(&id[55 << 1]);
		d->geo.sectors = get_unaligned_le16(&id[56 << 1]);
	}

	ata_ident_fixstring((u16 *) &id[10<<1], 10);	/* serial */
	ata_ident_fixstring((u16 *) &id[23<<1], 4);	/* firmware */
	ata_ident_fixstring((u16 *) &id[27<<1], 20);	/* model */
	memcpy(d->ident, id, sizeof(d->ident));

	if (d->ssize != ssize)
		printk(KERN_INFO
			"aoe: %pm e%ld.%d v%04x has %llu sectors\n",
			t->addr,
			d->aoemajor, d->aoeminor,
			d->fw_ver, (long long)ssize);
	d->ssize = ssize;
	d->geo.start = 0;
	if (d->flags & (DEVFL_GDALLOC|DEVFL_NEWSIZE))
		return;
	if (d->gd != NULL)
		d->flags |= DEVFL_NEWSIZE;
	else
		d->flags |= DEVFL_GDALLOC;
	queue_work(aoe_wq, &d->work);
}

static void
calc_rttavg(struct aoedev *d, struct aoetgt *t, int rtt)
{
	register long n;

	n = rtt;

	/* cf. Congestion Avoidance and Control, Jacobson & Karels, 1988 */
	n -= d->rttavg >> RTTSCALE;
	d->rttavg += n;
	if (n < 0)
		n = -n;
	n -= d->rttdev >> RTTDSCALE;
	d->rttdev += n;

	if (!t || t->maxout >= t->nframes)
		return;
	if (t->maxout < t->ssthresh)
		t->maxout += 1;
	else if (t->nout == t->maxout && t->next_cwnd-- == 0) {
		t->maxout += 1;
		t->next_cwnd = t->maxout;
	}
}

static struct aoetgt *
gettgt(struct aoedev *d, char *addr)
{
	struct aoetgt **t, **e;

	t = d->targets;
	e = t + d->ntargets;
	for (; t < e && *t; t++)
		if (memcmp((*t)->addr, addr, sizeof((*t)->addr)) == 0)
			return *t;
	return NULL;
}

static void
bvcpy(struct sk_buff *skb, struct bio *bio, struct bvec_iter iter, long cnt)
{
	int soff = 0;
	struct bio_vec bv;

	iter.bi_size = cnt;

	__bio_for_each_segment(bv, bio, iter, iter) {
		char *p = bvec_kmap_local(&bv);
		skb_copy_bits(skb, soff, p, bv.bv_len);
		kunmap_local(p);
		soff += bv.bv_len;
	}
}

void
aoe_end_request(struct aoedev *d, struct request *rq, int fastfail)
{
	struct bio *bio;
	int bok;
	struct request_queue *q;
	blk_status_t err = BLK_STS_OK;

	q = d->blkq;
	if (rq == d->ip.rq)
		d->ip.rq = NULL;
	do {
		bio = rq->bio;
		bok = !fastfail && !bio->bi_status;
		if (!bok)
			err = BLK_STS_IOERR;
	} while (blk_update_request(rq, bok ? BLK_STS_OK : BLK_STS_IOERR, bio->bi_iter.bi_size));

	__blk_mq_end_request(rq, err);

	/* cf. https://lore.kernel.org/lkml/20061031071040.GS14055@kernel.dk/ */
	if (!fastfail)
		blk_mq_run_hw_queues(q, true);
}

static void
aoe_end_buf(struct aoedev *d, struct buf *buf)
{
	struct request *rq = buf->rq;
	struct aoe_req *req = blk_mq_rq_to_pdu(rq);

	if (buf == d->ip.buf)
		d->ip.buf = NULL;
	mempool_free(buf, d->bufpool);
	if (--req->nr_bios == 0)
		aoe_end_request(d, rq, 0);
}

static void
ktiocomplete(struct frame *f)
{
	struct aoe_hdr *hin, *hout;
	struct aoe_atahdr *ahin, *ahout;
	struct buf *buf;
	struct sk_buff *skb;
	struct aoetgt *t;
	struct aoeif *ifp;
	struct aoedev *d;
	long n;
	int untainted;

	if (f == NULL)
		return;

	t = f->t;
	d = t->d;
	skb = f->r_skb;
	buf = f->buf;
	if (f->flags & FFL_PROBE)
		goto out;
	if (!skb)		/* just fail the buf. */
		goto noskb;

	hout = (struct aoe_hdr *) skb_mac_header(f->skb);
	ahout = (struct aoe_atahdr *) (hout+1);

	hin = (struct aoe_hdr *) skb->data;
	skb_pull(skb, sizeof(*hin));
	ahin = (struct aoe_atahdr *) skb->data;
	skb_pull(skb, sizeof(*ahin));
	if (ahin->cmdstat & 0xa9) {	/* these bits cleared on success */
		pr_err("aoe: ata error cmd=%2.2Xh stat=%2.2Xh from e%ld.%d\n",
			ahout->cmdstat, ahin->cmdstat,
			d->aoemajor, d->aoeminor);
noskb:		if (buf)
			buf->bio->bi_status = BLK_STS_IOERR;
		goto out;
	}

	n = ahout->scnt << 9;
	switch (ahout->cmdstat) {
	case ATA_CMD_PIO_READ:
	case ATA_CMD_PIO_READ_EXT:
		if (skb->len < n) {
			pr_err("%s e%ld.%d.  skb->len=%d need=%ld\n",
				"aoe: runt data size in read from",
				(long) d->aoemajor, d->aoeminor,
			       skb->len, n);
			buf->bio->bi_status = BLK_STS_IOERR;
			break;
		}
		if (n > f->iter.bi_size) {
			pr_err_ratelimited("%s e%ld.%d.  bytes=%ld need=%u\n",
				"aoe: too-large data size in read from",
				(long) d->aoemajor, d->aoeminor,
				n, f->iter.bi_size);
			buf->bio->bi_status = BLK_STS_IOERR;
			break;
		}
		bvcpy(skb, f->buf->bio, f->iter, n);
		fallthrough;
	case ATA_CMD_PIO_WRITE:
	case ATA_CMD_PIO_WRITE_EXT:
		spin_lock_irq(&d->lock);
		ifp = getif(t, skb->dev);
		if (ifp)
			ifp->lost = 0;
		spin_unlock_irq(&d->lock);
		break;
	case ATA_CMD_ID_ATA:
		if (skb->len < 512) {
			pr_info("%s e%ld.%d.  skb->len=%d need=512\n",
				"aoe: runt data size in ataid from",
				(long) d->aoemajor, d->aoeminor,
				skb->len);
			break;
		}
		if (skb_linearize(skb))
			break;
		spin_lock_irq(&d->lock);
		ataid_complete(d, t, skb->data);
		spin_unlock_irq(&d->lock);
		break;
	default:
		pr_info("aoe: unrecognized ata command %2.2Xh for %d.%d\n",
			ahout->cmdstat,
			be16_to_cpu(get_unaligned(&hin->major)),
			hin->minor);
	}
out:
	spin_lock_irq(&d->lock);
	if (t->taint > 0
	&& --t->taint > 0
	&& t->nout_probes == 0) {
		count_targets(d, &untainted);
		if (untainted > 0) {
			probe(t);
			t->nout_probes++;
		}
	}

	aoe_freetframe(f);

	if (buf && --buf->nframesout == 0 && buf->iter.bi_size == 0)
		aoe_end_buf(d, buf);

	spin_unlock_irq(&d->lock);
	aoedev_put(d);
	dev_kfree_skb(skb);
}

/* Enters with iocq.lock held.
 * Returns true iff responses needing processing remain.
 */
static int
ktio(int id)
{
	struct frame *f;
	struct list_head *pos;
	int i;
	int actual_id;

	for (i = 0; ; ++i) {
		if (i == MAXIOC)
			return 1;
		if (list_empty(&iocq[id].head))
			return 0;
		pos = iocq[id].head.next;
		list_del(pos);
		f = list_entry(pos, struct frame, head);
		spin_unlock_irq(&iocq[id].lock);
		ktiocomplete(f);

		/* Figure out if extra threads are required. */
		actual_id = f->t->d->aoeminor % ncpus;

		if (!kts[actual_id].active) {
			BUG_ON(id != 0);
			mutex_lock(&ktio_spawn_lock);
			if (!kts[actual_id].active
				&& aoe_ktstart(&kts[actual_id]) == 0)
				kts[actual_id].active = 1;
			mutex_unlock(&ktio_spawn_lock);
		}
		spin_lock_irq(&iocq[id].lock);
	}
}

static int
kthread(void *vp)
{
	struct ktstate *k;
	DECLARE_WAITQUEUE(wait, current);
	int more;

	k = vp;
	current->flags |= PF_NOFREEZE;
	set_user_nice(current, -10);
	complete(&k->rendez);	/* tell spawner we're running */
	do {
		spin_lock_irq(k->lock);
		more = k->fn(k->id);
		if (!more) {
			add_wait_queue(k->waitq, &wait);
			__set_current_state(TASK_INTERRUPTIBLE);
		}
		spin_unlock_irq(k->lock);
		if (!more) {
			schedule();
			remove_wait_queue(k->waitq, &wait);
		} else
			cond_resched();
	} while (!kthread_should_stop());
	complete(&k->rendez);	/* tell spawner we're stopping */
	return 0;
}

void
aoe_ktstop(struct ktstate *k)
{
	kthread_stop(k->task);
	wait_for_completion(&k->rendez);
}

int
aoe_ktstart(struct ktstate *k)
{
	struct task_struct *task;

	init_completion(&k->rendez);
	task = kthread_run(kthread, k, "%s", k->name);
	if (task == NULL || IS_ERR(task))
		return -ENOMEM;
	k->task = task;
	wait_for_completion(&k->rendez); /* allow kthread to start */
	init_completion(&k->rendez);	/* for waiting for exit later */
	return 0;
}

/* pass it off to kthreads for processing */
static void
ktcomplete(struct frame *f, struct sk_buff *skb)
{
	int id;
	ulong flags;

	f->r_skb = skb;
	id = f->t->d->aoeminor % ncpus;
	spin_lock_irqsave(&iocq[id].lock, flags);
	if (!kts[id].active) {
		spin_unlock_irqrestore(&iocq[id].lock, flags);
		/* The thread with id has not been spawned yet,
		 * so delegate the work to the main thread and
		 * try spawning a new thread.
		 */
		id = 0;
		spin_lock_irqsave(&iocq[id].lock, flags);
	}
	list_add_tail(&f->head, &iocq[id].head);
	spin_unlock_irqrestore(&iocq[id].lock, flags);
	wake_up(&ktiowq[id]);
}

struct sk_buff *
aoecmd_ata_rsp(struct sk_buff *skb)
{
	struct aoedev *d;
	struct aoe_hdr *h;
	struct frame *f;
	u32 n;
	ulong flags;
	char ebuf[128];
	u16 aoemajor;

	h = (struct aoe_hdr *) skb->data;
	aoemajor = be16_to_cpu(get_unaligned(&h->major));
	d = aoedev_by_aoeaddr(aoemajor, h->minor, 0);
	if (d == NULL) {
		snprintf(ebuf, sizeof ebuf, "aoecmd_ata_rsp: ata response "
			"for unknown device %d.%d\n",
			aoemajor, h->minor);
		aoechr_error(ebuf);
		return skb;
	}

	spin_lock_irqsave(&d->lock, flags);

	n = be32_to_cpu(get_unaligned(&h->tag));
	f = getframe(d, n);
	if (f) {
		calc_rttavg(d, f->t, tsince_hr(f));
		f->t->nout--;
		if (f->flags & FFL_PROBE)
			f->t->nout_probes--;
	} else {
		f = getframe_deferred(d, n);
		if (f) {
			calc_rttavg(d, NULL, tsince_hr(f));
		} else {
			calc_rttavg(d, NULL, tsince(n));
			spin_unlock_irqrestore(&d->lock, flags);
			aoedev_put(d);
			snprintf(ebuf, sizeof(ebuf),
				 "%15s e%d.%d    tag=%08x@%08lx s=%pm d=%pm\n",
				 "unexpected rsp",
				 get_unaligned_be16(&h->major),
				 h->minor,
				 get_unaligned_be32(&h->tag),
				 jiffies,
				 h->src,
				 h->dst);
			aoechr_error(ebuf);
			return skb;
		}
	}
	aoecmd_work(d);

	spin_unlock_irqrestore(&d->lock, flags);

	ktcomplete(f, skb);

	/*
	 * Note here that we do not perform an aoedev_put, as we are
	 * leaving this reference for the ktio to release.
	 */
	return NULL;
}

void
aoecmd_cfg(ushort aoemajor, unsigned char aoeminor)
{
	struct sk_buff_head queue;

	__skb_queue_head_init(&queue);
	aoecmd_cfg_pkts(aoemajor, aoeminor, &queue);
	aoenet_xmit(&queue);
}

struct sk_buff *
aoecmd_ata_id(struct aoedev *d)
{
	struct aoe_hdr *h;
	struct aoe_atahdr *ah;
	struct frame *f;
	struct sk_buff *skb;
	struct aoetgt *t;

	f = newframe(d);
	if (f == NULL)
		return NULL;

	t = *d->tgt;

	/* initialize the headers & frame */
	skb = f->skb;
	h = (struct aoe_hdr *) skb_mac_header(skb);
	ah = (struct aoe_atahdr *) (h+1);
	skb_put(skb, sizeof *h + sizeof *ah);
	memset(h, 0, skb->len);
	f->tag = aoehdr_atainit(d, t, h);
	fhash(f);
	t->nout++;
	f->waited = 0;
	f->waited_total = 0;

	/* set up ata header */
	ah->scnt = 1;
	ah->cmdstat = ATA_CMD_ID_ATA;
	ah->lba3 = 0xa0;

	skb->dev = t->ifp->nd;

	d->rttavg = RTTAVG_INIT;
	d->rttdev = RTTDEV_INIT;
	d->timer.function = rexmit_timer;

	skb = skb_clone(skb, GFP_ATOMIC);
	if (skb)
		f->sent = ktime_get();

	return skb;
}

static struct aoetgt **
grow_targets(struct aoedev *d)
{
	ulong oldn, newn;
	struct aoetgt **tt;

	oldn = d->ntargets;
	newn = oldn * 2;
	tt = kcalloc(newn, sizeof(*d->targets), GFP_ATOMIC);
	if (!tt)
		return NULL;
	memmove(tt, d->targets, sizeof(*d->targets) * oldn);
	d->tgt = tt + (d->tgt - d->targets);
	kfree(d->targets);
	d->targets = tt;
	d->ntargets = newn;

	return &d->targets[oldn];
}

static struct aoetgt *
addtgt(struct aoedev *d, char *addr, ulong nframes)
{
	struct aoetgt *t, **tt, **te;

	tt = d->targets;
	te = tt + d->ntargets;
	for (; tt < te && *tt; tt++)
		;

	if (tt == te) {
		tt = grow_targets(d);
		if (!tt)
			goto nomem;
	}
	t = kzalloc(sizeof(*t), GFP_ATOMIC);
	if (!t)
		goto nomem;
	t->nframes = nframes;
	t->d = d;
	memcpy(t->addr, addr, sizeof t->addr);
	t->ifp = t->ifs;
	aoecmd_wreset(t);
	t->maxout = t->nframes / 2;
	INIT_LIST_HEAD(&t->ffree);
	return *tt = t;

 nomem:
	pr_info("aoe: cannot allocate memory to add target\n");
	return NULL;
}

static void
setdbcnt(struct aoedev *d)
{
	struct aoetgt **t, **e;
	int bcnt = 0;

	t = d->targets;
	e = t + d->ntargets;
	for (; t < e && *t; t++)
		if (bcnt == 0 || bcnt > (*t)->minbcnt)
			bcnt = (*t)->minbcnt;
	if (bcnt != d->maxbcnt) {
		d->maxbcnt = bcnt;
		pr_info("aoe: e%ld.%d: setting %d byte data frames\n",
			d->aoemajor, d->aoeminor, bcnt);
	}
}

static void
setifbcnt(struct aoetgt *t, struct net_device *nd, int bcnt)
{
	struct aoedev *d;
	struct aoeif *p, *e;
	int minbcnt;

	d = t->d;
	minbcnt = bcnt;
	p = t->ifs;
	e = p + NAOEIFS;
	for (; p < e; p++) {
		if (p->nd == NULL)
			break;		/* end of the valid interfaces */
		if (p->nd == nd) {
			p->bcnt = bcnt;	/* we're updating */
			nd = NULL;
		} else if (minbcnt > p->bcnt)
			minbcnt = p->bcnt; /* find the min interface */
	}
	if (nd) {
		if (p == e) {
			pr_err("aoe: device setifbcnt failure; too many interfaces.\n");
			return;
		}
		dev_hold(nd);
		p->nd = nd;
		p->bcnt = bcnt;
	}
	t->minbcnt = minbcnt;
	setdbcnt(d);
}

void
aoecmd_cfg_rsp(struct sk_buff *skb)
{
	struct aoedev *d;
	struct aoe_hdr *h;
	struct aoe_cfghdr *ch;
	struct aoetgt *t;
	ulong flags, aoemajor;
	struct sk_buff *sl;
	struct sk_buff_head queue;
	u16 n;

	sl = NULL;
	h = (struct aoe_hdr *) skb_mac_header(skb);
	ch = (struct aoe_cfghdr *) (h+1);

	/*
	 * Enough people have their dip switches set backwards to
	 * warrant a loud message for this special case.
	 */
	aoemajor = get_unaligned_be16(&h->major);
	if (aoemajor == 0xfff) {
		printk(KERN_ERR "aoe: Warning: shelf address is all ones.  "
			"Check shelf dip switches.\n");
		return;
	}
	if (aoemajor == 0xffff) {
		pr_info("aoe: e%ld.%d: broadcast shelf number invalid\n",
			aoemajor, (int) h->minor);
		return;
	}
	if (h->minor == 0xff) {
		pr_info("aoe: e%ld.%d: broadcast slot number invalid\n",
			aoemajor, (int) h->minor);
		return;
	}

	n = be16_to_cpu(ch->bufcnt);
	if (n > aoe_maxout)	/* keep it reasonable */
		n = aoe_maxout;

	d = aoedev_by_aoeaddr(aoemajor, h->minor, 1);
	if (d == NULL) {
		pr_info("aoe: device allocation failure\n");
		return;
	}

	spin_lock_irqsave(&d->lock, flags);

	t = gettgt(d, h->src);
	if (t) {
		t->nframes = n;
		if (n < t->maxout)
			aoecmd_wreset(t);
	} else {
		t = addtgt(d, h->src, n);
		if (!t)
			goto bail;
	}
	n = skb->dev->mtu;
	n -= sizeof(struct aoe_hdr) + sizeof(struct aoe_atahdr);
	n /= 512;
	if (n > ch->scnt)
		n = ch->scnt;
	n = n ? n * 512 : DEFAULTBCNT;
	setifbcnt(t, skb->dev, n);

	/* don't change users' perspective */
	if (d->nopen == 0) {
		d->fw_ver = be16_to_cpu(ch->fwver);
		sl = aoecmd_ata_id(d);
	}
bail:
	spin_unlock_irqrestore(&d->lock, flags);
	aoedev_put(d);
	if (sl) {
		__skb_queue_head_init(&queue);
		__skb_queue_tail(&queue, sl);
		aoenet_xmit(&queue);
	}
}

void
aoecmd_wreset(struct aoetgt *t)
{
	t->maxout = 1;
	t->ssthresh = t->nframes / 2;
	t->next_cwnd = t->nframes;
}

void
aoecmd_cleanslate(struct aoedev *d)
{
	struct aoetgt **t, **te;

	d->rttavg = RTTAVG_INIT;
	d->rttdev = RTTDEV_INIT;
	d->maxbcnt = 0;

	t = d->targets;
	te = t + d->ntargets;
	for (; t < te && *t; t++)
		aoecmd_wreset(*t);
}

void
aoe_failbuf(struct aoedev *d, struct buf *buf)
{
	if (buf == NULL)
		return;
	buf->iter.bi_size = 0;
	buf->bio->bi_status = BLK_STS_IOERR;
	if (buf->nframesout == 0)
		aoe_end_buf(d, buf);
}

void
aoe_flush_iocq(void)
{
	int i;

	for (i = 0; i < ncpus; i++) {
		if (kts[i].active)
			aoe_flush_iocq_by_index(i);
	}
}

void
aoe_flush_iocq_by_index(int id)
{
	struct frame *f;
	struct aoedev *d;
	LIST_HEAD(flist);
	struct list_head *pos;
	struct sk_buff *skb;
	ulong flags;

	spin_lock_irqsave(&iocq[id].lock, flags);
	list_splice_init(&iocq[id].head, &flist);
	spin_unlock_irqrestore(&iocq[id].lock, flags);
	while (!list_empty(&flist)) {
		pos = flist.next;
		list_del(pos);
		f = list_entry(pos, struct frame, head);
		d = f->t->d;
		skb = f->r_skb;
		spin_lock_irqsave(&d->lock, flags);
		if (f->buf) {
			f->buf->nframesout--;
			aoe_failbuf(d, f->buf);
		}
		aoe_freetframe(f);
		spin_unlock_irqrestore(&d->lock, flags);
		dev_kfree_skb(skb);
		aoedev_put(d);
	}
}

int __init
aoecmd_init(void)
{
	void *p;
	int i;
	int ret;

	/* get_zeroed_page returns page with ref count 1 */
	p = (void *) get_zeroed_page(GFP_KERNEL);
	if (!p)
		return -ENOMEM;
	empty_page = virt_to_page(p);

	ncpus = num_online_cpus();

	iocq = kcalloc(ncpus, sizeof(struct iocq_ktio), GFP_KERNEL);
	if (!iocq)
		return -ENOMEM;

	kts = kcalloc(ncpus, sizeof(struct ktstate), GFP_KERNEL);
	if (!kts) {
		ret = -ENOMEM;
		goto kts_fail;
	}

	ktiowq = kcalloc(ncpus, sizeof(wait_queue_head_t), GFP_KERNEL);
	if (!ktiowq) {
		ret = -ENOMEM;
		goto ktiowq_fail;
	}

	for (i = 0; i < ncpus; i++) {
		INIT_LIST_HEAD(&iocq[i].head);
		spin_lock_init(&iocq[i].lock);
		init_waitqueue_head(&ktiowq[i]);
		snprintf(kts[i].name, sizeof(kts[i].name), "aoe_ktio%d", i);
		kts[i].fn = ktio;
		kts[i].waitq = &ktiowq[i];
		kts[i].lock = &iocq[i].lock;
		kts[i].id = i;
		kts[i].active = 0;
	}
	kts[0].active = 1;
	if (aoe_ktstart(&kts[0])) {
		ret = -ENOMEM;
		goto ktstart_fail;
	}
	return 0;

ktstart_fail:
	kfree(ktiowq);
ktiowq_fail:
	kfree(kts);
kts_fail:
	kfree(iocq);

	return ret;
}

void
aoecmd_exit(void)
{
	int i;

	for (i = 0; i < ncpus; i++)
		if (kts[i].active)
			aoe_ktstop(&kts[i]);

	aoe_flush_iocq();

	/* Free up the iocq and thread speicific configuration
	* allocated during startup.
	*/
	kfree(iocq);
	kfree(kts);
	kfree(ktiowq);

	free_page((unsigned long) page_address(empty_page));
	empty_page = NULL;
}