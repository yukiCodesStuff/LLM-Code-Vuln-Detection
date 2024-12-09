#include <linux/sched/loadavg.h>
#include <linux/sched/signal.h>
#include <trace/events/block.h>
#include <linux/blk-mq.h>
#include "blk-rq-qos.h"
#include "blk-stat.h"

#define DEFAULT_SCALE_COOKIE 1000000U
	u64 now = ktime_to_ns(ktime_get());
	bool issue_as_root = bio_issue_as_root_blkg(bio);
	bool enabled = false;
	int inflight = 0;

	blkg = bio->bi_blkg;
	if (!blkg || !bio_flagged(bio, BIO_TRACKED))
		return;
		return;

	enabled = blk_iolatency_enabled(iolat->blkiolat);
	if (!enabled)
		return;

	while (blkg && blkg->parent) {
		iolat = blkg_to_lat(blkg);
		if (!iolat) {
			blkg = blkg->parent;
		}
		rqw = &iolat->rq_wait;

		inflight = atomic_dec_return(&rqw->inflight);
		WARN_ON_ONCE(inflight < 0);
		if (iolat->min_lat_nsec == 0)
			goto next;
		iolatency_record_time(iolat, &bio->bi_issue, now,
				      issue_as_root);
		window_start = atomic64_read(&iolat->window_start);
	return 0;
}

/*
 * return 1 for enabling iolatency, return -1 for disabling iolatency, otherwise
 * return 0.
 */
static int iolatency_set_min_lat_nsec(struct blkcg_gq *blkg, u64 val)
{
	struct iolatency_grp *iolat = blkg_to_lat(blkg);
	u64 oldval = iolat->min_lat_nsec;

	iolat->min_lat_nsec = val;
	iolat->cur_win_nsec = max_t(u64, val << 4, BLKIOLATENCY_MIN_WIN_SIZE);
				    BLKIOLATENCY_MAX_WIN_SIZE);

	if (!oldval && val)
		return 1;
	if (oldval && !val)
		return -1;
	return 0;
}

static void iolatency_clear_scaling(struct blkcg_gq *blkg)
{
	u64 lat_val = 0;
	u64 oldval;
	int ret;
	int enable = 0;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_iolatency, buf, &ctx);
	if (ret)
		return ret;
	blkg = ctx.blkg;
	oldval = iolat->min_lat_nsec;

	enable = iolatency_set_min_lat_nsec(blkg, lat_val);
	if (enable) {
		WARN_ON_ONCE(!blk_get_queue(blkg->q));
		blkg_get(blkg);
	}

	if (oldval != iolat->min_lat_nsec) {
		iolatency_clear_scaling(blkg);
	}

	ret = 0;
out:
	blkg_conf_finish(&ctx);
	if (ret == 0 && enable) {
		struct iolatency_grp *tmp = blkg_to_lat(blkg);
		struct blk_iolatency *blkiolat = tmp->blkiolat;

		blk_mq_freeze_queue(blkg->q);

		if (enable == 1)
			atomic_inc(&blkiolat->enabled);
		else if (enable == -1)
			atomic_dec(&blkiolat->enabled);
		else
			WARN_ON_ONCE(1);

		blk_mq_unfreeze_queue(blkg->q);

		blkg_put(blkg);
		blk_put_queue(blkg->q);
	}
	return ret ?: nbytes;
}

static u64 iolatency_prfill_limit(struct seq_file *sf,
{
	struct iolatency_grp *iolat = pd_to_lat(pd);
	struct blkcg_gq *blkg = lat_to_blkg(iolat);
	struct blk_iolatency *blkiolat = iolat->blkiolat;
	int ret;

	ret = iolatency_set_min_lat_nsec(blkg, 0);
	if (ret == 1)
		atomic_inc(&blkiolat->enabled);
	if (ret == -1)
		atomic_dec(&blkiolat->enabled);
	iolatency_clear_scaling(blkg);
}

static void iolatency_pd_free(struct blkg_policy_data *pd)