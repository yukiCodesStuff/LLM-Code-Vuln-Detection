			val &= 0xffff;
		}
		vj = slhc_init(val2+1, val+1);
		if (!vj) {
			netdev_err(ppp->dev,
				   "PPP: no memory (VJ compressor)\n");
			err = -ENOMEM;
			break;
		}
		ppp_lock(ppp);
		if (ppp->vj)