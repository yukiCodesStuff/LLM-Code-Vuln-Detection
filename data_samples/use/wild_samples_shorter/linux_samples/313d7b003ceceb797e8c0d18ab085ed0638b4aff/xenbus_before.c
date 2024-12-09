		if (dev->state == XenbusStateConnected)
			break;

		err = connect_ring(be);
		if (err)
			break;
		update_blkif_status(be->blkif);
			break;
		/* fall through if not online */
	case XenbusStateUnknown:
		device_unregister(&dev->dev);
		break;

	default: