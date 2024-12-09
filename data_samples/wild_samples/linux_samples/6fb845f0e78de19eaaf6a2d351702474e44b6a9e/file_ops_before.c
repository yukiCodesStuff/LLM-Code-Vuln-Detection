		for (i = 0 ; i < uctxt->egrbufs.numbufs; i++) {
			memlen = uctxt->egrbufs.buffers[i].len;
			memvirt = uctxt->egrbufs.buffers[i].addr;
			ret = remap_pfn_range(
				vma, addr,
				/*
				 * virt_to_pfn() does the same, but
				 * it's not available on x86_64
				 * when CONFIG_MMU is enabled.
				 */
				PFN_DOWN(__pa(memvirt)),
				memlen,
				vma->vm_page_prot);
			if (ret < 0)
				goto done;
			addr += memlen;
		}
		ret = 0;
		goto done;
	}
	case UREGS:
		/*
		 * Map only the page that contains this context's user
		 * registers.
		 */
		memaddr = (unsigned long)
			(dd->physaddr + RXE_PER_CONTEXT_USER)
			+ (uctxt->ctxt * RXE_PER_CONTEXT_SIZE);
		/*
		 * TidFlow table is on the same page as the rest of the
		 * user registers.
		 */
		memlen = PAGE_SIZE;
		flags |= VM_DONTCOPY | VM_DONTEXPAND;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		mapio = 1;
		break;
	case EVENTS:
		/*
		 * Use the page where this context's flags are. User level
		 * knows where it's own bitmap is within the page.
		 */
		memaddr = (unsigned long)
			(dd->events + uctxt_offset(uctxt)) & PAGE_MASK;
		memlen = PAGE_SIZE;
		/*
		 * v3.7 removes VM_RESERVED but the effect is kept by
		 * using VM_IO.
		 */
		flags |= VM_IO | VM_DONTEXPAND;
		vmf = 1;
		break;
	case STATUS:
		if (flags & (unsigned long)(VM_WRITE | VM_EXEC)) {
			ret = -EPERM;
			goto done;
		}