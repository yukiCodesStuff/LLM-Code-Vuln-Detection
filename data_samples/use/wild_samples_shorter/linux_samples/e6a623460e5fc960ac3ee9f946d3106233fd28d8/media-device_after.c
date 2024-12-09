	struct media_entity *ent;
	struct media_entity_desc u_ent;

	memset(&u_ent, 0, sizeof(u_ent));
	if (copy_from_user(&u_ent.id, &uent->id, sizeof(u_ent.id)))
		return -EFAULT;

	ent = find_entity(mdev, u_ent.id);