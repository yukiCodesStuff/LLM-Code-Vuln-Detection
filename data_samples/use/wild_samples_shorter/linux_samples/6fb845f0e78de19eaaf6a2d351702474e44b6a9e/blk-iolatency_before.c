#include <linux/sched/loadavg.h>
#include <linux/sched/signal.h>
#include <trace/events/block.h>
#include "blk-rq-qos.h"
#include "blk-stat.h"

#define DEFAULT_SCALE_COOKIE 1000000U
	u64 now = ktime_to_ns(ktime_get());
	bool issue_as_root = bio_issue_as_root_blkg(bio);
	bool enabled = false;

	blkg = bio->bi_blkg;
	if (!blkg || !bio_flagged(bio, BIO_TRACKED))
		return;
		return;

	enabled = blk_iolatency_enabled(iolat->blkiolat);
	while (blkg && blkg->parent) {
		iolat = blkg_to_lat(blkg);
		if (!iolat) {
			blkg = blkg->parent;
		}
		rqw = &iolat->rq_wait;

		atomic_dec(&rqw->inflight);
		if (!enabled || iolat->min_lat_nsec == 0)
			goto next;
		iolatency_record_time(iolat, &bio->bi_issue, now,
				      issue_as_root);
		window_start = atomic64_read(&iolat->window_start);
	return 0;
}

static void iolatency_set_min_lat_nsec(struct blkcg_gq *blkg, u64 val)
{
	struct iolatency_grp *iolat = blkg_to_lat(blkg);
	struct blk_iolatency *blkiolat = iolat->blkiolat;
	u64 oldval = iolat->min_lat_nsec;

	iolat->min_lat_nsec = val;
	iolat->cur_win_nsec = max_t(u64, val << 4, BLKIOLATENCY_MIN_WIN_SIZE);
				    BLKIOLATENCY_MAX_WIN_SIZE);

	if (!oldval && val)
		atomic_inc(&blkiolat->enabled);
	if (oldval && !val)
		atomic_dec(&blkiolat->enabled);
}

static void iolatency_clear_scaling(struct blkcg_gq *blkg)
{
	u64 lat_val = 0;
	u64 oldval;
	int ret;

	ret = blkg_conf_prep(blkcg, &blkcg_policy_iolatency, buf, &ctx);
	if (ret)
		return ret;
	blkg = ctx.blkg;
	oldval = iolat->min_lat_nsec;

	iolatency_set_min_lat_nsec(blkg, lat_val);
	if (oldval != iolat->min_lat_nsec) {
		iolatency_clear_scaling(blkg);
	}

	ret = 0;
out:
	blkg_conf_finish(&ctx);
	return ret ?: nbytes;
}

static u64 iolatency_prfill_limit(struct seq_file *sf,
{
	struct iolatency_grp *iolat = pd_to_lat(pd);
	struct blkcg_gq *blkg = lat_to_blkg(iolat);

	iolatency_set_min_lat_nsec(blkg, 0);
	iolatency_clear_scaling(blkg);
}

static void iolatency_pd_free(struct blkg_policy_data *pd)