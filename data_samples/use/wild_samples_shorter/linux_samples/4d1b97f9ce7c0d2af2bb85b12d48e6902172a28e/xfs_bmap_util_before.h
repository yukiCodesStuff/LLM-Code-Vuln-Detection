
/* preallocation and hole punch interface */
int	xfs_alloc_file_space(struct xfs_inode *ip, xfs_off_t offset,
			     xfs_off_t len, int alloc_type);
int	xfs_free_file_space(struct xfs_inode *ip, xfs_off_t offset,
			    xfs_off_t len);
int	xfs_collapse_file_space(struct xfs_inode *, xfs_off_t offset,
				xfs_off_t len);