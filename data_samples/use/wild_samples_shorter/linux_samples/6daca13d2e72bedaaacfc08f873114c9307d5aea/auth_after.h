	/* ensure that an existing authorizer is up to date */
	int (*update_authorizer)(struct ceph_auth_client *ac, int peer_type,
				 struct ceph_auth_handshake *auth);
	int (*add_authorizer_challenge)(struct ceph_auth_client *ac,
					struct ceph_authorizer *a,
					void *challenge_buf,
					int challenge_buf_len);
	int (*verify_authorizer_reply)(struct ceph_auth_client *ac,
				       struct ceph_authorizer *a);
	void (*invalidate_authorizer)(struct ceph_auth_client *ac,
				      int peer_type);
extern int ceph_auth_update_authorizer(struct ceph_auth_client *ac,
				       int peer_type,
				       struct ceph_auth_handshake *a);
int ceph_auth_add_authorizer_challenge(struct ceph_auth_client *ac,
				       struct ceph_authorizer *a,
				       void *challenge_buf,
				       int challenge_buf_len);
extern int ceph_auth_verify_authorizer_reply(struct ceph_auth_client *ac,
					     struct ceph_authorizer *a);
extern void ceph_auth_invalidate_authorizer(struct ceph_auth_client *ac,
					    int peer_type);