{
	pasid_limit = max_num_of_processes;

	pasid_bitmap = kcalloc(BITS_TO_LONGS(pasid_limit), sizeof(long), GFP_KERNEL);
	if (!pasid_bitmap)
		return -ENOMEM;

	set_bit(0, pasid_bitmap); /* PASID 0 is reserved. */