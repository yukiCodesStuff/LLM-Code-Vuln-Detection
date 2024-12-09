			uint32_t queue_id)
{

	return kfd2kgd->hqd_is_occupied(mm->dev->kgd, queue_address,
					pipe_id, queue_id);

}
