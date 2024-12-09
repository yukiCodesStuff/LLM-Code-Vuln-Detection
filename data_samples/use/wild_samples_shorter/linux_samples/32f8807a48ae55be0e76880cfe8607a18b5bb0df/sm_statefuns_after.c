					void *arg,
					struct sctp_cmd_seq *commands);

static enum sctp_disposition
__sctp_sf_do_9_2_reshutack(struct net *net, const struct sctp_endpoint *ep,
			   const struct sctp_association *asoc,
			   const union sctp_subtype type, void *arg,
			   struct sctp_cmd_seq *commands);

/* Small helper function that checks if the chunk length
 * is of the appropriate length.  The 'required_length' argument
 * is set to be the size of a specific chunk we are testing.
 * Return Values:  true  = Valid length
	if (!chunk->singleton)
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* Make sure that the INIT chunk has a valid length.
	 * Normally, this would cause an ABORT with a Protocol Violation
	 * error, but since we don't have an association, we'll
	 * just discard the packet.
	 */
	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_init_chunk)))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* If the packet is an OOTB packet which is temporarily on the
	 * control endpoint, respond with an ABORT.
	 */
	if (ep == sctp_sk(net->sctp.ctl_sock)->ep) {
	if (chunk->sctp_hdr->vtag != 0)
		return sctp_sf_tabort_8_4_8(net, ep, asoc, type, arg, commands);

	/* If the INIT is coming toward a closing socket, we'll send back
	 * and ABORT.  Essentially, this catches the race of INIT being
	 * backloged to the socket at the same time as the user issues close().
	 * Since the socket and all its associations are going away, we
	struct sock *sk;
	int error = 0;

	if (asoc && !sctp_vtag_verify(chunk, asoc))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* If the packet is an OOTB packet which is temporarily on the
	 * control endpoint, respond with an ABORT.
	 */
	if (ep == sctp_sk(net->sctp.ctl_sock)->ep) {
	 * in sctp_unpack_cookie().
	 */
	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_chunkhdr)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);

	/* If the endpoint is not listening or if the number of associations
	 * on the TCP-style socket exceed the max backlog, respond with an
	 * ABORT.
	if (!chunk->singleton)
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* Make sure that the INIT chunk has a valid length. */
	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_init_chunk)))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* 3.1 A packet containing an INIT chunk MUST have a zero Verification
	 * Tag.
	 */
	if (chunk->sctp_hdr->vtag != 0)
		return sctp_sf_tabort_8_4_8(net, ep, asoc, type, arg, commands);

	if (SCTP_INPUT_CB(chunk->skb)->encap_port != chunk->transport->encap_port)
		return sctp_sf_new_encap_port(net, ep, asoc, type, arg, commands);

	/* Grab the INIT header.  */
	 * its peer.
	*/
	if (sctp_state(asoc, SHUTDOWN_ACK_SENT)) {
		disposition = __sctp_sf_do_9_2_reshutack(net, ep, asoc,
							 SCTP_ST_CHUNK(chunk->chunk_hdr->type),
							 chunk, commands);
		if (SCTP_DISPOSITION_NOMEM == disposition)
			goto nomem;

		err = sctp_make_op_error(asoc, chunk,
	 * enough for the chunk header.  Cookie length verification is
	 * done later.
	 */
	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_chunkhdr))) {
		if (!sctp_vtag_verify(chunk, asoc))
			asoc = NULL;
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg, commands);
	}

	/* "Decode" the chunk.  We have no optional parameters so we
	 * are in good shape.
	 */
	 */
	if (SCTP_ADDR_DEL ==
		    sctp_bind_addr_state(&asoc->base.bind_addr, &chunk->dest))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	if (!sctp_err_chunk_valid(chunk))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	 */
	if (SCTP_ADDR_DEL ==
		    sctp_bind_addr_state(&asoc->base.bind_addr, &chunk->dest))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	if (!sctp_err_chunk_valid(chunk))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	 */
	if (SCTP_ADDR_DEL ==
		    sctp_bind_addr_state(&asoc->base.bind_addr, &chunk->dest))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	if (!sctp_err_chunk_valid(chunk))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

 * that belong to this association, it should discard the INIT chunk and
 * retransmit the SHUTDOWN ACK chunk.
 */
