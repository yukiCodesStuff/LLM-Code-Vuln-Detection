		NAPI_GRO_CB(skb)->flush = 0;
		NAPI_GRO_CB(skb)->free = 0;
		NAPI_GRO_CB(skb)->encap_mark = 0;
		NAPI_GRO_CB(skb)->is_fou = 0;
		NAPI_GRO_CB(skb)->is_atomic = 1;
		NAPI_GRO_CB(skb)->gro_remcsum_start = 0;
