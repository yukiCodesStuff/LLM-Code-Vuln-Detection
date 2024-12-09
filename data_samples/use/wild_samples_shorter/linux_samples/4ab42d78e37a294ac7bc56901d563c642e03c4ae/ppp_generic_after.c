			val &= 0xffff;
		}
		vj = slhc_init(val2+1, val+1);
		if (IS_ERR(vj)) {
			err = PTR_ERR(vj);
			break;
		}
		ppp_lock(ppp);
		if (ppp->vj)