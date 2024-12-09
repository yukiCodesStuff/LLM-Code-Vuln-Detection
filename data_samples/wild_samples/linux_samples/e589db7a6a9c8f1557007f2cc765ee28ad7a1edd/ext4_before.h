struct mmpd_data {
	struct buffer_head *bh; /* bh from initial read_mmp_block() */
	struct super_block *sb;  /* super block of the fs */
};

/*
 * Check interval multiplier
 * The MMP block is written every update interval and initially checked every
 * update interval x the multiplier (the value is then adapted based on the
 * write latency). The reason is that writes can be delayed under load and we
 * don't want readers to incorrectly assume that the filesystem is no longer
 * in use.
 */
#define EXT4_MMP_CHECK_MULT		2UL

/*
 * Minimum interval for MMP checking in seconds.
 */
#define EXT4_MMP_MIN_CHECK_INTERVAL	5UL

/*
 * Maximum interval for MMP checking in seconds.
 */
#define EXT4_MMP_MAX_CHECK_INTERVAL	300UL

/*
 * Function prototypes
 */

/*
 * Ok, these declarations are also in <linux/kernel.h> but none of the
 * ext4 source programs needs to include it so they are duplicated here.
 */
# define NORET_TYPE	/**/
# define ATTRIB_NORET	__attribute__((noreturn))
# define NORET_AND	noreturn,

/* bitmap.c */
extern unsigned int ext4_count_free(char *bitmap, unsigned numchars);
void ext4_inode_bitmap_csum_set(struct super_block *sb, ext4_group_t group,
				struct ext4_group_desc *gdp,
				struct buffer_head *bh, int sz);
int ext4_inode_bitmap_csum_verify(struct super_block *sb, ext4_group_t group,
				  struct ext4_group_desc *gdp,
				  struct buffer_head *bh, int sz);
void ext4_block_bitmap_csum_set(struct super_block *sb, ext4_group_t group,
				struct ext4_group_desc *gdp,
				struct buffer_head *bh, int sz);
int ext4_block_bitmap_csum_verify(struct super_block *sb, ext4_group_t group,
				  struct ext4_group_desc *gdp,
				  struct buffer_head *bh, int sz);

/* balloc.c */
extern void ext4_validate_block_bitmap(struct super_block *sb,
				       struct ext4_group_desc *desc,
				       unsigned int block_group,
				       struct buffer_head *bh);
extern unsigned int ext4_block_group(struct super_block *sb,
			ext4_fsblk_t blocknr);
extern ext4_grpblk_t ext4_block_group_offset(struct super_block *sb,
			ext4_fsblk_t blocknr);
extern int ext4_bg_has_super(struct super_block *sb, ext4_group_t group);
extern unsigned long ext4_bg_num_gdb(struct super_block *sb,
			ext4_group_t group);
extern ext4_fsblk_t ext4_new_meta_blocks(handle_t *handle, struct inode *inode,
					 ext4_fsblk_t goal,
					 unsigned int flags,
					 unsigned long *count,
					 int *errp);
extern int ext4_claim_free_clusters(struct ext4_sb_info *sbi,
				    s64 nclusters, unsigned int flags);
extern ext4_fsblk_t ext4_count_free_clusters(struct super_block *);
extern void ext4_check_blocks_bitmap(struct super_block *);
extern struct ext4_group_desc * ext4_get_group_desc(struct super_block * sb,
						    ext4_group_t block_group,
						    struct buffer_head ** bh);
extern int ext4_should_retry_alloc(struct super_block *sb, int *retries);

extern struct buffer_head *ext4_read_block_bitmap_nowait(struct super_block *sb,
						ext4_group_t block_group);
extern int ext4_wait_block_bitmap(struct super_block *sb,
				  ext4_group_t block_group,
				  struct buffer_head *bh);
extern struct buffer_head *ext4_read_block_bitmap(struct super_block *sb,
						  ext4_group_t block_group);
extern void ext4_init_block_bitmap(struct super_block *sb,
				   struct buffer_head *bh,
				   ext4_group_t group,
				   struct ext4_group_desc *desc);
