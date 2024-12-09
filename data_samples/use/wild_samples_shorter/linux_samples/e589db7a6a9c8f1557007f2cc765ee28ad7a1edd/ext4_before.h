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
extern int ext4_calculate_overhead(struct super_block *sb);
extern int ext4_superblock_csum_verify(struct super_block *sb,
				       struct ext4_super_block *es);
extern void ext4_superblock_csum_set(struct super_block *sb,
				     struct ext4_super_block *es);
extern void *ext4_kvmalloc(size_t size, gfp_t flags);
extern void *ext4_kvzalloc(size_t size, gfp_t flags);
extern void ext4_kvfree(void *ptr);
extern int ext4_alloc_flex_bg_array(struct super_block *sb,