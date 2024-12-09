
static void blk_mq_sysfs_release(struct kobject *kobj)
{
	struct request_queue *q;

	q = container_of(kobj, struct request_queue, mq_kobj);
	free_percpu(q->queue_ctx);
}

static void blk_mq_ctx_release(struct kobject *kobj)
{
	struct blk_mq_ctx *ctx;

	ctx = container_of(kobj, struct blk_mq_ctx, kobj);
	kobject_put(&ctx->queue->mq_kobj);
}

static void blk_mq_hctx_release(struct kobject *kobj)
{
	struct blk_mq_hw_ctx *hctx;

	hctx = container_of(kobj, struct blk_mq_hw_ctx, kobj);
	kfree(hctx);
}

struct blk_mq_ctx_sysfs_entry {
	struct attribute attr;
static struct kobj_type blk_mq_ctx_ktype = {
	.sysfs_ops	= &blk_mq_sysfs_ops,
	.default_attrs	= default_ctx_attrs,
	.release	= blk_mq_ctx_release,
};

static struct kobj_type blk_mq_hw_ktype = {
	.sysfs_ops	= &blk_mq_hw_sysfs_ops,
	.default_attrs	= default_hw_ctx_attrs,
	.release	= blk_mq_hctx_release,
};

static void blk_mq_unregister_hctx(struct blk_mq_hw_ctx *hctx)
{
		return ret;

	hctx_for_each_ctx(hctx, ctx, i) {
		kobject_get(&q->mq_kobj);
		ret = kobject_add(&ctx->kobj, &hctx->kobj, "cpu%u", ctx->cpu);
		if (ret)
			break;
	}