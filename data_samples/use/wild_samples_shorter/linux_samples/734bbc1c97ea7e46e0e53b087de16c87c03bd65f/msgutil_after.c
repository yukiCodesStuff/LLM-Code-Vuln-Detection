#define DATALEN_MSG	((size_t)PAGE_SIZE-sizeof(struct msg_msg))
#define DATALEN_SEG	((size_t)PAGE_SIZE-sizeof(struct msg_msgseg))

static kmem_buckets *msg_buckets __ro_after_init;

static int __init init_msg_buckets(void)
{
	msg_buckets = kmem_buckets_create("msg_msg", SLAB_ACCOUNT,
					  sizeof(struct msg_msg),
					  DATALEN_MSG, NULL);

	return 0;
}
subsys_initcall(init_msg_buckets);

static struct msg_msg *alloc_msg(size_t len)
{
	struct msg_msg *msg;
	size_t alen;

	alen = min(len, DATALEN_MSG);
	msg = kmem_buckets_alloc(msg_buckets, sizeof(*msg) + alen, GFP_KERNEL);
	if (msg == NULL)
		return NULL;

	msg->next = NULL;