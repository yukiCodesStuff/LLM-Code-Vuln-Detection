extern ssize_t blk_mq_tag_sysfs_show(struct blk_mq_tags *tags, char *page);
extern void blk_mq_tag_init_last_tag(struct blk_mq_tags *tags, unsigned int *last_tag);
extern int blk_mq_tag_update_depth(struct blk_mq_tags *tags, unsigned int depth);
extern void blk_mq_tag_wakeup_all(struct blk_mq_tags *tags, bool);

enum {
	BLK_MQ_TAG_CACHE_MIN	= 1,
	BLK_MQ_TAG_CACHE_MAX	= 64,