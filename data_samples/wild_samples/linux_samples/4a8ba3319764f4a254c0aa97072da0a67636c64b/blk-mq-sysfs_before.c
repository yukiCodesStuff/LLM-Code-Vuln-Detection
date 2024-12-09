#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/backing-dev.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/smp.h>

#include <linux/blk-mq.h>
#include "blk-mq.h"
#include "blk-mq-tag.h"

static void blk_mq_sysfs_release(struct kobject *kobj)
{
}
static struct kobj_type blk_mq_ctx_ktype = {
	.sysfs_ops	= &blk_mq_sysfs_ops,
	.default_attrs	= default_ctx_attrs,
	.release	= blk_mq_sysfs_release,
};

static struct kobj_type blk_mq_hw_ktype = {
	.sysfs_ops	= &blk_mq_hw_sysfs_ops,
	.default_attrs	= default_hw_ctx_attrs,
	.release	= blk_mq_sysfs_release,
};

static void blk_mq_unregister_hctx(struct blk_mq_hw_ctx *hctx)
{
	struct blk_mq_ctx *ctx;
	int i;

	if (!hctx->nr_ctx || !(hctx->flags & BLK_MQ_F_SYSFS_UP))
		return;

	hctx_for_each_ctx(hctx, ctx, i)
		kobject_del(&ctx->kobj);

	kobject_del(&hctx->kobj);
}

static int blk_mq_register_hctx(struct blk_mq_hw_ctx *hctx)
{
	struct request_queue *q = hctx->queue;
	struct blk_mq_ctx *ctx;
	int i, ret;

	if (!hctx->nr_ctx || !(hctx->flags & BLK_MQ_F_SYSFS_UP))
		return 0;

	ret = kobject_add(&hctx->kobj, &q->mq_kobj, "%u", hctx->queue_num);
	if (ret)
		return ret;

	hctx_for_each_ctx(hctx, ctx, i) {
		ret = kobject_add(&ctx->kobj, &hctx->kobj, "cpu%u", ctx->cpu);
		if (ret)
			break;
	}

	return ret;
}

void blk_mq_unregister_disk(struct gendisk *disk)
{
	struct request_queue *q = disk->queue;
	struct blk_mq_hw_ctx *hctx;
	struct blk_mq_ctx *ctx;
	int i, j;

	queue_for_each_hw_ctx(q, hctx, i) {
		blk_mq_unregister_hctx(hctx);

		hctx_for_each_ctx(hctx, ctx, j)
			kobject_put(&ctx->kobj);

		kobject_put(&hctx->kobj);
	}

	kobject_uevent(&q->mq_kobj, KOBJ_REMOVE);
	kobject_del(&q->mq_kobj);
	kobject_put(&q->mq_kobj);

	kobject_put(&disk_to_dev(disk)->kobj);
}

static void blk_mq_sysfs_init(struct request_queue *q)
{
	struct blk_mq_hw_ctx *hctx;
	struct blk_mq_ctx *ctx;
	int i;

	kobject_init(&q->mq_kobj, &blk_mq_ktype);

	queue_for_each_hw_ctx(q, hctx, i)
		kobject_init(&hctx->kobj, &blk_mq_hw_ktype);

	queue_for_each_ctx(q, ctx, i)
		kobject_init(&ctx->kobj, &blk_mq_ctx_ktype);
}

/* see blk_register_queue() */
void blk_mq_finish_init(struct request_queue *q)
{
	percpu_ref_switch_to_percpu(&q->mq_usage_counter);
}

int blk_mq_register_disk(struct gendisk *disk)
{
	struct device *dev = disk_to_dev(disk);
	struct request_queue *q = disk->queue;
	struct blk_mq_hw_ctx *hctx;
	int ret, i;

	blk_mq_sysfs_init(q);

	ret = kobject_add(&q->mq_kobj, kobject_get(&dev->kobj), "%s", "mq");
	if (ret < 0)
		return ret;

	kobject_uevent(&q->mq_kobj, KOBJ_ADD);

	queue_for_each_hw_ctx(q, hctx, i) {
		hctx->flags |= BLK_MQ_F_SYSFS_UP;
		ret = blk_mq_register_hctx(hctx);
		if (ret)
			break;
	}

	if (ret) {
		blk_mq_unregister_disk(disk);
		return ret;
	}

	return 0;
}

void blk_mq_sysfs_unregister(struct request_queue *q)
{
	struct blk_mq_hw_ctx *hctx;
	int i;

	queue_for_each_hw_ctx(q, hctx, i)
		blk_mq_unregister_hctx(hctx);
}

int blk_mq_sysfs_register(struct request_queue *q)
{
	struct blk_mq_hw_ctx *hctx;
	int i, ret = 0;

	queue_for_each_hw_ctx(q, hctx, i) {
		ret = blk_mq_register_hctx(hctx);
		if (ret)
			break;
	}

	return ret;
}
{
	struct request_queue *q = hctx->queue;
	struct blk_mq_ctx *ctx;
	int i, ret;

	if (!hctx->nr_ctx || !(hctx->flags & BLK_MQ_F_SYSFS_UP))
		return 0;

	ret = kobject_add(&hctx->kobj, &q->mq_kobj, "%u", hctx->queue_num);
	if (ret)
		return ret;

	hctx_for_each_ctx(hctx, ctx, i) {
		ret = kobject_add(&ctx->kobj, &hctx->kobj, "cpu%u", ctx->cpu);
		if (ret)
			break;
	}