extern unsigned ext4_free_clusters_after_init(struct super_block *sb,
					      ext4_group_t block_group,
					      struct ext4_group_desc *gdp);
extern unsigned ext4_num_overhead_clusters(struct super_block *sb,
					   ext4_group_t block_group,
					   struct ext4_group_desc *gdp);
ext4_fsblk_t ext4_inode_to_goal_block(struct inode *);

/* dir.c */
extern int __ext4_check_dir_entry(const char *, unsigned int, struct inode *,
				  struct file *,
				  struct ext4_dir_entry_2 *,
				  struct buffer_head *, unsigned int);
#define ext4_check_dir_entry(dir, filp, de, bh, offset)			\
	unlikely(__ext4_check_dir_entry(__func__, __LINE__, (dir), (filp), \
					(de), (bh), (offset)))
extern int ext4_htree_store_dirent(struct file *dir_file, __u32 hash,
				    __u32 minor_hash,
				    struct ext4_dir_entry_2 *dirent);
extern void ext4_htree_free_dir_info(struct dir_private_info *p);

/* fsync.c */
extern int ext4_sync_file(struct file *, loff_t, loff_t, int);
extern int ext4_flush_unwritten_io(struct inode *);

/* hash.c */
extern int ext4fs_dirhash(const char *name, int len, struct
			  dx_hash_info *hinfo);

/* ialloc.c */
extern struct inode *ext4_new_inode(handle_t *, struct inode *, umode_t,
				    const struct qstr *qstr, __u32 goal,
				    uid_t *owner);
extern void ext4_free_inode(handle_t *, struct inode *);
extern struct inode * ext4_orphan_get(struct super_block *, unsigned long);
extern unsigned long ext4_count_free_inodes(struct super_block *);
extern unsigned long ext4_count_dirs(struct super_block *);
extern void ext4_check_inodes_bitmap(struct super_block *);
extern void ext4_mark_bitmap_end(int start_bit, int end_bit, char *bitmap);
extern int ext4_init_inode_table(struct super_block *sb,
				 ext4_group_t group, int barrier);
extern void ext4_end_bitmap_read(struct buffer_head *bh, int uptodate);

/* mballoc.c */
extern long ext4_mb_stats;
extern long ext4_mb_max_to_scan;
extern int ext4_mb_init(struct super_block *);
extern int ext4_mb_release(struct super_block *);
extern ext4_fsblk_t ext4_mb_new_blocks(handle_t *,
				struct ext4_allocation_request *, int *);
extern int ext4_mb_reserve_blocks(struct super_block *, int);
extern void ext4_discard_preallocations(struct inode *);
extern int __init ext4_init_mballoc(void);
extern void ext4_exit_mballoc(void);
extern void ext4_free_blocks(handle_t *handle, struct inode *inode,
			     struct buffer_head *bh, ext4_fsblk_t block,
			     unsigned long count, int flags);
extern int ext4_mb_alloc_groupinfo(struct super_block *sb,
				   ext4_group_t ngroups);
extern int ext4_mb_add_groupinfo(struct super_block *sb,
		ext4_group_t i, struct ext4_group_desc *desc);
extern int ext4_group_add_blocks(handle_t *handle, struct super_block *sb,
				ext4_fsblk_t block, unsigned long count);
extern int ext4_trim_fs(struct super_block *, struct fstrim_range *);

/* inode.c */
struct buffer_head *ext4_getblk(handle_t *, struct inode *,
						ext4_lblk_t, int, int *);
struct buffer_head *ext4_bread(handle_t *, struct inode *,
						ext4_lblk_t, int, int *);
int ext4_get_block(struct inode *inode, sector_t iblock,
				struct buffer_head *bh_result, int create);

extern struct inode *ext4_iget(struct super_block *, unsigned long);
extern int  ext4_write_inode(struct inode *, struct writeback_control *);
extern int  ext4_setattr(struct dentry *, struct iattr *);
extern int  ext4_getattr(struct vfsmount *mnt, struct dentry *dentry,
				struct kstat *stat);
