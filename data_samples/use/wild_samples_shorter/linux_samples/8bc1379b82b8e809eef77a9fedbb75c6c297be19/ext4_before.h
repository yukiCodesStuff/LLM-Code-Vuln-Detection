struct iomap;
extern int ext4_inline_data_iomap(struct inode *inode, struct iomap *iomap);

extern int ext4_try_to_evict_inline_data(handle_t *handle,
					 struct inode *inode,
					 int needed);
extern int ext4_inline_data_truncate(struct inode *inode, int *has_inline);

extern int ext4_convert_inline_data(struct inode *inode);
