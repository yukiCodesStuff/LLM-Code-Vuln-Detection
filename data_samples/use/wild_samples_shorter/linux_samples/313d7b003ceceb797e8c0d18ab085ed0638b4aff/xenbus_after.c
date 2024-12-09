		if (dev->state == XenbusStateConnected)
			break;

		/* Enforce precondition before potential leak point.
		 * blkif_disconnect() is idempotent.
		 */
		blkif_disconnect(be->blkif);

		err = connect_ring(be);
		if (err)
			break;
		update_blkif_status(be->blkif);
			break;
		/* fall through if not online */
	case XenbusStateUnknown:
		/* implies blkif_disconnect() via blkback_remove() */
		device_unregister(&dev->dev);
		break;

	default: