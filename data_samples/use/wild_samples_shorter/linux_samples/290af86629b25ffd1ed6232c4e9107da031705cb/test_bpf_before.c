				return NULL;
			}
		}
		/* We don't expect to fail. */
		if (*err) {
			pr_cont("FAIL to attach err=%d len=%d\n",
				*err, fprog.len);
			return NULL;
		}
		break;
		 * checks.
		 */
		fp = bpf_prog_select_runtime(fp, err);
		break;
	}

	*err = 0;
				pass_cnt++;
				continue;
			}

			return err;
		}

		pr_cont("jited:%u ", fp->jited);
