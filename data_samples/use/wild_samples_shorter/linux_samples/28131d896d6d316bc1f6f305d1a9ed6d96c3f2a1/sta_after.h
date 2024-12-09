 * @STA_FLG_MAX_AGG_SIZE_256K: maximal size for A-MPDU (256k supported)
 * @STA_FLG_MAX_AGG_SIZE_512K: maximal size for A-MPDU (512k supported)
 * @STA_FLG_MAX_AGG_SIZE_1024K: maximal size for A-MPDU (1024k supported)
 * @STA_FLG_MAX_AGG_SIZE_2M: maximal size for A-MPDU (2M supported)
 * @STA_FLG_MAX_AGG_SIZE_4M: maximal size for A-MPDU (4M supported)
 * @STA_FLG_AGG_MPDU_DENS_MSK: maximal MPDU density for Tx aggregation
 * @STA_FLG_FAT_EN_MSK: support for channel width (for Tx). This flag is
 *	initialised by driver and can be updated by fw upon reception of
 *	action frames that can change the channel width. When cleared the fw