extern void ext4_evict_inode(struct inode *);
extern void ext4_clear_inode(struct inode *);
extern int  ext4_sync_inode(handle_t *, struct inode *);
extern void ext4_dirty_inode(struct inode *, int);
extern int ext4_change_inode_journal_flag(struct inode *, int);
extern int ext4_get_inode_loc(struct inode *, struct ext4_iloc *);
extern int ext4_can_truncate(struct inode *inode);
extern void ext4_truncate(struct inode *);
extern int ext4_punch_hole(struct file *file, loff_t offset, loff_t length);
extern int ext4_truncate_restart_trans(handle_t *, struct inode *, int nblocks);
extern void ext4_set_inode_flags(struct inode *);
extern void ext4_get_inode_flags(struct ext4_inode_info *);
extern int ext4_alloc_da_blocks(struct inode *inode);
extern void ext4_set_aops(struct inode *inode);
extern int ext4_writepage_trans_blocks(struct inode *);
extern int ext4_chunk_trans_blocks(struct inode *, int nrblocks);
extern int ext4_discard_partial_page_buffers(handle_t *handle,
		struct address_space *mapping, loff_t from,
		loff_t length, int flags);
extern int ext4_page_mkwrite(struct vm_area_struct *vma, struct vm_fault *vmf);
extern qsize_t *ext4_get_reserved_space(struct inode *inode);
extern void ext4_da_update_reserve_space(struct inode *inode,
					int used, int quota_claim);

/* indirect.c */
extern int ext4_ind_map_blocks(handle_t *handle, struct inode *inode,
				struct ext4_map_blocks *map, int flags);
extern ssize_t ext4_ind_direct_IO(int rw, struct kiocb *iocb,
				const struct iovec *iov, loff_t offset,
				unsigned long nr_segs);
extern int ext4_ind_calc_metadata_amount(struct inode *inode, sector_t lblock);
extern int ext4_ind_trans_blocks(struct inode *inode, int nrblocks, int chunk);
extern void ext4_ind_truncate(struct inode *inode);

/* ioctl.c */
extern long ext4_ioctl(struct file *, unsigned int, unsigned long);
extern long ext4_compat_ioctl(struct file *, unsigned int, unsigned long);

/* migrate.c */
extern int ext4_ext_migrate(struct inode *);

/* namei.c */
extern int ext4_dirent_csum_verify(struct inode *inode,
				   struct ext4_dir_entry *dirent);
extern int ext4_orphan_add(handle_t *, struct inode *);
extern int ext4_orphan_del(handle_t *, struct inode *);
extern int ext4_htree_fill_tree(struct file *dir_file, __u32 start_hash,
				__u32 start_minor_hash, __u32 *next_hash);

/* resize.c */
extern int ext4_group_add(struct super_block *sb,
				struct ext4_new_group_data *input);
extern int ext4_group_extend(struct super_block *sb,
				struct ext4_super_block *es,
				ext4_fsblk_t n_blocks_count);
extern int ext4_resize_fs(struct super_block *sb, ext4_fsblk_t n_blocks_count);

/* super.c */
extern int ext4_calculate_overhead(struct super_block *sb);
extern int ext4_superblock_csum_verify(struct super_block *sb,
				       struct ext4_super_block *es);
extern void ext4_superblock_csum_set(struct super_block *sb,
				     struct ext4_super_block *es);
extern void *ext4_kvmalloc(size_t size, gfp_t flags);
extern void *ext4_kvzalloc(size_t size, gfp_t flags);
extern void ext4_kvfree(void *ptr);
extern int ext4_alloc_flex_bg_array(struct super_block *sb,
				    ext4_group_t ngroup);
extern __printf(4, 5)
void __ext4_error(struct super_block *, const char *, unsigned int,
		  const char *, ...);
#define ext4_error(sb, message...)	__ext4_error(sb, __func__,	\
						     __LINE__, ## message)
extern __printf(5, 6)
void ext4_error_inode(struct inode *, const char *, unsigned int, ext4_fsblk_t,
		      const char *, ...);