static enum sctp_disposition
__sctp_sf_do_9_2_reshutack(struct net *net, const struct sctp_endpoint *ep,
			   const struct sctp_association *asoc,
			   const union sctp_subtype type, void *arg,
			   struct sctp_cmd_seq *commands)
{
	struct sctp_chunk *chunk = arg;
	struct sctp_chunk *reply;

	return SCTP_DISPOSITION_NOMEM;
}

enum sctp_disposition
sctp_sf_do_9_2_reshutack(struct net *net, const struct sctp_endpoint *ep,
			 const struct sctp_association *asoc,
			 const union sctp_subtype type, void *arg,
			 struct sctp_cmd_seq *commands)
{
	struct sctp_chunk *chunk = arg;

	if (!chunk->singleton)
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_init_chunk)))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	if (chunk->sctp_hdr->vtag != 0)
		return sctp_sf_tabort_8_4_8(net, ep, asoc, type, arg, commands);

	return __sctp_sf_do_9_2_reshutack(net, ep, asoc, type, arg, commands);
}

/*
 * sctp_sf_do_ecn_cwr
 *
 * Section:  Appendix A: Explicit Congestion Notification

	SCTP_INC_STATS(net, SCTP_MIB_OUTOFBLUES);

	if (asoc && !sctp_vtag_verify(chunk, asoc))
		asoc = NULL;

	ch = (struct sctp_chunkhdr *)chunk->chunk_hdr;
	do {
		/* Report violation if the chunk is less then minimal */
		if (ntohs(ch->length) < sizeof(*ch))

	SCTP_INC_STATS(net, SCTP_MIB_OUTCTRLCHUNKS);

	/* We need to discard the rest of the packet to prevent
	 * potential boomming attacks from additional bundled chunks.
	 * This is documented in SCTP Threats ID.
	 */
{
	struct sctp_chunk *chunk = arg;

	if (!sctp_vtag_verify(chunk, asoc))
		asoc = NULL;

	/* Make sure that the SHUTDOWN_ACK chunk has a valid length. */
	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_chunkhdr)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
	}

	/* Make sure that the ASCONF ADDIP chunk has a valid length.  */
	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_addip_chunk)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);

	/* ADD-IP: Section 4.1.1
	 * This chunk MUST be sent in an authenticated way by using
	 * the mechanism defined in [I-D.ietf-tsvwg-sctp-auth]. If this chunk
	 * is received unauthenticated it MUST be silently discarded as
	 */
	if (!asoc->peer.asconf_capable ||
	    (!net->sctp.addip_noauth && !chunk->auth))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	hdr = (struct sctp_addiphdr *)chunk->skb->data;
	serial = ntohl(hdr->serial);

		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
	}

	/* Make sure that the ADDIP chunk has a valid length.  */
	if (!sctp_chunk_length_valid(asconf_ack,
				     sizeof(struct sctp_addip_chunk)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);

	/* ADD-IP, Section 4.1.2:
	 * This chunk MUST be sent in an authenticated way by using
	 * the mechanism defined in [I-D.ietf-tsvwg-sctp-auth]. If this chunk
	 * is received unauthenticated it MUST be silently discarded as
	 */
	if (!asoc->peer.asconf_capable ||
	    (!net->sctp.addip_noauth && !asconf_ack->auth))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	addip_hdr = (struct sctp_addiphdr *)asconf_ack->skb->data;
	rcvd_serial = ntohl(addip_hdr->serial);

{
	struct sctp_chunk *chunk = arg;

	if (asoc && !sctp_vtag_verify(chunk, asoc))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* Make sure that the chunk has a valid length.
	 * Since we don't know the chunk type, we use a general
	 * chunkhdr structure to make a comparison.
	 */
{
	struct sctp_chunk *chunk = arg;

	if (!sctp_vtag_verify(chunk, asoc))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* Make sure that the chunk has a valid length. */
	if (!sctp_chunk_length_valid(chunk, sizeof(struct sctp_chunkhdr)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);
		 * yet.
		 */
		switch (chunk->chunk_hdr->type) {
		case SCTP_CID_INIT:
		case SCTP_CID_INIT_ACK:
		{
			struct sctp_initack_chunk *initack;
