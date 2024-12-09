{
	pasid_limit = max_num_of_processes;

	pasid_bitmap = kcalloc(BITS_TO_LONGS(pasid_limit), sizeof(long), GFP_KERNEL);
	if (!pasid_bitmap)
		return -ENOMEM;

	set_bit(0, pasid_bitmap); /* PASID 0 is reserved. */

	return 0;
}

void kfd_pasid_exit(void)
{
	kfree(pasid_bitmap);
}

bool kfd_set_pasid_limit(unsigned int new_limit)
{
	if (new_limit < pasid_limit) {
		bool ok;

		mutex_lock(&pasid_mutex);

		/* ensure that no pasids >= new_limit are in-use */
		ok = (find_next_bit(pasid_bitmap, pasid_limit, new_limit) ==
								pasid_limit);
		if (ok)
			pasid_limit = new_limit;

		mutex_unlock(&pasid_mutex);

		return ok;
	}