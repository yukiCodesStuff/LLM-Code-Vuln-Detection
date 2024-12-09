			goto success;
		}
	}
	pr_err("Fast TSC calibration failed\n");
	return 0;

success:
	/*