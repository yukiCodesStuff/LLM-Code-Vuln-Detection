	} else {
		resp->lid = ib_lid_cpu16(attr->lid);
		resp->sm_lid = ib_lid_cpu16(attr->sm_lid);
	}

	resp->lmc = attr->lmc;
	resp->max_vl_num = attr->max_vl_num;
	resp->sm_sl = attr->sm_sl;
	resp->subnet_timeout = attr->subnet_timeout;
	resp->init_type_reply = attr->init_type_reply;
	resp->active_width = attr->active_width;
	resp->active_speed = attr->active_speed;
	resp->phys_state = attr->phys_state;
	resp->link_layer = rdma_port_get_link_layer(ib_dev, port_num);
}

static int UVERBS_HANDLER(UVERBS_METHOD_QUERY_PORT)(
	struct uverbs_attr_bundle *attrs)
{
	struct ib_device *ib_dev = attrs->ufile->device->ib_dev;
	struct ib_port_attr attr = {};
	struct ib_uverbs_query_port_resp_ex resp = {};
	int ret;
	u8 port_num;

	/* FIXME: Extend the UAPI_DEF_OBJ_NEEDS_FN stuff.. */
	if (!ib_dev->ops.query_port)
		return -EOPNOTSUPP;

	ret = uverbs_get_const(&port_num, attrs,
			       UVERBS_ATTR_QUERY_PORT_PORT_NUM);
	if (ret)
		return ret;

	ret = ib_query_port(ib_dev, port_num, &attr);
	if (ret)
		return ret;

	copy_port_attr_to_resp(&attr, &resp.legacy_resp, ib_dev, port_num);
	resp.port_cap_flags2 = attr.port_cap_flags2;

	return uverbs_copy_to_struct_or_zero(attrs, UVERBS_ATTR_QUERY_PORT_RESP,
					     &resp, sizeof(resp));
}