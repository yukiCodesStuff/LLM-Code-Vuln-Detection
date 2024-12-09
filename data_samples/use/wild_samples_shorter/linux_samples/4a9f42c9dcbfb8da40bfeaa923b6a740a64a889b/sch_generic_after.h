struct qdisc_walker;
struct tcf_walker;
struct module;
struct bpf_flow_keys;

typedef int tc_setup_cb_t(enum tc_setup_type type,
			  void *type_data, void *cb_priv);

};

struct qdisc_skb_cb {
	union {
		struct {
			unsigned int		pkt_len;
			u16			slave_dev_queue_mapping;
			u16			tc_classid;
		};
		struct bpf_flow_keys *flow_keys;
	};
#define QDISC_CB_PRIV_LEN 20
	unsigned char		data[QDISC_CB_PRIV_LEN];
};
