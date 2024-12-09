		vmf = 1;
		break;
	case STATUS:
		if (flags & VM_WRITE) {
			ret = -EPERM;
			goto done;
		}
		memaddr = kvirt_to_phys((void *)dd->status);