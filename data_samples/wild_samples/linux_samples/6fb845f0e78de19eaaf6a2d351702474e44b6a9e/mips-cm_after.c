		} else {
			buf[0] = 0;
		}

		pr_err("CM_ERROR=%llx %s <%s>\n", cm_error,
		       cm3_causes[cause], buf);
		pr_err("CM_ADDR =%llx\n", cm_addr);
		pr_err("CM_OTHER=%llx %s\n", cm_other, cm3_causes[ocause]);
	}

	/* reprime cause register */
	write_gcr_error_cause(cm_error);
}