extern __printf(5, 6)
void ext4_error_file(struct file *, const char *, unsigned int, ext4_fsblk_t,
		     const char *, ...);
extern void __ext4_std_error(struct super_block *, const char *,
			     unsigned int, int);
extern __printf(4, 5)
void __ext4_abort(struct super_block *, const char *, unsigned int,
		  const char *, ...);
#define ext4_abort(sb, message...)	__ext4_abort(sb, __func__, \
						       __LINE__, ## message)
extern __printf(4, 5)
void __ext4_warning(struct super_block *, const char *, unsigned int,
		    const char *, ...);
#define ext4_warning(sb, message...)	__ext4_warning(sb, __func__, \
						       __LINE__, ## message)
extern __printf(3, 4)
void ext4_msg(struct super_block *, const char *, const char *, ...);
extern void __dump_mmp_msg(struct super_block *, struct mmp_struct *mmp,
			   const char *, unsigned int, const char *);
#define dump_mmp_msg(sb, mmp, msg)	__dump_mmp_msg(sb, mmp, __func__, \
						       __LINE__, msg)
extern __printf(7, 8)
void __ext4_grp_locked_error(const char *, unsigned int,
			     struct super_block *, ext4_group_t,
			     unsigned long, ext4_fsblk_t,
			     const char *, ...);
