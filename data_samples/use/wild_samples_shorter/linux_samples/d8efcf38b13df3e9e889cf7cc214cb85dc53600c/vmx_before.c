		else if (is_page_fault(intr_info))
			return enable_ept;
		else if (is_no_device(intr_info) &&
			 !(nested_read_cr0(vmcs12) & X86_CR0_TS))
			return 0;
		return vmcs12->exception_bitmap &
				(1u << (intr_info & INTR_INFO_VECTOR_MASK));
	case EXIT_REASON_EXTERNAL_INTERRUPT: