		.extent_locked = 0,
		.sync_io = wbc->sync_mode == WB_SYNC_ALL,
	};
	struct btrfs_fs_info *fs_info = BTRFS_I(mapping->host)->root->fs_info;
	int ret = 0;
	int done = 0;
	int nr_to_write_done = 0;
	struct pagevec pvec;
		end_write_bio(&epd, ret);
		return ret;
	}
	/*
	 * If something went wrong, don't allow any metadata write bio to be
	 * submitted.
	 *
	 * This would prevent use-after-free if we had dirty pages not
	 * cleaned up, which can still happen by fuzzed images.
	 *
	 * - Bad extent tree
	 *   Allowing existing tree block to be allocated for other trees.
	 *
	 * - Log tree operations
	 *   Exiting tree blocks get allocated to log tree, bumps its
	 *   generation, then get cleaned in tree re-balance.
	 *   Such tree block will not be written back, since it's clean,
	 *   thus no WRITTEN flag set.
	 *   And after log writes back, this tree block is not traced by
	 *   any dirty extent_io_tree.
	 *
	 * - Offending tree block gets re-dirtied from its original owner
	 *   Since it has bumped generation, no WRITTEN flag, it can be
	 *   reused without COWing. This tree block will not be traced
	 *   by btrfs_transaction::dirty_pages.
	 *
	 *   Now such dirty tree block will not be cleaned by any dirty
	 *   extent io tree. Thus we don't want to submit such wild eb
	 *   if the fs already has error.
	 */
	if (!test_bit(BTRFS_FS_STATE_ERROR, &fs_info->fs_state)) {
		ret = flush_write_bio(&epd);
	} else {
		ret = -EUCLEAN;
		end_write_bio(&epd, ret);
	}
	return ret;
}

/**