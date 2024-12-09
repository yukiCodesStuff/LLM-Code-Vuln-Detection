		    (hdr->magic != SCSI_NL_MAGIC)) {
			err = -EPROTOTYPE;
			goto next_msg;
		}

		if (!netlink_capable(skb, CAP_SYS_ADMIN)) {
			err = -EPERM;
			goto next_msg;
		}

		if (nlh->nlmsg_len < (sizeof(*nlh) + hdr->msglen)) {
			printk(KERN_WARNING "%s: discarding partial message\n",
				 __func__);
			goto next_msg;
		}

		/*
		 * Deliver message to the appropriate transport
		 */
		tport = hdr->transport;
		if (tport == SCSI_NL_TRANSPORT) {
			switch (hdr->msgtype) {
			case SCSI_NL_SHOST_VENDOR:
				/* Locate the driver that corresponds to the message */
				err = -ESRCH;
				break;
			default:
				err = -EBADR;
				break;
			}