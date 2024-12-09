		}
	}

	if (trans == NULL) {
		kmem_cache_free(rds_conn_slab, conn);
		conn = ERR_PTR(-ENODEV);
		goto out;
	}

	conn->c_trans = trans;

	ret = trans->conn_alloc(conn, gfp);
	if (ret) {