#define ext4_grp_locked_error(sb, grp, message...) \
	__ext4_grp_locked_error(__func__, __LINE__, (sb), (grp), ## message)
extern void ext4_update_dynamic_rev(struct super_block *sb);
extern int ext4_update_compat_feature(handle_t *handle, struct super_block *sb,
					__u32 compat);
extern int ext4_update_rocompat_feature(handle_t *handle,
					struct super_block *sb,	__u32 rocompat);
extern int ext4_update_incompat_feature(handle_t *handle,
					struct super_block *sb,	__u32 incompat);
extern ext4_fsblk_t ext4_block_bitmap(struct super_block *sb,
				      struct ext4_group_desc *bg);
extern ext4_fsblk_t ext4_inode_bitmap(struct super_block *sb,
				      struct ext4_group_desc *bg);
extern ext4_fsblk_t ext4_inode_table(struct super_block *sb,
				     struct ext4_group_desc *bg);
extern __u32 ext4_free_group_clusters(struct super_block *sb,
				      struct ext4_group_desc *bg);
extern __u32 ext4_free_inodes_count(struct super_block *sb,
				 struct ext4_group_desc *bg);
extern __u32 ext4_used_dirs_count(struct super_block *sb,
				struct ext4_group_desc *bg);
extern __u32 ext4_itable_unused_count(struct super_block *sb,
				   struct ext4_group_desc *bg);
extern void ext4_block_bitmap_set(struct super_block *sb,
				  struct ext4_group_desc *bg, ext4_fsblk_t blk);
extern void ext4_inode_bitmap_set(struct super_block *sb,
				  struct ext4_group_desc *bg, ext4_fsblk_t blk);
extern void ext4_inode_table_set(struct super_block *sb,
				 struct ext4_group_desc *bg, ext4_fsblk_t blk);
extern void ext4_free_group_clusters_set(struct super_block *sb,
					 struct ext4_group_desc *bg,
					 __u32 count);
extern void ext4_free_inodes_set(struct super_block *sb,
				struct ext4_group_desc *bg, __u32 count);
extern void ext4_used_dirs_set(struct super_block *sb,
				struct ext4_group_desc *bg, __u32 count);
extern void ext4_itable_unused_set(struct super_block *sb,
				   struct ext4_group_desc *bg, __u32 count);
extern int ext4_group_desc_csum_verify(struct super_block *sb, __u32 group,
				       struct ext4_group_desc *gdp);
extern void ext4_group_desc_csum_set(struct super_block *sb, __u32 group,
				     struct ext4_group_desc *gdp);

static inline int ext4_has_group_desc_csum(struct super_block *sb)
{
	return EXT4_HAS_RO_COMPAT_FEATURE(sb,
					  EXT4_FEATURE_RO_COMPAT_GDT_CSUM |
					  EXT4_FEATURE_RO_COMPAT_METADATA_CSUM);
}
struct mmpd_data {
	struct buffer_head *bh; /* bh from initial read_mmp_block() */
	struct super_block *sb;  /* super block of the fs */
};

/*
 * Check interval multiplier
 * The MMP block is written every update interval and initially checked every
 * update interval x the multiplier (the value is then adapted based on the
 * write latency). The reason is that writes can be delayed under load and we
 * don't want readers to incorrectly assume that the filesystem is no longer
 * in use.
 */
#define EXT4_MMP_CHECK_MULT		2UL

/*
 * Minimum interval for MMP checking in seconds.
 */
#define EXT4_MMP_MIN_CHECK_INTERVAL	5UL

/*
 * Maximum interval for MMP checking in seconds.
 */
#define EXT4_MMP_MAX_CHECK_INTERVAL	300UL

/*
 * Function prototypes
 */

/*
 * Ok, these declarations are also in <linux/kernel.h> but none of the
 * ext4 source programs needs to include it so they are duplicated here.
 */
# define NORET_TYPE	/**/
# define ATTRIB_NORET	__attribute__((noreturn))
# define NORET_AND	noreturn,

/* bitmap.c */
extern unsigned int ext4_count_free(char *bitmap, unsigned numchars);
void ext4_inode_bitmap_csum_set(struct super_block *sb, ext4_group_t group,
				struct ext4_group_desc *gdp,
				struct buffer_head *bh, int sz);
int ext4_inode_bitmap_csum_verify(struct super_block *sb, ext4_group_t group,
				  struct ext4_group_desc *gdp,
				  struct buffer_head *bh, int sz);
void ext4_block_bitmap_csum_set(struct super_block *sb, ext4_group_t group,
				struct ext4_group_desc *gdp,
				struct buffer_head *bh, int sz);
int ext4_block_bitmap_csum_verify(struct super_block *sb, ext4_group_t group,
				  struct ext4_group_desc *gdp,
				  struct buffer_head *bh, int sz);

/* balloc.c */
extern void ext4_validate_block_bitmap(struct super_block *sb,
				       struct ext4_group_desc *desc,
				       unsigned int block_group,
				       struct buffer_head *bh);
extern unsigned int ext4_block_group(struct super_block *sb,
			ext4_fsblk_t blocknr);
extern ext4_grpblk_t ext4_block_group_offset(struct super_block *sb,
			ext4_fsblk_t blocknr);
extern int ext4_bg_has_super(struct super_block *sb, ext4_group_t group);
extern unsigned long ext4_bg_num_gdb(struct super_block *sb,
			ext4_group_t group);
extern ext4_fsblk_t ext4_new_meta_blocks(handle_t *handle, struct inode *inode,
					 ext4_fsblk_t goal,
					 unsigned int flags,
					 unsigned long *count,
					 int *errp);
extern int ext4_claim_free_clusters(struct ext4_sb_info *sbi,
				    s64 nclusters, unsigned int flags);
extern ext4_fsblk_t ext4_count_free_clusters(struct super_block *);
extern void ext4_check_blocks_bitmap(struct super_block *);
extern struct ext4_group_desc * ext4_get_group_desc(struct super_block * sb,
						    ext4_group_t block_group,
						    struct buffer_head ** bh);
extern int ext4_should_retry_alloc(struct super_block *sb, int *retries);

extern struct buffer_head *ext4_read_block_bitmap_nowait(struct super_block *sb,
						ext4_group_t block_group);
extern int ext4_wait_block_bitmap(struct super_block *sb,
				  ext4_group_t block_group,
				  struct buffer_head *bh);
extern struct buffer_head *ext4_read_block_bitmap(struct super_block *sb,
						  ext4_group_t block_group);
extern void ext4_init_block_bitmap(struct super_block *sb,
				   struct buffer_head *bh,
				   ext4_group_t group,
				   struct ext4_group_desc *desc);
extern unsigned ext4_free_clusters_after_init(struct super_block *sb,
					      ext4_group_t block_group,
					      struct ext4_group_desc *gdp);
extern unsigned ext4_num_overhead_clusters(struct super_block *sb,
					   ext4_group_t block_group,
					   struct ext4_group_desc *gdp);
ext4_fsblk_t ext4_inode_to_goal_block(struct inode *);

/* dir.c */
extern int __ext4_check_dir_entry(const char *, unsigned int, struct inode *,
				  struct file *,
				  struct ext4_dir_entry_2 *,
				  struct buffer_head *, unsigned int);
#define ext4_check_dir_entry(dir, filp, de, bh, offset)			\
	unlikely(__ext4_check_dir_entry(__func__, __LINE__, (dir), (filp), \
					(de), (bh), (offset)))
extern int ext4_htree_store_dirent(struct file *dir_file, __u32 hash,
				    __u32 minor_hash,
				    struct ext4_dir_entry_2 *dirent);
extern void ext4_htree_free_dir_info(struct dir_private_info *p);

/* fsync.c */
extern int ext4_sync_file(struct file *, loff_t, loff_t, int);
extern int ext4_flush_unwritten_io(struct inode *);

/* hash.c */
extern int ext4fs_dirhash(const char *name, int len, struct
			  dx_hash_info *hinfo);

/* ialloc.c */
extern struct inode *ext4_new_inode(handle_t *, struct inode *, umode_t,
				    const struct qstr *qstr, __u32 goal,
				    uid_t *owner);
extern void ext4_free_inode(handle_t *, struct inode *);
extern struct inode * ext4_orphan_get(struct super_block *, unsigned long);
extern unsigned long ext4_count_free_inodes(struct super_block *);
extern unsigned long ext4_count_dirs(struct super_block *);
extern void ext4_check_inodes_bitmap(struct super_block *);
extern void ext4_mark_bitmap_end(int start_bit, int end_bit, char *bitmap);
extern int ext4_init_inode_table(struct super_block *sb,
				 ext4_group_t group, int barrier);
extern void ext4_end_bitmap_read(struct buffer_head *bh, int uptodate);

/* mballoc.c */
extern long ext4_mb_stats;
extern long ext4_mb_max_to_scan;
extern int ext4_mb_init(struct super_block *);
extern int ext4_mb_release(struct super_block *);
extern ext4_fsblk_t ext4_mb_new_blocks(handle_t *,
				struct ext4_allocation_request *, int *);
extern int ext4_mb_reserve_blocks(struct super_block *, int);
extern void ext4_discard_preallocations(struct inode *);
extern int __init ext4_init_mballoc(void);
extern void ext4_exit_mballoc(void);
extern void ext4_free_blocks(handle_t *handle, struct inode *inode,
			     struct buffer_head *bh, ext4_fsblk_t block,
			     unsigned long count, int flags);
extern int ext4_mb_alloc_groupinfo(struct super_block *sb,
				   ext4_group_t ngroups);
extern int ext4_mb_add_groupinfo(struct super_block *sb,
		ext4_group_t i, struct ext4_group_desc *desc);
extern int ext4_group_add_blocks(handle_t *handle, struct super_block *sb,
				ext4_fsblk_t block, unsigned long count);
extern int ext4_trim_fs(struct super_block *, struct fstrim_range *);

/* inode.c */
struct buffer_head *ext4_getblk(handle_t *, struct inode *,
						ext4_lblk_t, int, int *);
struct buffer_head *ext4_bread(handle_t *, struct inode *,
						ext4_lblk_t, int, int *);
int ext4_get_block(struct inode *inode, sector_t iblock,
				struct buffer_head *bh_result, int create);

extern struct inode *ext4_iget(struct super_block *, unsigned long);
extern int  ext4_write_inode(struct inode *, struct writeback_control *);
extern int  ext4_setattr(struct dentry *, struct iattr *);
extern int  ext4_getattr(struct vfsmount *mnt, struct dentry *dentry,
				struct kstat *stat);
extern void ext4_evict_inode(struct inode *);
extern void ext4_clear_inode(struct inode *);
extern int  ext4_sync_inode(handle_t *, struct inode *);
extern void ext4_dirty_inode(struct inode *, int);
extern int ext4_change_inode_journal_flag(struct inode *, int);
extern int ext4_get_inode_loc(struct inode *, struct ext4_iloc *);
extern int ext4_can_truncate(struct inode *inode);
extern void ext4_truncate(struct inode *);
extern int ext4_punch_hole(struct file *file, loff_t offset, loff_t length);
extern int ext4_truncate_restart_trans(handle_t *, struct inode *, int nblocks);
extern void ext4_set_inode_flags(struct inode *);
extern void ext4_get_inode_flags(struct ext4_inode_info *);
extern int ext4_alloc_da_blocks(struct inode *inode);
extern void ext4_set_aops(struct inode *inode);
extern int ext4_writepage_trans_blocks(struct inode *);
extern int ext4_chunk_trans_blocks(struct inode *, int nrblocks);
extern int ext4_discard_partial_page_buffers(handle_t *handle,
		struct address_space *mapping, loff_t from,
		loff_t length, int flags);
extern int ext4_page_mkwrite(struct vm_area_struct *vma, struct vm_fault *vmf);
extern qsize_t *ext4_get_reserved_space(struct inode *inode);
extern void ext4_da_update_reserve_space(struct inode *inode,
					int used, int quota_claim);

/* indirect.c */
extern int ext4_ind_map_blocks(handle_t *handle, struct inode *inode,
				struct ext4_map_blocks *map, int flags);
extern ssize_t ext4_ind_direct_IO(int rw, struct kiocb *iocb,
				const struct iovec *iov, loff_t offset,
				unsigned long nr_segs);
extern int ext4_ind_calc_metadata_amount(struct inode *inode, sector_t lblock);
extern int ext4_ind_trans_blocks(struct inode *inode, int nrblocks, int chunk);
extern void ext4_ind_truncate(struct inode *inode);

/* ioctl.c */
extern long ext4_ioctl(struct file *, unsigned int, unsigned long);
extern long ext4_compat_ioctl(struct file *, unsigned int, unsigned long);

/* migrate.c */
extern int ext4_ext_migrate(struct inode *);

/* namei.c */
extern int ext4_dirent_csum_verify(struct inode *inode,
				   struct ext4_dir_entry *dirent);
extern int ext4_orphan_add(handle_t *, struct inode *);
extern int ext4_orphan_del(handle_t *, struct inode *);
extern int ext4_htree_fill_tree(struct file *dir_file, __u32 start_hash,
				__u32 start_minor_hash, __u32 *next_hash);

/* resize.c */
extern int ext4_group_add(struct super_block *sb,
				struct ext4_new_group_data *input);
extern int ext4_group_extend(struct super_block *sb,
				struct ext4_super_block *es,
				ext4_fsblk_t n_blocks_count);
extern int ext4_resize_fs(struct super_block *sb, ext4_fsblk_t n_blocks_count);

/* super.c */
extern int ext4_calculate_overhead(struct super_block *sb);
extern int ext4_superblock_csum_verify(struct super_block *sb,
				       struct ext4_super_block *es);
extern void ext4_superblock_csum_set(struct super_block *sb,
				     struct ext4_super_block *es);
extern void *ext4_kvmalloc(size_t size, gfp_t flags);
extern void *ext4_kvzalloc(size_t size, gfp_t flags);
extern void ext4_kvfree(void *ptr);
extern int ext4_alloc_flex_bg_array(struct super_block *sb,
				    ext4_group_t ngroup);
extern __printf(4, 5)
void __ext4_error(struct super_block *, const char *, unsigned int,
		  const char *, ...);
#define ext4_error(sb, message...)	__ext4_error(sb, __func__,	\
						     __LINE__, ## message)
extern __printf(5, 6)
void ext4_error_inode(struct inode *, const char *, unsigned int, ext4_fsblk_t,
		      const char *, ...);
extern __printf(5, 6)
void ext4_error_file(struct file *, const char *, unsigned int, ext4_fsblk_t,
		     const char *, ...);
extern void __ext4_std_error(struct super_block *, const char *,
			     unsigned int, int);
extern __printf(4, 5)
void __ext4_abort(struct super_block *, const char *, unsigned int,
		  const char *, ...);
#define ext4_abort(sb, message...)	__ext4_abort(sb, __func__, \
						       __LINE__, ## message)
extern __printf(4, 5)
void __ext4_warning(struct super_block *, const char *, unsigned int,
		    const char *, ...);
#define ext4_warning(sb, message...)	__ext4_warning(sb, __func__, \
						       __LINE__, ## message)
extern __printf(3, 4)
void ext4_msg(struct super_block *, const char *, const char *, ...);
extern void __dump_mmp_msg(struct super_block *, struct mmp_struct *mmp,
			   const char *, unsigned int, const char *);
#define dump_mmp_msg(sb, mmp, msg)	__dump_mmp_msg(sb, mmp, __func__, \
						       __LINE__, msg)
