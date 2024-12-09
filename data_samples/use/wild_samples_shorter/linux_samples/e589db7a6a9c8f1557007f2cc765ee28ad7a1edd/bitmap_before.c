
int ext4_block_bitmap_csum_verify(struct super_block *sb, ext4_group_t group,
				  struct ext4_group_desc *gdp,
				  struct buffer_head *bh, int sz)
{
	__u32 hi;
	__u32 provided, calculated;
	struct ext4_sb_info *sbi = EXT4_SB(sb);

	if (!EXT4_HAS_RO_COMPAT_FEATURE(sb,
					EXT4_FEATURE_RO_COMPAT_METADATA_CSUM))
		return 1;

void ext4_block_bitmap_csum_set(struct super_block *sb, ext4_group_t group,
				struct ext4_group_desc *gdp,
				struct buffer_head *bh, int sz)
{
	__u32 csum;
	struct ext4_sb_info *sbi = EXT4_SB(sb);

	if (!EXT4_HAS_RO_COMPAT_FEATURE(sb,