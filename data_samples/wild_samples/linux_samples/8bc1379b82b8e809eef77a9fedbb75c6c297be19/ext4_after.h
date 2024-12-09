{
	spin_unlock(ext4_group_lock_ptr(sb, group));
}

/*
 * Block validity checking
 */
#define ext4_check_indirect_blockref(inode, bh)				\
	ext4_check_blockref(__func__, __LINE__, inode,			\
			    (__le32 *)(bh)->b_data,			\
			    EXT4_ADDR_PER_BLOCK((inode)->i_sb))

#define ext4_ind_check_inode(inode)					\
	ext4_check_blockref(__func__, __LINE__, inode,			\
			    EXT4_I(inode)->i_data,			\
			    EXT4_NDIR_BLOCKS)

/*
 * Inodes and files operations
 */

/* dir.c */
extern const struct file_operations ext4_dir_operations;

/* file.c */
extern const struct inode_operations ext4_file_inode_operations;
extern const struct file_operations ext4_file_operations;
extern loff_t ext4_llseek(struct file *file, loff_t offset, int origin);

/* inline.c */
extern int ext4_get_max_inline_size(struct inode *inode);
extern int ext4_find_inline_data_nolock(struct inode *inode);
extern int ext4_init_inline_data(handle_t *handle, struct inode *inode,
				 unsigned int len);
extern int ext4_destroy_inline_data(handle_t *handle, struct inode *inode);

extern int ext4_readpage_inline(struct inode *inode, struct page *page);
extern int ext4_try_to_write_inline_data(struct address_space *mapping,
					 struct inode *inode,
					 loff_t pos, unsigned len,
					 unsigned flags,
					 struct page **pagep);
extern int ext4_write_inline_data_end(struct inode *inode,
				      loff_t pos, unsigned len,
				      unsigned copied,
				      struct page *page);
extern struct buffer_head *
ext4_journalled_write_inline_data(struct inode *inode,
				  unsigned len,
				  struct page *page);
extern int ext4_da_write_inline_data_begin(struct address_space *mapping,
					   struct inode *inode,
					   loff_t pos, unsigned len,
					   unsigned flags,
					   struct page **pagep,
					   void **fsdata);
extern int ext4_da_write_inline_data_end(struct inode *inode, loff_t pos,
					 unsigned len, unsigned copied,
					 struct page *page);
extern int ext4_try_add_inline_entry(handle_t *handle,
				     struct ext4_filename *fname,
				     struct inode *dir, struct inode *inode);
extern int ext4_try_create_inline_dir(handle_t *handle,
				      struct inode *parent,
				      struct inode *inode);
extern int ext4_read_inline_dir(struct file *filp,
				struct dir_context *ctx,
				int *has_inline_data);
extern int htree_inlinedir_to_tree(struct file *dir_file,
				   struct inode *dir, ext4_lblk_t block,
				   struct dx_hash_info *hinfo,
				   __u32 start_hash, __u32 start_minor_hash,
				   int *has_inline_data);
extern struct buffer_head *ext4_find_inline_entry(struct inode *dir,
					struct ext4_filename *fname,
					struct ext4_dir_entry_2 **res_dir,
					int *has_inline_data);
extern int ext4_delete_inline_entry(handle_t *handle,
				    struct inode *dir,
				    struct ext4_dir_entry_2 *de_del,
				    struct buffer_head *bh,
				    int *has_inline_data);
extern bool empty_inline_dir(struct inode *dir, int *has_inline_data);
extern struct buffer_head *ext4_get_first_inline_block(struct inode *inode,
					struct ext4_dir_entry_2 **parent_de,
					int *retval);
extern int ext4_inline_data_fiemap(struct inode *inode,
				   struct fiemap_extent_info *fieinfo,
				   int *has_inline, __u64 start, __u64 len);

struct iomap;
extern int ext4_inline_data_iomap(struct inode *inode, struct iomap *iomap);

extern int ext4_inline_data_truncate(struct inode *inode, int *has_inline);

extern int ext4_convert_inline_data(struct inode *inode);

static inline int ext4_has_inline_data(struct inode *inode)
{
	return ext4_test_inode_flag(inode, EXT4_INODE_INLINE_DATA) &&
	       EXT4_I(inode)->i_inline_off;
}