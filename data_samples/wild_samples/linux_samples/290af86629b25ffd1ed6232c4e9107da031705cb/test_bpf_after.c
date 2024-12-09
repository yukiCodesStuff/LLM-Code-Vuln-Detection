			} else {
				pr_cont("UNEXPECTED_PASS\n");
				/* Verifier didn't reject the test that's
				 * bad enough, just return!
				 */
				*err = -EINVAL;
				return NULL;
			}
		}
		if (*err) {
			pr_cont("FAIL to prog_create err=%d len=%d\n",
				*err, fprog.len);
			return NULL;
		}
		break;

	case INTERNAL:
		fp = bpf_prog_alloc(bpf_prog_size(flen), 0);
		if (fp == NULL) {
			pr_cont("UNEXPECTED_FAIL no memory left\n");
			*err = -ENOMEM;
			return NULL;
		}

		fp->len = flen;
		/* Type doesn't really matter here as long as it's not unspec. */
		fp->type = BPF_PROG_TYPE_SOCKET_FILTER;
		memcpy(fp->insnsi, fptr, fp->len * sizeof(struct bpf_insn));
		fp->aux->stack_depth = tests[which].stack_depth;

		/* We cannot error here as we don't need type compatibility
		 * checks.
		 */
		fp = bpf_prog_select_runtime(fp, err);
		if (*err) {
			pr_cont("FAIL to select_runtime err=%d\n", *err);
			return NULL;
		}
		break;
	}

	*err = 0;
	return fp;
}

static void release_filter(struct bpf_prog *fp, int which)
{
	__u8 test_type = tests[which].aux & TEST_TYPE_MASK;

	switch (test_type) {
	case CLASSIC:
		bpf_prog_destroy(fp);
		break;
	case INTERNAL:
		bpf_prog_free(fp);
		break;
	}
}

static int __run_one(const struct bpf_prog *fp, const void *data,
		     int runs, u64 *duration)
{
	u64 start, finish;
	int ret = 0, i;

	start = ktime_get_ns();

	for (i = 0; i < runs; i++)
		ret = BPF_PROG_RUN(fp, data);

	finish = ktime_get_ns();

	*duration = finish - start;
	do_div(*duration, runs);

	return ret;
}

static int run_one(const struct bpf_prog *fp, struct bpf_test *test)
{
	int err_cnt = 0, i, runs = MAX_TESTRUNS;

	for (i = 0; i < MAX_SUBTESTS; i++) {
		void *data;
		u64 duration;
		u32 ret;

		if (test->test[i].data_size == 0 &&
		    test->test[i].result == 0)
			break;

		data = generate_test_data(test, i);
		if (!data && !(test->aux & FLAG_NO_DATA)) {
			pr_cont("data generation failed ");
			err_cnt++;
			break;
		}
		ret = __run_one(fp, data, runs, &duration);
		release_test_data(test, data);

		if (ret == test->test[i].result) {
			pr_cont("%lld ", duration);
		} else {
			pr_cont("ret %d != %d ", ret,
				test->test[i].result);
			err_cnt++;
		}
	}

	return err_cnt;
}

static char test_name[64];
module_param_string(test_name, test_name, sizeof(test_name), 0);

static int test_id = -1;
module_param(test_id, int, 0);

static int test_range[2] = { 0, ARRAY_SIZE(tests) - 1 };
module_param_array(test_range, int, NULL, 0);

static __init int find_test_index(const char *test_name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		if (!strcmp(tests[i].descr, test_name))
			return i;
	}
	return -1;
}

static __init int prepare_bpf_tests(void)
{
	int i;

	if (test_id >= 0) {
		/*
		 * if a test_id was specified, use test_range to
		 * cover only that test.
		 */
		if (test_id >= ARRAY_SIZE(tests)) {
			pr_err("test_bpf: invalid test_id specified.\n");
			return -EINVAL;
		}

		test_range[0] = test_id;
		test_range[1] = test_id;
	} else if (*test_name) {
		/*
		 * if a test_name was specified, find it and setup
		 * test_range to cover only that test.
		 */
		int idx = find_test_index(test_name);

		if (idx < 0) {
			pr_err("test_bpf: no test named '%s' found.\n",
			       test_name);
			return -EINVAL;
		}
		if (fp == NULL) {
			pr_cont("UNEXPECTED_FAIL no memory left\n");
			*err = -ENOMEM;
			return NULL;
		}

		fp->len = flen;
		/* Type doesn't really matter here as long as it's not unspec. */
		fp->type = BPF_PROG_TYPE_SOCKET_FILTER;
		memcpy(fp->insnsi, fptr, fp->len * sizeof(struct bpf_insn));
		fp->aux->stack_depth = tests[which].stack_depth;

		/* We cannot error here as we don't need type compatibility
		 * checks.
		 */
		fp = bpf_prog_select_runtime(fp, err);
		if (*err) {
			pr_cont("FAIL to select_runtime err=%d\n", *err);
			return NULL;
		}
			if (err == 0) {
				pass_cnt++;
				continue;
			}
			err_cnt++;
			continue;
		}

		pr_cont("jited:%u ", fp->jited);

		run_cnt++;
		if (fp->jited)
			jit_cnt++;

		err = run_one(fp, &tests[i]);
		release_filter(fp, i);

		if (err) {
			pr_cont("FAIL (%d times)\n", err);
			err_cnt++;
		} else {
			pr_cont("PASS\n");
			pass_cnt++;
		}
	}

	pr_info("Summary: %d PASSED, %d FAILED, [%d/%d JIT'ed]\n",
		pass_cnt, err_cnt, jit_cnt, run_cnt);

	return err_cnt ? -EINVAL : 0;
}

static int __init test_bpf_init(void)
{
	int ret;

	ret = prepare_bpf_tests();
	if (ret < 0)
		return ret;

	ret = test_bpf();

	destroy_bpf_tests();
	return ret;
}

static void __exit test_bpf_exit(void)
{
}

module_init(test_bpf_init);
module_exit(test_bpf_exit);

MODULE_LICENSE("GPL");