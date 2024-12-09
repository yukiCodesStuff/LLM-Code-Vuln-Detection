		struct ecryptfs_cache_info *info;

		info = &ecryptfs_cache_infos[i];
		kmem_cache_destroy(*(info->cache));
	}
}

/**