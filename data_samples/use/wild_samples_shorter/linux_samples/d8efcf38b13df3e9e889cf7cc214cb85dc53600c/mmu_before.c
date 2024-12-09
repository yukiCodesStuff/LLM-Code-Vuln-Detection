			break;
		}

		if (!is_shadow_present_pte(*iterator.sptep)) {
			u64 base_addr = iterator.addr;

			base_addr &= PT64_LVL_ADDR_MASK(iterator.level);