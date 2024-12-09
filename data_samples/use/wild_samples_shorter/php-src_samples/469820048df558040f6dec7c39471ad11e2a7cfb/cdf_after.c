		    i, inp[i].pi_id, inp[i].pi_type, q - p, offs));
		if (inp[i].pi_type & CDF_VECTOR) {
			nelements = CDF_GETUINT32(q, 1);
			if (nelements > CDF_ELEMENT_LIMIT || nelements == 0) {
				DPRINTF(("CDF_VECTOR with nelements == %"
				    SIZE_T_FORMAT "u\n", nelements));
				goto out;
			}
			o = 2;
		} else {
				*info = inp;
				inp = *info + nelem;
			}
			for (j = 0; j < nelements && i < sh.sh_properties;
			    j++, i++)
			{
				uint32_t l = CDF_GETUINT32(q, o);