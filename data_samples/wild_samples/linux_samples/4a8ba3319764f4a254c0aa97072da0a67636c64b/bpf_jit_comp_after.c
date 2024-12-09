				if (unlikely(proglen + ilen > oldproglen)) {
					pr_err("bpb_jit_compile fatal error\n");
					kfree(addrs);
					module_memfree(image);
					return;
				}
				memcpy(image + proglen, temp, ilen);
			}
			proglen += ilen;
			addrs[i] = proglen;
			prog = temp;
		}
		/* last bpf instruction is always a RET :
		 * use it to give the cleanup instruction(s) addr
		 */
		cleanup_addr = proglen - 8; /* jmpl; mov r_A,%o0; */
		if (seen_or_pass0 & SEEN_MEM)
			cleanup_addr -= 4; /* add %sp, X, %sp; */

		if (image) {
			if (proglen != oldproglen)
				pr_err("bpb_jit_compile proglen=%u != oldproglen=%u\n",
				       proglen, oldproglen);
			break;
		}
		if (proglen == oldproglen) {
			image = module_alloc(proglen);
			if (!image)
				goto out;
		}
		oldproglen = proglen;
	}

	if (bpf_jit_enable > 1)
		bpf_jit_dump(flen, proglen, pass, image);

	if (image) {
		bpf_flush_icache(image, image + proglen);
		fp->bpf_func = (void *)image;
		fp->jited = true;
	}
out:
	kfree(addrs);
	return;
}

void bpf_jit_free(struct bpf_prog *fp)
{
	if (fp->jited)
		module_memfree(fp->bpf_func);

	bpf_prog_unlock_free(fp);
}
{
	if (fp->jited)
		module_memfree(fp->bpf_func);

	bpf_prog_unlock_free(fp);
}