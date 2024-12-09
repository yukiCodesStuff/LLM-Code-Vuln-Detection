/* External tools reserve first few grant table entries. */
#define NR_RESERVED_ENTRIES 8
#define GNTTAB_LIST_END 0xffffffff

static grant_ref_t **gnttab_list;
static unsigned int nr_grant_frames;
static unsigned int boot_max_nr_grant_frames;
static grant_status_t *grstatus;

static int grant_table_version;
static int grefs_per_grant_frame;

static struct gnttab_free_callback *gnttab_free_callback_list;

static int gnttab_expand(unsigned int req_entries);
	unsigned int new_nr_grant_frames, extra_entries, i;
	unsigned int nr_glist_frames, new_nr_glist_frames;

	BUG_ON(grefs_per_grant_frame == 0);

	new_nr_grant_frames = nr_grant_frames + more_frames;
	extra_entries       = more_frames * grefs_per_grant_frame;

	nr_glist_frames = (nr_grant_frames * grefs_per_grant_frame + RPP - 1) / RPP;
	new_nr_glist_frames =
		(new_nr_grant_frames * grefs_per_grant_frame + RPP - 1) / RPP;
	for (i = nr_glist_frames; i < new_nr_glist_frames; i++) {
		gnttab_list[i] = (grant_ref_t *)__get_free_page(GFP_ATOMIC);
		if (!gnttab_list[i])
			goto grow_nomem;
	}


	for (i = grefs_per_grant_frame * nr_grant_frames;
	     i < grefs_per_grant_frame * new_nr_grant_frames - 1; i++)
		gnttab_entry(i) = i + 1;

	gnttab_entry(i) = gnttab_free_head;
	gnttab_free_head = grefs_per_grant_frame * nr_grant_frames;
	gnttab_free_count += extra_entries;

	nr_grant_frames = new_nr_grant_frames;


static unsigned nr_status_frames(unsigned nr_grant_frames)
{
	BUG_ON(grefs_per_grant_frame == 0);
	return (nr_grant_frames * grefs_per_grant_frame + SPP - 1) / SPP;
}

static int gnttab_map_frames_v1(xen_pfn_t *frames, unsigned int nr_gframes)
{
	rc = HYPERVISOR_grant_table_op(GNTTABOP_set_version, &gsv, 1);
	if (rc == 0 && gsv.version == 2) {
		grant_table_version = 2;
		grefs_per_grant_frame = PAGE_SIZE / sizeof(union grant_entry_v2);
		gnttab_interface = &gnttab_v2_ops;
	} else if (grant_table_version == 2) {
		/*
		 * If we've already used version 2 features,
		panic("we need grant tables version 2, but only version 1 is available");
	} else {
		grant_table_version = 1;
		grefs_per_grant_frame = PAGE_SIZE / sizeof(struct grant_entry_v1);
		gnttab_interface = &gnttab_v1_ops;
	}
	printk(KERN_INFO "Grant tables using version %d layout.\n",
		grant_table_version);
}

static int gnttab_setup(void)
{
	unsigned int max_nr_gframes;

	max_nr_gframes = gnttab_max_grant_frames();
	if (max_nr_gframes < nr_grant_frames)
		return -ENOSYS;

	return 0;
}

int gnttab_resume(void)
{
	gnttab_request_version();
	return gnttab_setup();
}

int gnttab_suspend(void)
{
	gnttab_interface->unmap_frames();
	return 0;
	int rc;
	unsigned int cur, extra;

	BUG_ON(grefs_per_grant_frame == 0);
	cur = nr_grant_frames;
	extra = ((req_entries + (grefs_per_grant_frame-1)) /
		 grefs_per_grant_frame);
	if (cur + extra > gnttab_max_grant_frames())
		return -ENOSPC;

	rc = gnttab_map(cur, cur + extra - 1);
	unsigned int nr_init_grefs;
	int ret;

	gnttab_request_version();
	nr_grant_frames = 1;
	boot_max_nr_grant_frames = __max_nr_grant_frames();

	/* Determine the maximum number of frames required for the
	 * grant reference free list on the current hypervisor.
	 */
	BUG_ON(grefs_per_grant_frame == 0);
	max_nr_glist_frames = (boot_max_nr_grant_frames *
			       grefs_per_grant_frame / RPP);

	gnttab_list = kmalloc(max_nr_glist_frames * sizeof(grant_ref_t *),
			      GFP_KERNEL);
	if (gnttab_list == NULL)
		return -ENOMEM;

	nr_glist_frames = (nr_grant_frames * grefs_per_grant_frame + RPP - 1) / RPP;
	for (i = 0; i < nr_glist_frames; i++) {
		gnttab_list[i] = (grant_ref_t *)__get_free_page(GFP_KERNEL);
		if (gnttab_list[i] == NULL) {
			ret = -ENOMEM;
		}
	}

	if (gnttab_setup() < 0) {
		ret = -ENODEV;
		goto ini_nomem;
	}

	nr_init_grefs = nr_grant_frames * grefs_per_grant_frame;

	for (i = NR_RESERVED_ENTRIES; i < nr_init_grefs - 1; i++)
		gnttab_entry(i) = i + 1;
