		vmf = 1;
		break;
	case STATUS:
		if (flags & (unsigned long)(VM_WRITE | VM_EXEC)) {
			ret = -EPERM;
			goto done;
		}
		memaddr = kvirt_to_phys((void *)dd->status);