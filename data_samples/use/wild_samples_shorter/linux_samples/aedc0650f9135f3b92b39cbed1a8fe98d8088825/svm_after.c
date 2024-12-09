		if (npt_enabled)
			entry->edx |= F(NPT);

	}
}

static int svm_get_lpage_level(void)