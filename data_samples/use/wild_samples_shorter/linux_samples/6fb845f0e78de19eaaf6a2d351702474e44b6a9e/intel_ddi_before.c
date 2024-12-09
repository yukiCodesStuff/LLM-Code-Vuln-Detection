			return DDI_CLK_SEL_TBT_810;
		default:
			MISSING_CASE(clock);
			break;
		}
	case DPLL_ID_ICL_MGPLL1:
	case DPLL_ID_ICL_MGPLL2:
	case DPLL_ID_ICL_MGPLL3: