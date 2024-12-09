{
	if (!test_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state) &&
	    !test_and_set_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state))
		atomic_inc(&hctx->tags->active_queues);

	return true;
}

/*
 * Wakeup all potentially sleeping on tags
 */
void blk_mq_tag_wakeup_all(struct blk_mq_tags *tags, bool include_reserve)
{
	struct blk_mq_bitmap_tags *bt;
	int i, wake_index;

	bt = &tags->bitmap_tags;
	wake_index = atomic_read(&bt->wake_index);
	for (i = 0; i < BT_WAIT_QUEUES; i++) {
		struct bt_wait_state *bs = &bt->bs[wake_index];

		if (waitqueue_active(&bs->wait))
			wake_up(&bs->wait);

		wake_index = bt_index_inc(wake_index);
	}

	if (include_reserve) {
		bt = &tags->breserved_tags;
		if (waitqueue_active(&bt->bs[0].wait))
			wake_up(&bt->bs[0].wait);
	}
}
	for (i = 0; i < BT_WAIT_QUEUES; i++) {
		struct bt_wait_state *bs = &bt->bs[wake_index];

		if (waitqueue_active(&bs->wait))
			wake_up(&bs->wait);

		wake_index = bt_index_inc(wake_index);
	}

	if (include_reserve) {
		bt = &tags->breserved_tags;
		if (waitqueue_active(&bt->bs[0].wait))
			wake_up(&bt->bs[0].wait);
	}
}

/*
 * If a previously busy queue goes inactive, potential waiters could now
 * be allowed to queue. Wake them up and check.
 */
void __blk_mq_tag_idle(struct blk_mq_hw_ctx *hctx)
{
	struct blk_mq_tags *tags = hctx->tags;

	if (!test_and_clear_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state))
		return;

	atomic_dec(&tags->active_queues);

	blk_mq_tag_wakeup_all(tags, false);
}

/*
 * For shared tag users, we track the number of currently active users
 * and attempt to provide a fair share of the tag depth for each of them.
 */
static inline bool hctx_may_queue(struct blk_mq_hw_ctx *hctx,
				  struct blk_mq_bitmap_tags *bt)
{
	unsigned int depth, users;

	if (!hctx || !(hctx->flags & BLK_MQ_F_TAG_SHARED))
		return true;
	if (!test_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state))
		return true;

	/*
	 * Don't try dividing an ant
	 */
	if (bt->depth == 1)
		return true;

	users = atomic_read(&hctx->tags->active_queues);
	if (!users)
		return true;

	/*
	 * Allow at least some tags
	 */
	depth = max((bt->depth + users - 1) / users, 4U);
	return atomic_read(&hctx->nr_active) < depth;
}

static int __bt_get_word(struct blk_align_bitmap *bm, unsigned int last_tag)
{
	int tag, org_last_tag, end;
	bool wrap = last_tag != 0;

	org_last_tag = last_tag;
	end = bm->depth;
	do {
restart:
		tag = find_next_zero_bit(&bm->word, end, last_tag);
		if (unlikely(tag >= end)) {
			/*
			 * We started with an offset, start from 0 to
			 * exhaust the map.
			 */
			if (wrap) {
				wrap = false;
				end = org_last_tag;
				last_tag = 0;
				goto restart;
			}
			return -1;
		}
{
	struct blk_mq_tags *tags = hctx->tags;

	if (!test_and_clear_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state))
		return;

	atomic_dec(&tags->active_queues);

	blk_mq_tag_wakeup_all(tags, false);
}

/*
 * For shared tag users, we track the number of currently active users
 * and attempt to provide a fair share of the tag depth for each of them.
 */
static inline bool hctx_may_queue(struct blk_mq_hw_ctx *hctx,
				  struct blk_mq_bitmap_tags *bt)
{
	unsigned int depth, users;

	if (!hctx || !(hctx->flags & BLK_MQ_F_TAG_SHARED))
		return true;
	if (!test_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state))
		return true;

	/*
	 * Don't try dividing an ant
	 */
	if (bt->depth == 1)
		return true;

	users = atomic_read(&hctx->tags->active_queues);
	if (!users)
		return true;

	/*
	 * Allow at least some tags
	 */
	depth = max((bt->depth + users - 1) / users, 4U);
	return atomic_read(&hctx->nr_active) < depth;
}

static int __bt_get_word(struct blk_align_bitmap *bm, unsigned int last_tag)
{
	int tag, org_last_tag, end;
	bool wrap = last_tag != 0;

	org_last_tag = last_tag;
	end = bm->depth;
	do {
restart:
		tag = find_next_zero_bit(&bm->word, end, last_tag);
		if (unlikely(tag >= end)) {
			/*
			 * We started with an offset, start from 0 to
			 * exhaust the map.
			 */
			if (wrap) {
				wrap = false;
				end = org_last_tag;
				last_tag = 0;
				goto restart;
			}
			return -1;
		}
		last_tag = tag + 1;
	} while (test_and_set_bit(tag, &bm->word));

	return tag;
}
{
	tdepth -= tags->nr_reserved_tags;
	if (tdepth > tags->nr_tags)
		return -EINVAL;

	/*
	 * Don't need (or can't) update reserved tags here, they remain
	 * static and should never need resizing.
	 */
	bt_update_count(&tags->bitmap_tags, tdepth);
	blk_mq_tag_wakeup_all(tags, false);
	return 0;
}

/**
 * blk_mq_unique_tag() - return a tag that is unique queue-wide
 * @rq: request for which to compute a unique tag
 *
 * The tag field in struct request is unique per hardware queue but not over
 * all hardware queues. Hence this function that returns a tag with the
 * hardware context index in the upper bits and the per hardware queue tag in
 * the lower bits.
 *
 * Note: When called for a request that is queued on a non-multiqueue request
 * queue, the hardware context index is set to zero.
 */
u32 blk_mq_unique_tag(struct request *rq)
{
	struct request_queue *q = rq->q;
	struct blk_mq_hw_ctx *hctx;
	int hwq = 0;

	if (q->mq_ops) {
		hctx = q->mq_ops->map_queue(q, rq->mq_ctx->cpu);
		hwq = hctx->queue_num;
	}