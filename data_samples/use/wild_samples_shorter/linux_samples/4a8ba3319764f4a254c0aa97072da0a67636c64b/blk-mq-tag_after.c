}

/*
 * Wakeup all potentially sleeping on tags
 */
void blk_mq_tag_wakeup_all(struct blk_mq_tags *tags, bool include_reserve)
{
	struct blk_mq_bitmap_tags *bt;
	int i, wake_index;


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

	atomic_dec(&tags->active_queues);

	blk_mq_tag_wakeup_all(tags, false);
}

/*
 * For shared tag users, we track the number of currently active users
	 * static and should never need resizing.
	 */
	bt_update_count(&tags->bitmap_tags, tdepth);
	blk_mq_tag_wakeup_all(tags, false);
	return 0;
}

/**