extern __printf(7, 8)
void __ext4_grp_locked_error(const char *, unsigned int,
			     struct super_block *, ext4_group_t,
			     unsigned long, ext4_fsblk_t,
			     const char *, ...);
#define ext4_grp_locked_error(sb, grp, message...) \
	__ext4_grp_locked_error(__func__, __LINE__, (sb), (grp), ## message)
extern void ext4_update_dynamic_rev(struct super_block *sb);
extern int ext4_update_compat_feature(handle_t *handle, struct super_block *sb,
					__u32 compat);
extern int ext4_update_rocompat_feature(handle_t *handle,
					struct super_block *sb,	__u32 rocompat);
extern int ext4_update_incompat_feature(handle_t *handle,
					struct super_block *sb,	__u32 incompat);
extern ext4_fsblk_t ext4_block_bitmap(struct super_block *sb,
				      struct ext4_group_desc *bg);
extern ext4_fsblk_t ext4_inode_bitmap(struct super_block *sb,
				      struct ext4_group_desc *bg);
extern ext4_fsblk_t ext4_inode_table(struct super_block *sb,
				     struct ext4_group_desc *bg);
extern __u32 ext4_free_group_clusters(struct super_block *sb,
				      struct ext4_group_desc *bg);
extern __u32 ext4_free_inodes_count(struct super_block *sb,
				 struct ext4_group_desc *bg);
extern __u32 ext4_used_dirs_count(struct super_block *sb,
				struct ext4_group_desc *bg);
extern __u32 ext4_itable_unused_count(struct super_block *sb,
				   struct ext4_group_desc *bg);
extern void ext4_block_bitmap_set(struct super_block *sb,
				  struct ext4_group_desc *bg, ext4_fsblk_t blk);
extern void ext4_inode_bitmap_set(struct super_block *sb,
				  struct ext4_group_desc *bg, ext4_fsblk_t blk);
extern void ext4_inode_table_set(struct super_block *sb,
				 struct ext4_group_desc *bg, ext4_fsblk_t blk);
extern void ext4_free_group_clusters_set(struct super_block *sb,
					 struct ext4_group_desc *bg,
					 __u32 count);
extern void ext4_free_inodes_set(struct super_block *sb,
				struct ext4_group_desc *bg, __u32 count);
extern void ext4_used_dirs_set(struct super_block *sb,
				struct ext4_group_desc *bg, __u32 count);
extern void ext4_itable_unused_set(struct super_block *sb,
				   struct ext4_group_desc *bg, __u32 count);
extern int ext4_group_desc_csum_verify(struct super_block *sb, __u32 group,
				       struct ext4_group_desc *gdp);
extern void ext4_group_desc_csum_set(struct super_block *sb, __u32 group,
				     struct ext4_group_desc *gdp);

static inline int ext4_has_group_desc_csum(struct super_block *sb)
{
	return EXT4_HAS_RO_COMPAT_FEATURE(sb,
					  EXT4_FEATURE_RO_COMPAT_GDT_CSUM |
					  EXT4_FEATURE_RO_COMPAT_METADATA_CSUM);
}