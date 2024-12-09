
		if (!vcc->dev || !test_bit(ATM_VF_ADDR, &vcc->flags))
			return -ENOTCONN;
		pvc.sap_family = AF_ATMPVC;
		pvc.sap_addr.itf = vcc->dev->number;
		pvc.sap_addr.vpi = vcc->vpi;
		pvc.sap_addr.vci = vcc->vci;