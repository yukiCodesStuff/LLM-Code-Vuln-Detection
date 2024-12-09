
#define DRV_DESCRIPTION	"Intel(R) Wireless WiFi driver for Linux"
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_LICENSE("GPL");

#ifdef CONFIG_IWLWIFI_DEBUGFS
static struct dentry *iwl_dbgfs_root;
	return 0;
}

static void iwl_drv_set_dump_exclude(struct iwl_drv *drv,
				     enum iwl_ucode_tlv_type tlv_type,
				     const void *tlv_data, u32 tlv_len)
{
	const struct iwl_fw_dump_exclude *fw = tlv_data;
	struct iwl_dump_exclude *excl;

	if (tlv_len < sizeof(*fw))
		return;

	if (tlv_type == IWL_UCODE_TLV_SEC_TABLE_ADDR) {
		excl = &drv->fw.dump_excl[0];

		/* second time we find this, it's for WoWLAN */
		if (excl->addr)
			excl = &drv->fw.dump_excl_wowlan[0];
	} else if (fw_has_capa(&drv->fw.ucode_capa,
			       IWL_UCODE_TLV_CAPA_CNSLDTD_D3_D0_IMG)) {
		/* IWL_UCODE_TLV_D3_KEK_KCK_ADDR is regular image */
		excl = &drv->fw.dump_excl[0];
	} else {
		/* IWL_UCODE_TLV_D3_KEK_KCK_ADDR is WoWLAN image */
		excl = &drv->fw.dump_excl_wowlan[0];
	}

	if (excl->addr)
		excl++;

	if (excl->addr) {
		IWL_DEBUG_FW_INFO(drv, "found too many excludes in fw file\n");
		return;
	}

	excl->addr = le32_to_cpu(fw->addr) & ~FW_ADDR_CACHE_CONTROL;
	excl->size = le32_to_cpu(fw->size);
}

static int iwl_parse_tlv_firmware(struct iwl_drv *drv,
				const struct firmware *ucode_raw,
				struct iwl_firmware_pieces *pieces,
				struct iwl_ucode_capabilities *capa,
		case IWL_UCODE_TLV_TYPE_HCMD:
		case IWL_UCODE_TLV_TYPE_REGIONS:
		case IWL_UCODE_TLV_TYPE_TRIGGERS:
		case IWL_UCODE_TLV_TYPE_CONF_SET:
			if (iwlwifi_mod_params.enable_ini)
				iwl_dbg_tlv_alloc(drv->trans, tlv, false);
			break;
		case IWL_UCODE_TLV_CMD_VERSIONS:
				return -ENOMEM;
			drv->fw.phy_integration_ver_len = tlv_len;
			break;
		case IWL_UCODE_TLV_SEC_TABLE_ADDR:
		case IWL_UCODE_TLV_D3_KEK_KCK_ADDR:
			iwl_drv_set_dump_exclude(drv, tlv_type,
						 tlv_data, tlv_len);
			break;
		default:
			IWL_DEBUG_INFO(drv, "unknown TLV: %d\n", tlv_type);
			break;
		}