	case BPF_LIRC_MODE2:
		ptype = BPF_PROG_TYPE_LIRC_MODE2;
		break;
	default:
		return -EINVAL;
	}

	case BPF_PROG_TYPE_LIRC_MODE2:
		ret = lirc_prog_attach(attr, prog);
		break;
	default:
		ret = cgroup_bpf_prog_attach(attr, ptype, prog);
	}

		return sockmap_get_from_fd(attr, BPF_PROG_TYPE_SK_SKB, NULL);
	case BPF_LIRC_MODE2:
		return lirc_prog_detach(attr);
	default:
		return -EINVAL;
	}
