			goto success;
		}
	}
	pr_info("Fast TSC calibration failed\n");
	return 0;

success:
	/*