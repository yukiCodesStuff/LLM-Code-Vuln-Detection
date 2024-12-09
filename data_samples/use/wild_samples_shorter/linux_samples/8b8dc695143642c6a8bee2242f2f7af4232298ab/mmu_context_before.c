
void slb_setup_new_exec(void);

static int hash__init_new_context(struct mm_struct *mm)
{
	int index;

	index = hash__alloc_context_id();
	if (index < 0)
		return index;

	mm->context.hash_context = kmalloc(sizeof(struct hash_mm_context),
					   GFP_KERNEL);
	if (!mm->context.hash_context) {
		ida_free(&mmu_context_ida, index);
		return -ENOMEM;
	}

	/*
	 * The old code would re-promote on fork, we don't do that when using
	 * slices as it could cause problem promoting slices that have been
			mm->context.hash_context->spt = kmalloc(sizeof(struct subpage_prot_table),
								GFP_KERNEL);
			if (!mm->context.hash_context->spt) {
				ida_free(&mmu_context_ida, index);
				kfree(mm->context.hash_context);
				return -ENOMEM;
			}
		}
#endif

	}

	pkey_mm_init(mm);
	return index;