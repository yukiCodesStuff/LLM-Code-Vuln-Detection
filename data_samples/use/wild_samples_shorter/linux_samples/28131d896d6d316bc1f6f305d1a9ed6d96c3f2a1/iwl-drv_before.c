
#define DRV_DESCRIPTION	"Intel(R) Wireless WiFi driver for Linux"
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_LICENSE("GPL");

#ifdef CONFIG_IWLWIFI_DEBUGFS
static struct dentry *iwl_dbgfs_root;
	return 0;
}

static int iwl_parse_tlv_firmware(struct iwl_drv *drv,
				const struct firmware *ucode_raw,
				struct iwl_firmware_pieces *pieces,
				struct iwl_ucode_capabilities *capa,
		case IWL_UCODE_TLV_TYPE_HCMD:
		case IWL_UCODE_TLV_TYPE_REGIONS:
		case IWL_UCODE_TLV_TYPE_TRIGGERS:
			if (iwlwifi_mod_params.enable_ini)
				iwl_dbg_tlv_alloc(drv->trans, tlv, false);
			break;
		case IWL_UCODE_TLV_CMD_VERSIONS:
				return -ENOMEM;
			drv->fw.phy_integration_ver_len = tlv_len;
			break;
		default:
			IWL_DEBUG_INFO(drv, "unknown TLV: %d\n", tlv_type);
			break;
		}