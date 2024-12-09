		cr_wdt_csr = NCT6102D_WDT_CSR;
		break;
	case NCT6116_ID:
		ret = nct6116;
		cr_wdt_timeout = NCT6102D_WDT_TIMEOUT;
		cr_wdt_control = NCT6102D_WDT_CONTROL;
		cr_wdt_csr = NCT6102D_WDT_CSR;
		break;