		break;
	case amd_pp_dpp_clock:
		pclk_vol_table = pinfo->vdd_dep_on_dppclk;
	default:
		return -EINVAL;
	}
