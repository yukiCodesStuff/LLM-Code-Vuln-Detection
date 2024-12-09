					  (struct floppy_struct **)&outparam);
		if (ret)
			return ret;
		break;
	case FDMSGON:
		UDP->flags |= FTD_MSG;
		return 0;