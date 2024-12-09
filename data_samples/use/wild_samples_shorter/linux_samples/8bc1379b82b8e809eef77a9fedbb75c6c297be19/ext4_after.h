struct iomap;
extern int ext4_inline_data_iomap(struct inode *inode, struct iomap *iomap);

extern int ext4_inline_data_truncate(struct inode *inode, int *has_inline);

extern int ext4_convert_inline_data(struct inode *inode);
