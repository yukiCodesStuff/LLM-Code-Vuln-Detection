typedef struct {
  /**
   * An arbitrary user supplied data.  This is passed to each
   * allocator function.
   */
  void *mem_user_data;
  /**
   * Custom allocator function to replace malloc().
   */
  nghttp2_malloc malloc;
  /**
   * Custom allocator function to replace free().
   */
  nghttp2_free free;
  /**
   * Custom allocator function to replace calloc().
   */
  nghttp2_calloc calloc;
  /**
   * Custom allocator function to replace realloc().
   */
  nghttp2_realloc realloc;
} nghttp2_mem;

struct nghttp2_option;

/**
 * @struct
 *
 * Configuration options for :type:`nghttp2_session`.  The details of
 * this structure are intentionally hidden from the public API.
 */
typedef struct nghttp2_option nghttp2_option;

/**
 * @function
 *
 * Initializes |*option_ptr| with default values.
 *
 * When the application finished using this object, it can use
 * `nghttp2_option_del()` to free its memory.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_option_new(nghttp2_option **option_ptr);

/**
 * @function
 *
 * Frees any resources allocated for |option|.  If |option| is
 * ``NULL``, this function does nothing.
 */
NGHTTP2_EXTERN void nghttp2_option_del(nghttp2_option *option);

/**
 * @function
 *
 * This option prevents the library from sending WINDOW_UPDATE for a
 * connection automatically.  If this option is set to nonzero, the
 * library won't send WINDOW_UPDATE for DATA until application calls
 * `nghttp2_session_consume()` to indicate the consumed amount of
 * data.  Don't use `nghttp2_submit_window_update()` for this purpose.
 * By default, this option is set to zero.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_no_auto_window_update(nghttp2_option *option, int val);

/**
 * @function
 *
 * This option sets the SETTINGS_MAX_CONCURRENT_STREAMS value of
 * remote endpoint as if it is received in SETTINGS frame.  Without
 * specifying this option, the maximum number of outgoing concurrent
 * streams is initially limited to 100 to avoid issues when the local
 * endpoint submits lots of requests before receiving initial SETTINGS
 * frame from the remote endpoint, since sending them at once to the
 * remote endpoint could lead to rejection of some of the requests.
 * This value will be overwritten when the local endpoint receives
 * initial SETTINGS frame from the remote endpoint, either to the
 * value advertised in SETTINGS_MAX_CONCURRENT_STREAMS or to the
 * default value (unlimited) if none was advertised.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_peer_max_concurrent_streams(nghttp2_option *option,
                                               uint32_t val);

/**
 * @function
 *
 * By default, nghttp2 library, if configured as server, requires
 * first 24 bytes of client magic byte string (MAGIC).  In most cases,
 * this will simplify the implementation of server.  But sometimes
 * server may want to detect the application protocol based on first
 * few bytes on clear text communication.
 *
 * If this option is used with nonzero |val|, nghttp2 library does not
 * handle MAGIC.  It still checks following SETTINGS frame.  This
 * means that applications should deal with MAGIC by themselves.
 *
 * If this option is not used or used with zero value, if MAGIC does
 * not match :macro:`NGHTTP2_CLIENT_MAGIC`, `nghttp2_session_recv()`
 * and `nghttp2_session_mem_recv()` will return error
 * :enum:`NGHTTP2_ERR_BAD_CLIENT_MAGIC`, which is fatal error.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_no_recv_client_magic(nghttp2_option *option, int val);

/**
 * @function
 *
 * By default, nghttp2 library enforces subset of HTTP Messaging rules
 * described in `HTTP/2 specification, section 8
 * <https://tools.ietf.org/html/rfc7540#section-8>`_.  See
 * :ref:`http-messaging` section for details.  For those applications
 * who use nghttp2 library as non-HTTP use, give nonzero to |val| to
 * disable this enforcement.  Please note that disabling this feature
 * does not change the fundamental client and server model of HTTP.
 * That is, even if the validation is disabled, only client can send
 * requests.
 */
NGHTTP2_EXTERN void nghttp2_option_set_no_http_messaging(nghttp2_option *option,
                                                         int val);

/**
 * @function
 *
 * RFC 7540 does not enforce any limit on the number of incoming
 * reserved streams (in RFC 7540 terms, streams in reserved (remote)
 * state).  This only affects client side, since only server can push
 * streams.  Malicious server can push arbitrary number of streams,
 * and make client's memory exhausted.  This option can set the
 * maximum number of such incoming streams to avoid possible memory
 * exhaustion.  If this option is set, and pushed streams are
 * automatically closed on reception, without calling user provided
 * callback, if they exceed the given limit.  The default value is
 * 200.  If session is configured as server side, this option has no
 * effect.  Server can control the number of streams to push.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_max_reserved_remote_streams(nghttp2_option *option,
                                               uint32_t val);

/**
 * @function
 *
 * Sets extension frame type the application is willing to handle with
 * user defined callbacks (see
 * :type:`nghttp2_on_extension_chunk_recv_callback` and
 * :type:`nghttp2_unpack_extension_callback`).  The |type| is
 * extension frame type, and must be strictly greater than 0x9.
 * Otherwise, this function does nothing.  The application can call
 * this function multiple times to set more than one frame type to
 * receive.  The application does not have to call this function if it
 * just sends extension frames.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_user_recv_extension_type(nghttp2_option *option,
                                            uint8_t type);

/**
 * @function
 *
 * Sets extension frame type the application is willing to receive
 * using builtin handler.  The |type| is the extension frame type to
 * receive, and must be strictly greater than 0x9.  Otherwise, this
 * function does nothing.  The application can call this function
 * multiple times to set more than one frame type to receive.  The
 * application does not have to call this function if it just sends
 * extension frames.
 *
 * If same frame type is passed to both
 * `nghttp2_option_set_builtin_recv_extension_type()` and
 * `nghttp2_option_set_user_recv_extension_type()`, the latter takes
 * precedence.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_builtin_recv_extension_type(nghttp2_option *option,
                                               uint8_t type);

/**
 * @function
 *
 * This option prevents the library from sending PING frame with ACK
 * flag set automatically when PING frame without ACK flag set is
 * received.  If this option is set to nonzero, the library won't send
 * PING frame with ACK flag set in the response for incoming PING
 * frame.  The application can send PING frame with ACK flag set using
 * `nghttp2_submit_ping()` with :enum:`NGHTTP2_FLAG_ACK` as flags
 * parameter.
 */
NGHTTP2_EXTERN void nghttp2_option_set_no_auto_ping_ack(nghttp2_option *option,
                                                        int val);

/**
 * @function
 *
 * This option sets the maximum length of header block (a set of
 * header fields per one HEADERS frame) to send.  The length of a
 * given set of header fields is calculated using
 * `nghttp2_hd_deflate_bound()`.  The default value is 64KiB.  If
 * application attempts to send header fields larger than this limit,
 * the transmission of the frame fails with error code
 * :enum:`NGHTTP2_ERR_FRAME_SIZE_ERROR`.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_max_send_header_block_length(nghttp2_option *option,
                                                size_t val);

/**
 * @function
 *
 * This option sets the maximum dynamic table size for deflating
 * header fields.  The default value is 4KiB.  In HTTP/2, receiver of
 * deflated header block can specify maximum dynamic table size.  The
 * actual maximum size is the minimum of the size receiver specified
 * and this option value.
 */
NGHTTP2_EXTERN void
nghttp2_option_set_max_deflate_dynamic_table_size(nghttp2_option *option,
                                                  size_t val);

/**
 * @function
 *
 * This option prevents the library from retaining closed streams to
 * maintain the priority tree.  If this option is set to nonzero,
 * applications can discard closed stream completely to save memory.
 */
NGHTTP2_EXTERN void nghttp2_option_set_no_closed_streams(nghttp2_option *option,
                                                         int val);

/**
 * @function
 *
 * This function sets the maximum number of outgoing SETTINGS ACK and
 * PING ACK frames retained in :type:`nghttp2_session` object.  If
 * more than those frames are retained, the peer is considered to be
 * misbehaving and session will be closed.  The default value is 1000.
 */
NGHTTP2_EXTERN void nghttp2_option_set_max_outbound_ack(nghttp2_option *option,
                                                        size_t val);

/**
 * @function
 *
 * Initializes |*session_ptr| for client use.  The all members of
 * |callbacks| are copied to |*session_ptr|.  Therefore |*session_ptr|
 * does not store |callbacks|.  The |user_data| is an arbitrary user
 * supplied data, which will be passed to the callback functions.
 *
 * The :type:`nghttp2_send_callback` must be specified.  If the
 * application code uses `nghttp2_session_recv()`, the
 * :type:`nghttp2_recv_callback` must be specified.  The other members
 * of |callbacks| can be ``NULL``.
 *
 * If this function fails, |*session_ptr| is left untouched.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int
nghttp2_session_client_new(nghttp2_session **session_ptr,
                           const nghttp2_session_callbacks *callbacks,
                           void *user_data);

/**
 * @function
 *
 * Initializes |*session_ptr| for server use.  The all members of
 * |callbacks| are copied to |*session_ptr|. Therefore |*session_ptr|
 * does not store |callbacks|.  The |user_data| is an arbitrary user
 * supplied data, which will be passed to the callback functions.
 *
 * The :type:`nghttp2_send_callback` must be specified.  If the
 * application code uses `nghttp2_session_recv()`, the
 * :type:`nghttp2_recv_callback` must be specified.  The other members
 * of |callbacks| can be ``NULL``.
 *
 * If this function fails, |*session_ptr| is left untouched.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int
nghttp2_session_server_new(nghttp2_session **session_ptr,
                           const nghttp2_session_callbacks *callbacks,
                           void *user_data);

/**
 * @function
 *
 * Like `nghttp2_session_client_new()`, but with additional options
 * specified in the |option|.
 *
 * The |option| can be ``NULL`` and the call is equivalent to
 * `nghttp2_session_client_new()`.
 *
 * This function does not take ownership |option|.  The application is
 * responsible for freeing |option| if it finishes using the object.
 *
 * The library code does not refer to |option| after this function
 * returns.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int
nghttp2_session_client_new2(nghttp2_session **session_ptr,
                            const nghttp2_session_callbacks *callbacks,
                            void *user_data, const nghttp2_option *option);

/**
 * @function
 *
 * Like `nghttp2_session_server_new()`, but with additional options
 * specified in the |option|.
 *
 * The |option| can be ``NULL`` and the call is equivalent to
 * `nghttp2_session_server_new()`.
 *
 * This function does not take ownership |option|.  The application is
 * responsible for freeing |option| if it finishes using the object.
 *
 * The library code does not refer to |option| after this function
 * returns.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int
nghttp2_session_server_new2(nghttp2_session **session_ptr,
                            const nghttp2_session_callbacks *callbacks,
                            void *user_data, const nghttp2_option *option);

/**
 * @function
 *
 * Like `nghttp2_session_client_new2()`, but with additional custom
 * memory allocator specified in the |mem|.
 *
 * The |mem| can be ``NULL`` and the call is equivalent to
 * `nghttp2_session_client_new2()`.
 *
 * This function does not take ownership |mem|.  The application is
 * responsible for freeing |mem|.
 *
 * The library code does not refer to |mem| pointer after this
 * function returns, so the application can safely free it.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_session_client_new3(
    nghttp2_session **session_ptr, const nghttp2_session_callbacks *callbacks,
    void *user_data, const nghttp2_option *option, nghttp2_mem *mem);

/**
 * @function
 *
 * Like `nghttp2_session_server_new2()`, but with additional custom
 * memory allocator specified in the |mem|.
 *
 * The |mem| can be ``NULL`` and the call is equivalent to
 * `nghttp2_session_server_new2()`.
 *
 * This function does not take ownership |mem|.  The application is
 * responsible for freeing |mem|.
 *
 * The library code does not refer to |mem| pointer after this
 * function returns, so the application can safely free it.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_session_server_new3(
    nghttp2_session **session_ptr, const nghttp2_session_callbacks *callbacks,
    void *user_data, const nghttp2_option *option, nghttp2_mem *mem);

/**
 * @function
 *
 * Frees any resources allocated for |session|.  If |session| is
 * ``NULL``, this function does nothing.
 */
NGHTTP2_EXTERN void nghttp2_session_del(nghttp2_session *session);

/**
 * @function
 *
 * Sends pending frames to the remote peer.
 *
 * This function retrieves the highest prioritized frame from the
 * outbound queue and sends it to the remote peer.  It does this as
 * many as possible until the user callback
 * :type:`nghttp2_send_callback` returns
 * :enum:`NGHTTP2_ERR_WOULDBLOCK` or the outbound queue becomes empty.
 * This function calls several callback functions which are passed
 * when initializing the |session|.  Here is the simple time chart
 * which tells when each callback is invoked:
 *
 * 1. Get the next frame to send from outbound queue.
 *
 * 2. Prepare transmission of the frame.
 *
 * 3. If the control frame cannot be sent because some preconditions
 *    are not met (e.g., request HEADERS cannot be sent after GOAWAY),
 *    :type:`nghttp2_on_frame_not_send_callback` is invoked.  Abort
 *    the following steps.
 *
 * 4. If the frame is HEADERS, PUSH_PROMISE or DATA,
 *    :type:`nghttp2_select_padding_callback` is invoked.
 *
 * 5. If the frame is request HEADERS, the stream is opened here.
 *
 * 6. :type:`nghttp2_before_frame_send_callback` is invoked.
 *
 * 7. If :enum:`NGHTTP2_ERR_CANCEL` is returned from
 *    :type:`nghttp2_before_frame_send_callback`, the current frame
 *    transmission is canceled, and
 *    :type:`nghttp2_on_frame_not_send_callback` is invoked.  Abort
 *    the following steps.
 *
 * 8. :type:`nghttp2_send_callback` is invoked one or more times to
 *    send the frame.
 *
 * 9. :type:`nghttp2_on_frame_send_callback` is invoked.
 *
 * 10. If the transmission of the frame triggers closure of the
 *     stream, the stream is closed and
 *     :type:`nghttp2_on_stream_close_callback` is invoked.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_CALLBACK_FAILURE`
 *     The callback function failed.
 */
NGHTTP2_EXTERN int nghttp2_session_send(nghttp2_session *session);

/**
 * @function
 *
 * Returns the serialized data to send.
 *
 * This function behaves like `nghttp2_session_send()` except that it
 * does not use :type:`nghttp2_send_callback` to transmit data.
 * Instead, it assigns the pointer to the serialized data to the
 * |*data_ptr| and returns its length.  The other callbacks are called
 * in the same way as they are in `nghttp2_session_send()`.
 *
 * If no data is available to send, this function returns 0.
 *
 * This function may not return all serialized data in one invocation.
 * To get all data, call this function repeatedly until it returns 0
 * or one of negative error codes.
 *
 * The assigned |*data_ptr| is valid until the next call of
 * `nghttp2_session_mem_send()` or `nghttp2_session_send()`.
 *
 * The caller must send all data before sending the next chunk of
 * data.
 *
 * This function returns the length of the data pointed by the
 * |*data_ptr| if it succeeds, or one of the following negative error
 * codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 *
 * .. note::
 *
 *   This function may produce very small byte string.  If that is the
 *   case, and application disables Nagle algorithm (``TCP_NODELAY``),
 *   then writing this small chunk leads to very small packet, and it
 *   is very inefficient.  An application should be responsible to
 *   buffer up small chunks of data as necessary to avoid this
 *   situation.
 */
NGHTTP2_EXTERN ssize_t nghttp2_session_mem_send(nghttp2_session *session,
                                                const uint8_t **data_ptr);

/**
 * @function
 *
 * Receives frames from the remote peer.
 *
 * This function receives as many frames as possible until the user
 * callback :type:`nghttp2_recv_callback` returns
 * :enum:`NGHTTP2_ERR_WOULDBLOCK`.  This function calls several
 * callback functions which are passed when initializing the
 * |session|.  Here is the simple time chart which tells when each
 * callback is invoked:
 *
 * 1. :type:`nghttp2_recv_callback` is invoked one or more times to
 *    receive frame header.
 *
 * 2. When frame header is received,
 *    :type:`nghttp2_on_begin_frame_callback` is invoked.
 *
 * 3. If the frame is DATA frame:
 *
 *    1. :type:`nghttp2_recv_callback` is invoked to receive DATA
 *       payload. For each chunk of data,
 *       :type:`nghttp2_on_data_chunk_recv_callback` is invoked.
 *
 *    2. If one DATA frame is completely received,
 *       :type:`nghttp2_on_frame_recv_callback` is invoked.  If the
 *       reception of the frame triggers the closure of the stream,
 *       :type:`nghttp2_on_stream_close_callback` is invoked.
 *
 * 4. If the frame is the control frame:
 *
 *    1. :type:`nghttp2_recv_callback` is invoked one or more times to
 *       receive whole frame.
 *
 *    2. If the received frame is valid, then following actions are
 *       taken.  If the frame is either HEADERS or PUSH_PROMISE,
 *       :type:`nghttp2_on_begin_headers_callback` is invoked.  Then
 *       :type:`nghttp2_on_header_callback` is invoked for each header
 *       name/value pair.  For invalid header field,
 *       :type:`nghttp2_on_invalid_header_callback` is called.  After
 *       all name/value pairs are emitted successfully,
 *       :type:`nghttp2_on_frame_recv_callback` is invoked.  For other
 *       frames, :type:`nghttp2_on_frame_recv_callback` is invoked.
 *       If the reception of the frame triggers the closure of the
 *       stream, :type:`nghttp2_on_stream_close_callback` is invoked.
 *
 *    3. If the received frame is unpacked but is interpreted as
 *       invalid, :type:`nghttp2_on_invalid_frame_recv_callback` is
 *       invoked.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_EOF`
 *     The remote peer did shutdown on the connection.
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_CALLBACK_FAILURE`
 *     The callback function failed.
 * :enum:`NGHTTP2_ERR_BAD_CLIENT_MAGIC`
 *     Invalid client magic was detected.  This error only returns
 *     when |session| was configured as server and
 *     `nghttp2_option_set_no_recv_client_magic()` is not used with
 *     nonzero value.
 * :enum:`NGHTTP2_ERR_FLOODED`
 *     Flooding was detected in this HTTP/2 session, and it must be
 *     closed.  This is most likely caused by misbehaviour of peer.
 */
NGHTTP2_EXTERN int nghttp2_session_recv(nghttp2_session *session);

/**
 * @function
 *
 * Processes data |in| as an input from the remote endpoint.  The
 * |inlen| indicates the number of bytes in the |in|.
 *
 * This function behaves like `nghttp2_session_recv()` except that it
 * does not use :type:`nghttp2_recv_callback` to receive data; the
 * |in| is the only data for the invocation of this function.  If all
 * bytes are processed, this function returns.  The other callbacks
 * are called in the same way as they are in `nghttp2_session_recv()`.
 *
 * In the current implementation, this function always tries to
 * processes all input data unless either an error occurs or
 * :enum:`NGHTTP2_ERR_PAUSE` is returned from
 * :type:`nghttp2_on_header_callback` or
 * :type:`nghttp2_on_data_chunk_recv_callback`.  If
 * :enum:`NGHTTP2_ERR_PAUSE` is used, the return value includes the
 * number of bytes which was used to produce the data or frame for the
 * callback.
 *
 * This function returns the number of processed bytes, or one of the
 * following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_CALLBACK_FAILURE`
 *     The callback function failed.
 * :enum:`NGHTTP2_ERR_BAD_CLIENT_MAGIC`
 *     Invalid client magic was detected.  This error only returns
 *     when |session| was configured as server and
 *     `nghttp2_option_set_no_recv_client_magic()` is not used with
 *     nonzero value.
 * :enum:`NGHTTP2_ERR_FLOODED`
 *     Flooding was detected in this HTTP/2 session, and it must be
 *     closed.  This is most likely caused by misbehaviour of peer.
 */
NGHTTP2_EXTERN ssize_t nghttp2_session_mem_recv(nghttp2_session *session,
                                                const uint8_t *in,
                                                size_t inlen);

/**
 * @function
 *
 * Puts back previously deferred DATA frame in the stream |stream_id|
 * to the outbound queue.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The stream does not exist; or no deferred data exist.
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_session_resume_data(nghttp2_session *session,
                                               int32_t stream_id);

/**
 * @function
 *
 * Returns nonzero value if |session| wants to receive data from the
 * remote peer.
 *
 * If both `nghttp2_session_want_read()` and
 * `nghttp2_session_want_write()` return 0, the application should
 * drop the connection.
 */
NGHTTP2_EXTERN int nghttp2_session_want_read(nghttp2_session *session);

/**
 * @function
 *
 * Returns nonzero value if |session| wants to send data to the remote
 * peer.
 *
 * If both `nghttp2_session_want_read()` and
 * `nghttp2_session_want_write()` return 0, the application should
 * drop the connection.
 */
NGHTTP2_EXTERN int nghttp2_session_want_write(nghttp2_session *session);

/**
 * @function
 *
 * Returns stream_user_data for the stream |stream_id|.  The
 * stream_user_data is provided by `nghttp2_submit_request()`,
 * `nghttp2_submit_headers()` or
 * `nghttp2_session_set_stream_user_data()`.  Unless it is set using
 * `nghttp2_session_set_stream_user_data()`, if the stream is
 * initiated by the remote endpoint, stream_user_data is always
 * ``NULL``.  If the stream does not exist, this function returns
 * ``NULL``.
 */
NGHTTP2_EXTERN void *
nghttp2_session_get_stream_user_data(nghttp2_session *session,
                                     int32_t stream_id);

/**
 * @function
 *
 * Sets the |stream_user_data| to the stream denoted by the
 * |stream_id|.  If a stream user data is already set to the stream,
 * it is replaced with the |stream_user_data|.  It is valid to specify
 * ``NULL`` in the |stream_user_data|, which nullifies the associated
 * data pointer.
 *
 * It is valid to set the |stream_user_data| to the stream reserved by
 * PUSH_PROMISE frame.
 *
 * This function returns 0 if it succeeds, or one of following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The stream does not exist
 */
NGHTTP2_EXTERN int
nghttp2_session_set_stream_user_data(nghttp2_session *session,
                                     int32_t stream_id, void *stream_user_data);

/**
 * @function
 *
 * Sets |user_data| to |session|, overwriting the existing user data
 * specified in `nghttp2_session_client_new()`, or
 * `nghttp2_session_server_new()`.
 */
NGHTTP2_EXTERN void nghttp2_session_set_user_data(nghttp2_session *session,
                                                  void *user_data);

/**
 * @function
 *
 * Returns the number of frames in the outbound queue.  This does not
 * include the deferred DATA frames.
 */
NGHTTP2_EXTERN size_t
nghttp2_session_get_outbound_queue_size(nghttp2_session *session);

/**
 * @function
 *
 * Returns the number of DATA payload in bytes received without
 * WINDOW_UPDATE transmission for the stream |stream_id|.  The local
 * (receive) window size can be adjusted by
 * `nghttp2_submit_window_update()`.  This function takes into account
 * that and returns effective data length.  In particular, if the
 * local window size is reduced by submitting negative
 * window_size_increment with `nghttp2_submit_window_update()`, this
 * function returns the number of bytes less than actually received.
 *
 * This function returns -1 if it fails.
 */
NGHTTP2_EXTERN int32_t nghttp2_session_get_stream_effective_recv_data_length(
    nghttp2_session *session, int32_t stream_id);

/**
 * @function
 *
 * Returns the local (receive) window size for the stream |stream_id|.
 * The local window size can be adjusted by
 * `nghttp2_submit_window_update()`.  This function takes into account
 * that and returns effective window size.
 *
 * This function does not take into account the amount of received
 * data from the remote endpoint.  Use
 * `nghttp2_session_get_stream_local_window_size()` to know the amount
 * of data the remote endpoint can send without receiving stream level
 * WINDOW_UPDATE frame.  Note that each stream is still subject to the
 * connection level flow control.
 *
 * This function returns -1 if it fails.
 */
NGHTTP2_EXTERN int32_t nghttp2_session_get_stream_effective_local_window_size(
    nghttp2_session *session, int32_t stream_id);

/**
 * @function
 *
 * Returns the amount of flow-controlled payload (e.g., DATA) that the
 * remote endpoint can send without receiving stream level
 * WINDOW_UPDATE frame.  It is also subject to the connection level
 * flow control.  So the actual amount of data to send is
 * min(`nghttp2_session_get_stream_local_window_size()`,
 * `nghttp2_session_get_local_window_size()`).
 *
 * This function returns -1 if it fails.
 */
NGHTTP2_EXTERN int32_t nghttp2_session_get_stream_local_window_size(
    nghttp2_session *session, int32_t stream_id);

/**
 * @function
 *
 * Returns the number of DATA payload in bytes received without
 * WINDOW_UPDATE transmission for a connection.  The local (receive)
 * window size can be adjusted by `nghttp2_submit_window_update()`.
 * This function takes into account that and returns effective data
 * length.  In particular, if the local window size is reduced by
 * submitting negative window_size_increment with
 * `nghttp2_submit_window_update()`, this function returns the number
 * of bytes less than actually received.
 *
 * This function returns -1 if it fails.
 */
NGHTTP2_EXTERN int32_t
nghttp2_session_get_effective_recv_data_length(nghttp2_session *session);

/**
 * @function
 *
 * Returns the local (receive) window size for a connection.  The
 * local window size can be adjusted by
 * `nghttp2_submit_window_update()`.  This function takes into account
 * that and returns effective window size.
 *
 * This function does not take into account the amount of received
 * data from the remote endpoint.  Use
 * `nghttp2_session_get_local_window_size()` to know the amount of
 * data the remote endpoint can send without receiving
 * connection-level WINDOW_UPDATE frame.  Note that each stream is
 * still subject to the stream level flow control.
 *
 * This function returns -1 if it fails.
 */
NGHTTP2_EXTERN int32_t
nghttp2_session_get_effective_local_window_size(nghttp2_session *session);

/**
 * @function
 *
 * Returns the amount of flow-controlled payload (e.g., DATA) that the
 * remote endpoint can send without receiving connection level
 * WINDOW_UPDATE frame.  Note that each stream is still subject to the
 * stream level flow control (see
 * `nghttp2_session_get_stream_local_window_size()`).
 *
 * This function returns -1 if it fails.
 */
NGHTTP2_EXTERN int32_t
nghttp2_session_get_local_window_size(nghttp2_session *session);

/**
 * @function
 *
 * Returns the remote window size for a given stream |stream_id|.
 *
 * This is the amount of flow-controlled payload (e.g., DATA) that the
 * local endpoint can send without stream level WINDOW_UPDATE.  There
 * is also connection level flow control, so the effective size of
 * payload that the local endpoint can actually send is
 * min(`nghttp2_session_get_stream_remote_window_size()`,
 * `nghttp2_session_get_remote_window_size()`).
 *
 * This function returns -1 if it fails.
 */
NGHTTP2_EXTERN int32_t nghttp2_session_get_stream_remote_window_size(
    nghttp2_session *session, int32_t stream_id);

/**
 * @function
 *
 * Returns the remote window size for a connection.
 *
 * This function always succeeds.
 */
NGHTTP2_EXTERN int32_t
nghttp2_session_get_remote_window_size(nghttp2_session *session);

/**
 * @function
 *
 * Returns 1 if local peer half closed the given stream |stream_id|.
 * Returns 0 if it did not.  Returns -1 if no such stream exists.
 */
NGHTTP2_EXTERN int
nghttp2_session_get_stream_local_close(nghttp2_session *session,
                                       int32_t stream_id);

/**
 * @function
 *
 * Returns 1 if remote peer half closed the given stream |stream_id|.
 * Returns 0 if it did not.  Returns -1 if no such stream exists.
 */
NGHTTP2_EXTERN int
nghttp2_session_get_stream_remote_close(nghttp2_session *session,
                                        int32_t stream_id);

/**
 * @function
 *
 * Returns the current dynamic table size of HPACK inflater, including
 * the overhead 32 bytes per entry described in RFC 7541.
 */
NGHTTP2_EXTERN size_t
nghttp2_session_get_hd_inflate_dynamic_table_size(nghttp2_session *session);

/**
 * @function
 *
 * Returns the current dynamic table size of HPACK deflater including
 * the overhead 32 bytes per entry described in RFC 7541.
 */
NGHTTP2_EXTERN size_t
nghttp2_session_get_hd_deflate_dynamic_table_size(nghttp2_session *session);

/**
 * @function
 *
 * Signals the session so that the connection should be terminated.
 *
 * The last stream ID is the minimum value between the stream ID of a
 * stream for which :type:`nghttp2_on_frame_recv_callback` was called
 * most recently and the last stream ID we have sent to the peer
 * previously.
 *
 * The |error_code| is the error code of this GOAWAY frame.  The
 * pre-defined error code is one of :enum:`nghttp2_error_code`.
 *
 * After the transmission, both `nghttp2_session_want_read()` and
 * `nghttp2_session_want_write()` return 0.
 *
 * This function should be called when the connection should be
 * terminated after sending GOAWAY.  If the remaining streams should
 * be processed after GOAWAY, use `nghttp2_submit_goaway()` instead.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_session_terminate_session(nghttp2_session *session,
                                                     uint32_t error_code);

/**
 * @function
 *
 * Signals the session so that the connection should be terminated.
 *
 * This function behaves like `nghttp2_session_terminate_session()`,
 * but the last stream ID can be specified by the application for fine
 * grained control of stream.  The HTTP/2 specification does not allow
 * last_stream_id to be increased.  So the actual value sent as
 * last_stream_id is the minimum value between the given
 * |last_stream_id| and the last_stream_id we have previously sent to
 * the peer.
 *
 * The |last_stream_id| is peer's stream ID or 0.  So if |session| is
 * initialized as client, |last_stream_id| must be even or 0.  If
 * |session| is initialized as server, |last_stream_id| must be odd or
 * 0.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |last_stream_id| is invalid.
 */
NGHTTP2_EXTERN int nghttp2_session_terminate_session2(nghttp2_session *session,
                                                      int32_t last_stream_id,
                                                      uint32_t error_code);

/**
 * @function
 *
 * Signals to the client that the server started graceful shutdown
 * procedure.
 *
 * This function is only usable for server.  If this function is
 * called with client side session, this function returns
 * :enum:`NGHTTP2_ERR_INVALID_STATE`.
 *
 * To gracefully shutdown HTTP/2 session, server should call this
 * function to send GOAWAY with last_stream_id (1u << 31) - 1.  And
 * after some delay (e.g., 1 RTT), send another GOAWAY with the stream
 * ID that the server has some processing using
 * `nghttp2_submit_goaway()`.  See also
 * `nghttp2_session_get_last_proc_stream_id()`.
 *
 * Unlike `nghttp2_submit_goaway()`, this function just sends GOAWAY
 * and does nothing more.  This is a mere indication to the client
 * that session shutdown is imminent.  The application should call
 * `nghttp2_submit_goaway()` with appropriate last_stream_id after
 * this call.
 *
 * If one or more GOAWAY frame have been already sent by either
 * `nghttp2_submit_goaway()` or `nghttp2_session_terminate_session()`,
 * this function has no effect.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     The |session| is initialized as client.
 */
NGHTTP2_EXTERN int nghttp2_submit_shutdown_notice(nghttp2_session *session);

/**
 * @function
 *
 * Returns the value of SETTINGS |id| notified by a remote endpoint.
 * The |id| must be one of values defined in
 * :enum:`nghttp2_settings_id`.
 */
NGHTTP2_EXTERN uint32_t nghttp2_session_get_remote_settings(
    nghttp2_session *session, nghttp2_settings_id id);

/**
 * @function
 *
 * Returns the value of SETTINGS |id| of local endpoint acknowledged
 * by the remote endpoint.  The |id| must be one of the values defined
 * in :enum:`nghttp2_settings_id`.
 */
NGHTTP2_EXTERN uint32_t nghttp2_session_get_local_settings(
    nghttp2_session *session, nghttp2_settings_id id);

/**
 * @function
 *
 * Tells the |session| that next stream ID is |next_stream_id|.  The
 * |next_stream_id| must be equal or greater than the value returned
 * by `nghttp2_session_get_next_stream_id()`.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |next_stream_id| is strictly less than the value
 *     `nghttp2_session_get_next_stream_id()` returns; or
 *     |next_stream_id| is invalid (e.g., even integer for client, or
 *     odd integer for server).
 */
NGHTTP2_EXTERN int nghttp2_session_set_next_stream_id(nghttp2_session *session,
                                                      int32_t next_stream_id);

/**
 * @function
 *
 * Returns the next outgoing stream ID.  Notice that return type is
 * uint32_t.  If we run out of stream ID for this session, this
 * function returns 1 << 31.
 */
NGHTTP2_EXTERN uint32_t
nghttp2_session_get_next_stream_id(nghttp2_session *session);

/**
 * @function
 *
 * Tells the |session| that |size| bytes for a stream denoted by
 * |stream_id| were consumed by application and are ready to
 * WINDOW_UPDATE.  The consumed bytes are counted towards both
 * connection and stream level WINDOW_UPDATE (see
 * `nghttp2_session_consume_connection()` and
 * `nghttp2_session_consume_stream()` to update consumption
 * independently).  This function is intended to be used without
 * automatic window update (see
 * `nghttp2_option_set_no_auto_window_update()`).
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0.
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     Automatic WINDOW_UPDATE is not disabled.
 */
NGHTTP2_EXTERN int nghttp2_session_consume(nghttp2_session *session,
                                           int32_t stream_id, size_t size);

/**
 * @function
 *
 * Like `nghttp2_session_consume()`, but this only tells library that
 * |size| bytes were consumed only for connection level.  Note that
 * HTTP/2 maintains connection and stream level flow control windows
 * independently.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     Automatic WINDOW_UPDATE is not disabled.
 */
NGHTTP2_EXTERN int nghttp2_session_consume_connection(nghttp2_session *session,
                                                      size_t size);

/**
 * @function
 *
 * Like `nghttp2_session_consume()`, but this only tells library that
 * |size| bytes were consumed only for stream denoted by |stream_id|.
 * Note that HTTP/2 maintains connection and stream level flow control
 * windows independently.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0.
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     Automatic WINDOW_UPDATE is not disabled.
 */
NGHTTP2_EXTERN int nghttp2_session_consume_stream(nghttp2_session *session,
                                                  int32_t stream_id,
                                                  size_t size);

/**
 * @function
 *
 * Changes priority of existing stream denoted by |stream_id|.  The
 * new priority specification is |pri_spec|.
 *
 * The priority is changed silently and instantly, and no PRIORITY
 * frame will be sent to notify the peer of this change.  This
 * function may be useful for server to change the priority of pushed
 * stream.
 *
 * If |session| is initialized as server, and ``pri_spec->stream_id``
 * points to the idle stream, the idle stream is created if it does
 * not exist.  The created idle stream will depend on root stream
 * (stream 0) with weight 16.
 *
 * Otherwise, if stream denoted by ``pri_spec->stream_id`` is not
 * found, we use default priority instead of given |pri_spec|.  That
 * is make stream depend on root stream with weight 16.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     Attempted to depend on itself; or no stream exist for the given
 *     |stream_id|; or |stream_id| is 0
 */
NGHTTP2_EXTERN int
nghttp2_session_change_stream_priority(nghttp2_session *session,
                                       int32_t stream_id,
                                       const nghttp2_priority_spec *pri_spec);

/**
 * @function
 *
 * Creates idle stream with the given |stream_id|, and priority
 * |pri_spec|.
 *
 * The stream creation is done without sending PRIORITY frame, which
 * means that peer does not know about the existence of this idle
 * stream in the local endpoint.
 *
 * RFC 7540 does not disallow the use of creation of idle stream with
 * odd or even stream ID regardless of client or server.  So this
 * function can create odd or even stream ID regardless of client or
 * server.  But probably it is a bit safer to use the stream ID the
 * local endpoint can initiate (in other words, use odd stream ID for
 * client, and even stream ID for server), to avoid potential
 * collision from peer's instruction.  Also we can use
 * `nghttp2_session_set_next_stream_id()` to avoid to open created
 * idle streams accidentally if we follow this recommendation.
 *
 * If |session| is initialized as server, and ``pri_spec->stream_id``
 * points to the idle stream, the idle stream is created if it does
 * not exist.  The created idle stream will depend on root stream
 * (stream 0) with weight 16.
 *
 * Otherwise, if stream denoted by ``pri_spec->stream_id`` is not
 * found, we use default priority instead of given |pri_spec|.  That
 * is make stream depend on root stream with weight 16.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     Attempted to depend on itself; or stream denoted by |stream_id|
 *     already exists; or |stream_id| cannot be used to create idle
 *     stream (in other words, local endpoint has already opened
 *     stream ID greater than or equal to the given stream ID; or
 *     |stream_id| is 0
 */
NGHTTP2_EXTERN int
nghttp2_session_create_idle_stream(nghttp2_session *session, int32_t stream_id,
                                   const nghttp2_priority_spec *pri_spec);

/**
 * @function
 *
 * Performs post-process of HTTP Upgrade request.  This function can
 * be called from both client and server, but the behavior is very
 * different in each other.
 *
 * .. warning::
 *
 *   This function is deprecated in favor of
 *   `nghttp2_session_upgrade2()`, because this function lacks the
 *   parameter to tell the library the request method used in the
 *   original HTTP request.  This information is required for client
 *   to validate actual response body length against content-length
 *   header field (see `nghttp2_option_set_no_http_messaging()`).  If
 *   HEAD is used in request, the length of response body must be 0
 *   regardless of value included in content-length header field.
 *
 * If called from client side, the |settings_payload| must be the
 * value sent in ``HTTP2-Settings`` header field and must be decoded
 * by base64url decoder.  The |settings_payloadlen| is the length of
 * |settings_payload|.  The |settings_payload| is unpacked and its
 * setting values will be submitted using `nghttp2_submit_settings()`.
 * This means that the client application code does not need to submit
 * SETTINGS by itself.  The stream with stream ID=1 is opened and the
 * |stream_user_data| is used for its stream_user_data.  The opened
 * stream becomes half-closed (local) state.
 *
 * If called from server side, the |settings_payload| must be the
 * value received in ``HTTP2-Settings`` header field and must be
 * decoded by base64url decoder.  The |settings_payloadlen| is the
 * length of |settings_payload|.  It is treated as if the SETTINGS
 * frame with that payload is received.  Thus, callback functions for
 * the reception of SETTINGS frame will be invoked.  The stream with
 * stream ID=1 is opened.  The |stream_user_data| is ignored.  The
 * opened stream becomes half-closed (remote).
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |settings_payload| is badly formed.
 * :enum:`NGHTTP2_ERR_PROTO`
 *     The stream ID 1 is already used or closed; or is not available.
 */
NGHTTP2_EXTERN int nghttp2_session_upgrade(nghttp2_session *session,
                                           const uint8_t *settings_payload,
                                           size_t settings_payloadlen,
                                           void *stream_user_data);

/**
 * @function
 *
 * Performs post-process of HTTP Upgrade request.  This function can
 * be called from both client and server, but the behavior is very
 * different in each other.
 *
 * If called from client side, the |settings_payload| must be the
 * value sent in ``HTTP2-Settings`` header field and must be decoded
 * by base64url decoder.  The |settings_payloadlen| is the length of
 * |settings_payload|.  The |settings_payload| is unpacked and its
 * setting values will be submitted using `nghttp2_submit_settings()`.
 * This means that the client application code does not need to submit
 * SETTINGS by itself.  The stream with stream ID=1 is opened and the
 * |stream_user_data| is used for its stream_user_data.  The opened
 * stream becomes half-closed (local) state.
 *
 * If called from server side, the |settings_payload| must be the
 * value received in ``HTTP2-Settings`` header field and must be
 * decoded by base64url decoder.  The |settings_payloadlen| is the
 * length of |settings_payload|.  It is treated as if the SETTINGS
 * frame with that payload is received.  Thus, callback functions for
 * the reception of SETTINGS frame will be invoked.  The stream with
 * stream ID=1 is opened.  The |stream_user_data| is ignored.  The
 * opened stream becomes half-closed (remote).
 *
 * If the request method is HEAD, pass nonzero value to
 * |head_request|.  Otherwise, pass 0.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |settings_payload| is badly formed.
 * :enum:`NGHTTP2_ERR_PROTO`
 *     The stream ID 1 is already used or closed; or is not available.
 */
NGHTTP2_EXTERN int nghttp2_session_upgrade2(nghttp2_session *session,
                                            const uint8_t *settings_payload,
                                            size_t settings_payloadlen,
                                            int head_request,
                                            void *stream_user_data);

/**
 * @function
 *
 * Serializes the SETTINGS values |iv| in the |buf|.  The size of the
 * |buf| is specified by |buflen|.  The number of entries in the |iv|
 * array is given by |niv|.  The required space in |buf| for the |niv|
 * entries is ``6*niv`` bytes and if the given buffer is too small, an
 * error is returned.  This function is used mainly for creating a
 * SETTINGS payload to be sent with the ``HTTP2-Settings`` header
 * field in an HTTP Upgrade request.  The data written in |buf| is NOT
 * base64url encoded and the application is responsible for encoding.
 *
 * This function returns the number of bytes written in |buf|, or one
 * of the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |iv| contains duplicate settings ID or invalid value.
 *
 * :enum:`NGHTTP2_ERR_INSUFF_BUFSIZE`
 *     The provided |buflen| size is too small to hold the output.
 */
NGHTTP2_EXTERN ssize_t nghttp2_pack_settings_payload(
    uint8_t *buf, size_t buflen, const nghttp2_settings_entry *iv, size_t niv);

/**
 * @function
 *
 * Returns string describing the |lib_error_code|.  The
 * |lib_error_code| must be one of the :enum:`nghttp2_error`.
 */
NGHTTP2_EXTERN const char *nghttp2_strerror(int lib_error_code);

/**
 * @function
 *
 * Returns string representation of HTTP/2 error code |error_code|
 * (e.g., ``PROTOCOL_ERROR`` is returned if ``error_code ==
 * NGHTTP2_PROTOCOL_ERROR``).  If string representation is unknown for
 * given |error_code|, this function returns string ``unknown``.
 */
NGHTTP2_EXTERN const char *nghttp2_http2_strerror(uint32_t error_code);

/**
 * @function
 *
 * Initializes |pri_spec| with the |stream_id| of the stream to depend
 * on with |weight| and its exclusive flag.  If |exclusive| is
 * nonzero, exclusive flag is set.
 *
 * The |weight| must be in [:enum:`NGHTTP2_MIN_WEIGHT`,
 * :enum:`NGHTTP2_MAX_WEIGHT`], inclusive.
 */
NGHTTP2_EXTERN void nghttp2_priority_spec_init(nghttp2_priority_spec *pri_spec,
                                               int32_t stream_id,
                                               int32_t weight, int exclusive);

/**
 * @function
 *
 * Initializes |pri_spec| with the default values.  The default values
 * are: stream_id = 0, weight = :macro:`NGHTTP2_DEFAULT_WEIGHT` and
 * exclusive = 0.
 */
NGHTTP2_EXTERN void
nghttp2_priority_spec_default_init(nghttp2_priority_spec *pri_spec);

/**
 * @function
 *
 * Returns nonzero if the |pri_spec| is filled with default values.
 */
NGHTTP2_EXTERN int
nghttp2_priority_spec_check_default(const nghttp2_priority_spec *pri_spec);

/**
 * @function
 *
 * Submits HEADERS frame and optionally one or more DATA frames.
 *
 * The |pri_spec| is priority specification of this request.  ``NULL``
 * means the default priority (see
 * `nghttp2_priority_spec_default_init()`).  To specify the priority,
 * use `nghttp2_priority_spec_init()`.  If |pri_spec| is not ``NULL``,
 * this function will copy its data members.
 *
 * The ``pri_spec->weight`` must be in [:enum:`NGHTTP2_MIN_WEIGHT`,
 * :enum:`NGHTTP2_MAX_WEIGHT`], inclusive.  If ``pri_spec->weight`` is
 * strictly less than :enum:`NGHTTP2_MIN_WEIGHT`, it becomes
 * :enum:`NGHTTP2_MIN_WEIGHT`.  If it is strictly greater than
 * :enum:`NGHTTP2_MAX_WEIGHT`, it becomes :enum:`NGHTTP2_MAX_WEIGHT`.
 *
 * The |nva| is an array of name/value pair :type:`nghttp2_nv` with
 * |nvlen| elements.  The application is responsible to include
 * required pseudo-header fields (header field whose name starts with
 * ":") in |nva| and must place pseudo-headers before regular header
 * fields.
 *
 * This function creates copies of all name/value pairs in |nva|.  It
 * also lower-cases all names in |nva|.  The order of elements in
 * |nva| is preserved.  For header fields with
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME` and
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_VALUE` are set, header field name
 * and value are not copied respectively.  With
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME`, application is responsible to
 * pass header field name in lowercase.  The application should
 * maintain the references to them until
 * :type:`nghttp2_on_frame_send_callback` or
 * :type:`nghttp2_on_frame_not_send_callback` is called.
 *
 * HTTP/2 specification has requirement about header fields in the
 * request HEADERS.  See the specification for more details.
 *
 * If |data_prd| is not ``NULL``, it provides data which will be sent
 * in subsequent DATA frames.  In this case, a method that allows
 * request message bodies
 * (https://tools.ietf.org/html/rfc7231#section-4) must be specified
 * with ``:method`` key in |nva| (e.g. ``POST``).  This function does
 * not take ownership of the |data_prd|.  The function copies the
 * members of the |data_prd|.  If |data_prd| is ``NULL``, HEADERS have
 * END_STREAM set.  The |stream_user_data| is data associated to the
 * stream opened by this request and can be an arbitrary pointer,
 * which can be retrieved later by
 * `nghttp2_session_get_stream_user_data()`.
 *
 * This function returns assigned stream ID if it succeeds, or one of
 * the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE`
 *     No stream ID is available because maximum stream ID was
 *     reached.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     Trying to depend on itself (new stream ID equals
 *     ``pri_spec->stream_id``).
 * :enum:`NGHTTP2_ERR_PROTO`
 *     The |session| is server session.
 *
 * .. warning::
 *
 *   This function returns assigned stream ID if it succeeds.  But
 *   that stream is not created yet.  The application must not submit
 *   frame to that stream ID before
 *   :type:`nghttp2_before_frame_send_callback` is called for this
 *   frame.  This means `nghttp2_session_get_stream_user_data()` does
 *   not work before the callback.  But
 *   `nghttp2_session_set_stream_user_data()` handles this situation
 *   specially, and it can set data to a stream during this period.
 *
 */
NGHTTP2_EXTERN int32_t nghttp2_submit_request(
    nghttp2_session *session, const nghttp2_priority_spec *pri_spec,
    const nghttp2_nv *nva, size_t nvlen, const nghttp2_data_provider *data_prd,
    void *stream_user_data);

/**
 * @function
 *
 * Submits response HEADERS frame and optionally one or more DATA
 * frames against the stream |stream_id|.
 *
 * The |nva| is an array of name/value pair :type:`nghttp2_nv` with
 * |nvlen| elements.  The application is responsible to include
 * required pseudo-header fields (header field whose name starts with
 * ":") in |nva| and must place pseudo-headers before regular header
 * fields.
 *
 * This function creates copies of all name/value pairs in |nva|.  It
 * also lower-cases all names in |nva|.  The order of elements in
 * |nva| is preserved.  For header fields with
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME` and
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_VALUE` are set, header field name
 * and value are not copied respectively.  With
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME`, application is responsible to
 * pass header field name in lowercase.  The application should
 * maintain the references to them until
 * :type:`nghttp2_on_frame_send_callback` or
 * :type:`nghttp2_on_frame_not_send_callback` is called.
 *
 * HTTP/2 specification has requirement about header fields in the
 * response HEADERS.  See the specification for more details.
 *
 * If |data_prd| is not ``NULL``, it provides data which will be sent
 * in subsequent DATA frames.  This function does not take ownership
 * of the |data_prd|.  The function copies the members of the
 * |data_prd|.  If |data_prd| is ``NULL``, HEADERS will have
 * END_STREAM flag set.
 *
 * This method can be used as normal HTTP response and push response.
 * When pushing a resource using this function, the |session| must be
 * configured using `nghttp2_session_server_new()` or its variants and
 * the target stream denoted by the |stream_id| must be reserved using
 * `nghttp2_submit_push_promise()`.
 *
 * To send non-final response headers (e.g., HTTP status 101), don't
 * use this function because this function half-closes the outbound
 * stream.  Instead, use `nghttp2_submit_headers()` for this purpose.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0.
 * :enum:`NGHTTP2_ERR_DATA_EXIST`
 *     DATA or HEADERS has been already submitted and not fully
 *     processed yet.  Normally, this does not happen, but when
 *     application wrongly calls `nghttp2_submit_response()` twice,
 *     this may happen.
 * :enum:`NGHTTP2_ERR_PROTO`
 *     The |session| is client session.
 *
 * .. warning::
 *
 *   Calling this function twice for the same stream ID may lead to
 *   program crash.  It is generally considered to a programming error
 *   to commit response twice.
 */
NGHTTP2_EXTERN int
nghttp2_submit_response(nghttp2_session *session, int32_t stream_id,
                        const nghttp2_nv *nva, size_t nvlen,
                        const nghttp2_data_provider *data_prd);

/**
 * @function
 *
 * Submits trailer fields HEADERS against the stream |stream_id|.
 *
 * The |nva| is an array of name/value pair :type:`nghttp2_nv` with
 * |nvlen| elements.  The application must not include pseudo-header
 * fields (headers whose names starts with ":") in |nva|.
 *
 * This function creates copies of all name/value pairs in |nva|.  It
 * also lower-cases all names in |nva|.  The order of elements in
 * |nva| is preserved.  For header fields with
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME` and
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_VALUE` are set, header field name
 * and value are not copied respectively.  With
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME`, application is responsible to
 * pass header field name in lowercase.  The application should
 * maintain the references to them until
 * :type:`nghttp2_on_frame_send_callback` or
 * :type:`nghttp2_on_frame_not_send_callback` is called.
 *
 * For server, trailer fields must follow response HEADERS or response
 * DATA without END_STREAM flat set.  The library does not enforce
 * this requirement, and applications should do this for themselves.
 * If `nghttp2_submit_trailer()` is called before any response HEADERS
 * submission (usually by `nghttp2_submit_response()`), the content of
 * |nva| will be sent as response headers, which will result in error.
 *
 * This function has the same effect with `nghttp2_submit_headers()`,
 * with flags = :enum:`NGHTTP2_FLAG_END_STREAM` and both pri_spec and
 * stream_user_data to NULL.
 *
 * To submit trailer fields after `nghttp2_submit_response()` is
 * called, the application has to specify
 * :type:`nghttp2_data_provider` to `nghttp2_submit_response()`.
 * Inside of :type:`nghttp2_data_source_read_callback`, when setting
 * :enum:`NGHTTP2_DATA_FLAG_EOF`, also set
 * :enum:`NGHTTP2_DATA_FLAG_NO_END_STREAM`.  After that, the
 * application can send trailer fields using
 * `nghttp2_submit_trailer()`.  `nghttp2_submit_trailer()` can be used
 * inside :type:`nghttp2_data_source_read_callback`.
 *
 * This function returns 0 if it succeeds and |stream_id| is -1.
 * Otherwise, this function returns 0 if it succeeds, or one of the
 * following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0.
 */
NGHTTP2_EXTERN int nghttp2_submit_trailer(nghttp2_session *session,
                                          int32_t stream_id,
                                          const nghttp2_nv *nva, size_t nvlen);

/**
 * @function
 *
 * Submits HEADERS frame. The |flags| is bitwise OR of the
 * following values:
 *
 * * :enum:`NGHTTP2_FLAG_END_STREAM`
 *
 * If |flags| includes :enum:`NGHTTP2_FLAG_END_STREAM`, this frame has
 * END_STREAM flag set.
 *
 * The library handles the CONTINUATION frame internally and it
 * correctly sets END_HEADERS to the last sequence of the PUSH_PROMISE
 * or CONTINUATION frame.
 *
 * If the |stream_id| is -1, this frame is assumed as request (i.e.,
 * request HEADERS frame which opens new stream).  In this case, the
 * assigned stream ID will be returned.  Otherwise, specify stream ID
 * in |stream_id|.
 *
 * The |pri_spec| is priority specification of this request.  ``NULL``
 * means the default priority (see
 * `nghttp2_priority_spec_default_init()`).  To specify the priority,
 * use `nghttp2_priority_spec_init()`.  If |pri_spec| is not ``NULL``,
 * this function will copy its data members.
 *
 * The ``pri_spec->weight`` must be in [:enum:`NGHTTP2_MIN_WEIGHT`,
 * :enum:`NGHTTP2_MAX_WEIGHT`], inclusive.  If ``pri_spec->weight`` is
 * strictly less than :enum:`NGHTTP2_MIN_WEIGHT`, it becomes
 * :enum:`NGHTTP2_MIN_WEIGHT`.  If it is strictly greater than
 * :enum:`NGHTTP2_MAX_WEIGHT`, it becomes :enum:`NGHTTP2_MAX_WEIGHT`.
 *
 * The |nva| is an array of name/value pair :type:`nghttp2_nv` with
 * |nvlen| elements.  The application is responsible to include
 * required pseudo-header fields (header field whose name starts with
 * ":") in |nva| and must place pseudo-headers before regular header
 * fields.
 *
 * This function creates copies of all name/value pairs in |nva|.  It
 * also lower-cases all names in |nva|.  The order of elements in
 * |nva| is preserved.  For header fields with
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME` and
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_VALUE` are set, header field name
 * and value are not copied respectively.  With
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME`, application is responsible to
 * pass header field name in lowercase.  The application should
 * maintain the references to them until
 * :type:`nghttp2_on_frame_send_callback` or
 * :type:`nghttp2_on_frame_not_send_callback` is called.
 *
 * The |stream_user_data| is a pointer to an arbitrary data which is
 * associated to the stream this frame will open.  Therefore it is
 * only used if this frame opens streams, in other words, it changes
 * stream state from idle or reserved to open.
 *
 * This function is low-level in a sense that the application code can
 * specify flags directly.  For usual HTTP request,
 * `nghttp2_submit_request()` is useful.  Likewise, for HTTP response,
 * prefer `nghttp2_submit_response()`.
 *
 * This function returns newly assigned stream ID if it succeeds and
 * |stream_id| is -1.  Otherwise, this function returns 0 if it
 * succeeds, or one of the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE`
 *     No stream ID is available because maximum stream ID was
 *     reached.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0; or trying to depend on itself (stream ID
 *     equals ``pri_spec->stream_id``).
 * :enum:`NGHTTP2_ERR_DATA_EXIST`
 *     DATA or HEADERS has been already submitted and not fully
 *     processed yet.  This happens if stream denoted by |stream_id|
 *     is in reserved state.
 * :enum:`NGHTTP2_ERR_PROTO`
 *     The |stream_id| is -1, and |session| is server session.
 *
 * .. warning::
 *
 *   This function returns assigned stream ID if it succeeds and
 *   |stream_id| is -1.  But that stream is not opened yet.  The
 *   application must not submit frame to that stream ID before
 *   :type:`nghttp2_before_frame_send_callback` is called for this
 *   frame.
 *
 */
NGHTTP2_EXTERN int32_t nghttp2_submit_headers(
    nghttp2_session *session, uint8_t flags, int32_t stream_id,
    const nghttp2_priority_spec *pri_spec, const nghttp2_nv *nva, size_t nvlen,
    void *stream_user_data);

/**
 * @function
 *
 * Submits one or more DATA frames to the stream |stream_id|.  The
 * data to be sent are provided by |data_prd|.  If |flags| contains
 * :enum:`NGHTTP2_FLAG_END_STREAM`, the last DATA frame has END_STREAM
 * flag set.
 *
 * This function does not take ownership of the |data_prd|.  The
 * function copies the members of the |data_prd|.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_DATA_EXIST`
 *     DATA or HEADERS has been already submitted and not fully
 *     processed yet.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0.
 * :enum:`NGHTTP2_ERR_STREAM_CLOSED`
 *     The stream was already closed; or the |stream_id| is invalid.
 *
 * .. note::
 *
 *   Currently, only one DATA or HEADERS is allowed for a stream at a
 *   time.  Submitting these frames more than once before first DATA
 *   or HEADERS is finished results in :enum:`NGHTTP2_ERR_DATA_EXIST`
 *   error code.  The earliest callback which tells that previous
 *   frame is done is :type:`nghttp2_on_frame_send_callback`.  In side
 *   that callback, new data can be submitted using
 *   `nghttp2_submit_data()`.  Of course, all data except for last one
 *   must not have :enum:`NGHTTP2_FLAG_END_STREAM` flag set in
 *   |flags|.  This sounds a bit complicated, and we recommend to use
 *   `nghttp2_submit_request()` and `nghttp2_submit_response()` to
 *   avoid this cascading issue.  The experience shows that for HTTP
 *   use, these two functions are enough to implement both client and
 *   server.
 */
NGHTTP2_EXTERN int nghttp2_submit_data(nghttp2_session *session, uint8_t flags,
                                       int32_t stream_id,
                                       const nghttp2_data_provider *data_prd);

/**
 * @function
 *
 * Submits PRIORITY frame to change the priority of stream |stream_id|
 * to the priority specification |pri_spec|.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * The |pri_spec| is priority specification of this request.  ``NULL``
 * is not allowed for this function. To specify the priority, use
 * `nghttp2_priority_spec_init()`.  This function will copy its data
 * members.
 *
 * The ``pri_spec->weight`` must be in [:enum:`NGHTTP2_MIN_WEIGHT`,
 * :enum:`NGHTTP2_MAX_WEIGHT`], inclusive.  If ``pri_spec->weight`` is
 * strictly less than :enum:`NGHTTP2_MIN_WEIGHT`, it becomes
 * :enum:`NGHTTP2_MIN_WEIGHT`.  If it is strictly greater than
 * :enum:`NGHTTP2_MAX_WEIGHT`, it becomes :enum:`NGHTTP2_MAX_WEIGHT`.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0; or the |pri_spec| is NULL; or trying to
 *     depend on itself.
 */
NGHTTP2_EXTERN int
nghttp2_submit_priority(nghttp2_session *session, uint8_t flags,
                        int32_t stream_id,
                        const nghttp2_priority_spec *pri_spec);

/**
 * @function
 *
 * Submits RST_STREAM frame to cancel/reject the stream |stream_id|
 * with the error code |error_code|.
 *
 * The pre-defined error code is one of :enum:`nghttp2_error_code`.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0.
 */
NGHTTP2_EXTERN int nghttp2_submit_rst_stream(nghttp2_session *session,
                                             uint8_t flags, int32_t stream_id,
                                             uint32_t error_code);

/**
 * @function
 *
 * Stores local settings and submits SETTINGS frame.  The |iv| is the
 * pointer to the array of :type:`nghttp2_settings_entry`.  The |niv|
 * indicates the number of :type:`nghttp2_settings_entry`.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * This function does not take ownership of the |iv|.  This function
 * copies all the elements in the |iv|.
 *
 * While updating individual stream's local window size, if the window
 * size becomes strictly larger than NGHTTP2_MAX_WINDOW_SIZE,
 * RST_STREAM is issued against such a stream.
 *
 * SETTINGS with :enum:`NGHTTP2_FLAG_ACK` is automatically submitted
 * by the library and application could not send it at its will.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |iv| contains invalid value (e.g., initial window size
 *     strictly greater than (1 << 31) - 1.
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_submit_settings(nghttp2_session *session,
                                           uint8_t flags,
                                           const nghttp2_settings_entry *iv,
                                           size_t niv);

/**
 * @function
 *
 * Submits PUSH_PROMISE frame.
 *
 * The |flags| is currently ignored.  The library handles the
 * CONTINUATION frame internally and it correctly sets END_HEADERS to
 * the last sequence of the PUSH_PROMISE or CONTINUATION frame.
 *
 * The |stream_id| must be client initiated stream ID.
 *
 * The |nva| is an array of name/value pair :type:`nghttp2_nv` with
 * |nvlen| elements.  The application is responsible to include
 * required pseudo-header fields (header field whose name starts with
 * ":") in |nva| and must place pseudo-headers before regular header
 * fields.
 *
 * This function creates copies of all name/value pairs in |nva|.  It
 * also lower-cases all names in |nva|.  The order of elements in
 * |nva| is preserved.  For header fields with
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME` and
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_VALUE` are set, header field name
 * and value are not copied respectively.  With
 * :enum:`NGHTTP2_NV_FLAG_NO_COPY_NAME`, application is responsible to
 * pass header field name in lowercase.  The application should
 * maintain the references to them until
 * :type:`nghttp2_on_frame_send_callback` or
 * :type:`nghttp2_on_frame_not_send_callback` is called.
 *
 * The |promised_stream_user_data| is a pointer to an arbitrary data
 * which is associated to the promised stream this frame will open and
 * make it in reserved state.  It is available using
 * `nghttp2_session_get_stream_user_data()`.  The application can
 * access it in :type:`nghttp2_before_frame_send_callback` and
 * :type:`nghttp2_on_frame_send_callback` of this frame.
 *
 * The client side is not allowed to use this function.
 *
 * To submit response headers and data, use
 * `nghttp2_submit_response()`.
 *
 * This function returns assigned promised stream ID if it succeeds,
 * or one of the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_PROTO`
 *     This function was invoked when |session| is initialized as
 *     client.
 * :enum:`NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE`
 *     No stream ID is available because maximum stream ID was
 *     reached.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is 0; The |stream_id| does not designate stream
 *     that peer initiated.
 * :enum:`NGHTTP2_ERR_STREAM_CLOSED`
 *     The stream was already closed; or the |stream_id| is invalid.
 *
 * .. warning::
 *
 *   This function returns assigned promised stream ID if it succeeds.
 *   As of 1.16.0, stream object for pushed resource is created when
 *   this function succeeds.  In that case, the application can submit
 *   push response for the promised frame.
 *
 *   In 1.15.0 or prior versions, pushed stream is not opened yet when
 *   this function succeeds.  The application must not submit frame to
 *   that stream ID before :type:`nghttp2_before_frame_send_callback`
 *   is called for this frame.
 *
 */
NGHTTP2_EXTERN int32_t nghttp2_submit_push_promise(
    nghttp2_session *session, uint8_t flags, int32_t stream_id,
    const nghttp2_nv *nva, size_t nvlen, void *promised_stream_user_data);

/**
 * @function
 *
 * Submits PING frame.  You don't have to send PING back when you
 * received PING frame.  The library automatically submits PING frame
 * in this case.
 *
 * The |flags| is bitwise OR of 0 or more of the following value.
 *
 * * :enum:`NGHTTP2_FLAG_ACK`
 *
 * Unless `nghttp2_option_set_no_auto_ping_ack()` is used, the |flags|
 * should be :enum:`NGHTTP2_FLAG_NONE`.
 *
 * If the |opaque_data| is non ``NULL``, then it should point to the 8
 * bytes array of memory to specify opaque data to send with PING
 * frame.  If the |opaque_data| is ``NULL``, zero-cleared 8 bytes will
 * be sent as opaque data.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_submit_ping(nghttp2_session *session, uint8_t flags,
                                       const uint8_t *opaque_data);

/**
 * @function
 *
 * Submits GOAWAY frame with the last stream ID |last_stream_id| and
 * the error code |error_code|.
 *
 * The pre-defined error code is one of :enum:`nghttp2_error_code`.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * The |last_stream_id| is peer's stream ID or 0.  So if |session| is
 * initialized as client, |last_stream_id| must be even or 0.  If
 * |session| is initialized as server, |last_stream_id| must be odd or
 * 0.
 *
 * The HTTP/2 specification says last_stream_id must not be increased
 * from the value previously sent.  So the actual value sent as
 * last_stream_id is the minimum value between the given
 * |last_stream_id| and the last_stream_id previously sent to the
 * peer.
 *
 * If the |opaque_data| is not ``NULL`` and |opaque_data_len| is not
 * zero, those data will be sent as additional debug data.  The
 * library makes a copy of the memory region pointed by |opaque_data|
 * with the length |opaque_data_len|, so the caller does not need to
 * keep this memory after the return of this function.  If the
 * |opaque_data_len| is 0, the |opaque_data| could be ``NULL``.
 *
 * After successful transmission of GOAWAY, following things happen.
 * All incoming streams having strictly more than |last_stream_id| are
 * closed.  All incoming HEADERS which starts new stream are simply
 * ignored.  After all active streams are handled, both
 * `nghttp2_session_want_read()` and `nghttp2_session_want_write()`
 * return 0 and the application can close session.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |opaque_data_len| is too large; the |last_stream_id| is
 *     invalid.
 */
NGHTTP2_EXTERN int nghttp2_submit_goaway(nghttp2_session *session,
                                         uint8_t flags, int32_t last_stream_id,
                                         uint32_t error_code,
                                         const uint8_t *opaque_data,
                                         size_t opaque_data_len);

/**
 * @function
 *
 * Returns the last stream ID of a stream for which
 * :type:`nghttp2_on_frame_recv_callback` was invoked most recently.
 * The returned value can be used as last_stream_id parameter for
 * `nghttp2_submit_goaway()` and
 * `nghttp2_session_terminate_session2()`.
 *
 * This function always succeeds.
 */
NGHTTP2_EXTERN int32_t
nghttp2_session_get_last_proc_stream_id(nghttp2_session *session);

/**
 * @function
 *
 * Returns nonzero if new request can be sent from local endpoint.
 *
 * This function return 0 if request is not allowed for this session.
 * There are several reasons why request is not allowed.  Some of the
 * reasons are: session is server; stream ID has been spent; GOAWAY
 * has been sent or received.
 *
 * The application can call `nghttp2_submit_request()` without
 * consulting this function.  In that case, `nghttp2_submit_request()`
 * may return error.  Or, request is failed to sent, and
 * :type:`nghttp2_on_stream_close_callback` is called.
 */
NGHTTP2_EXTERN int
nghttp2_session_check_request_allowed(nghttp2_session *session);

/**
 * @function
 *
 * Returns nonzero if |session| is initialized as server side session.
 */
NGHTTP2_EXTERN int
nghttp2_session_check_server_session(nghttp2_session *session);

/**
 * @function
 *
 * Submits WINDOW_UPDATE frame.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * The |stream_id| is the stream ID to send this WINDOW_UPDATE.  To
 * send connection level WINDOW_UPDATE, specify 0 to |stream_id|.
 *
 * If the |window_size_increment| is positive, the WINDOW_UPDATE with
 * that value as window_size_increment is queued.  If the
 * |window_size_increment| is larger than the received bytes from the
 * remote endpoint, the local window size is increased by that
 * difference.  If the sole purpose is to increase the local window
 * size, consider to use `nghttp2_session_set_local_window_size()`.
 *
 * If the |window_size_increment| is negative, the local window size
 * is decreased by -|window_size_increment|.  If automatic
 * WINDOW_UPDATE is enabled
 * (`nghttp2_option_set_no_auto_window_update()`), and the library
 * decided that the WINDOW_UPDATE should be submitted, then
 * WINDOW_UPDATE is queued with the current received bytes count.  If
 * the sole purpose is to decrease the local window size, consider to
 * use `nghttp2_session_set_local_window_size()`.
 *
 * If the |window_size_increment| is 0, the function does nothing and
 * returns 0.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_FLOW_CONTROL`
 *     The local window size overflow or gets negative.
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_submit_window_update(nghttp2_session *session,
                                                uint8_t flags,
                                                int32_t stream_id,
                                                int32_t window_size_increment);

/**
 * @function
 *
 * Set local window size (local endpoints's window size) to the given
 * |window_size| for the given stream denoted by |stream_id|.  To
 * change connection level window size, specify 0 to |stream_id|.  To
 * increase window size, this function may submit WINDOW_UPDATE frame
 * to transmission queue.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * This sounds similar to `nghttp2_submit_window_update()`, but there
 * are 2 differences.  The first difference is that this function
 * takes the absolute value of window size to set, rather than the
 * delta.  To change the window size, this may be easier to use since
 * the application just declares the intended window size, rather than
 * calculating delta.  The second difference is that
 * `nghttp2_submit_window_update()` affects the received bytes count
 * which has not acked yet.  By the specification of
 * `nghttp2_submit_window_update()`, to strictly increase the local
 * window size, we have to submit delta including all received bytes
 * count, which might not be desirable in some cases.  On the other
 * hand, this function does not affect the received bytes count.  It
 * just sets the local window size to the given value.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The |stream_id| is negative.
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int
nghttp2_session_set_local_window_size(nghttp2_session *session, uint8_t flags,
                                      int32_t stream_id, int32_t window_size);

/**
 * @function
 *
 * Submits extension frame.
 *
 * Application can pass arbitrary frame flags and stream ID in |flags|
 * and |stream_id| respectively.  The |payload| is opaque pointer, and
 * it can be accessible though ``frame->ext.payload`` in
 * :type:`nghttp2_pack_extension_callback`.  The library will not own
 * passed |payload| pointer.
 *
 * The application must set :type:`nghttp2_pack_extension_callback`
 * using `nghttp2_session_callbacks_set_pack_extension_callback()`.
 *
 * The application should retain the memory pointed by |payload| until
 * the transmission of extension frame is done (which is indicated by
 * :type:`nghttp2_on_frame_send_callback`), or transmission fails
 * (which is indicated by :type:`nghttp2_on_frame_not_send_callback`).
 * If application does not touch this memory region after packing it
 * into a wire format, application can free it inside
 * :type:`nghttp2_pack_extension_callback`.
 *
 * The standard HTTP/2 frame cannot be sent with this function, so
 * |type| must be strictly grater than 0x9.  Otherwise, this function
 * will fail with error code :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     If :type:`nghttp2_pack_extension_callback` is not set.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     If  |type| specifies  standard  HTTP/2 frame  type.  The  frame
 *     types  in the  rage [0x0,  0x9], both  inclusive, are  standard
 *     HTTP/2 frame type, and cannot be sent using this function.
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory
 */
NGHTTP2_EXTERN int nghttp2_submit_extension(nghttp2_session *session,
                                            uint8_t type, uint8_t flags,
                                            int32_t stream_id, void *payload);

/**
 * @struct
 *
 * The payload of ALTSVC frame.  ALTSVC frame is a non-critical
 * extension to HTTP/2.  If this frame is received, and
 * `nghttp2_option_set_user_recv_extension_type()` is not set, and
 * `nghttp2_option_set_builtin_recv_extension_type()` is set for
 * :enum:`NGHTTP2_ALTSVC`, ``nghttp2_extension.payload`` will point to
 * this struct.
 *
 * It has the following members:
 */
typedef struct {
  /**
   * The pointer to origin which this alternative service is
   * associated with.  This is not necessarily NULL-terminated.
   */
  uint8_t *origin;
  /**
   * The length of the |origin|.
   */
  size_t origin_len;
  /**
   * The pointer to Alt-Svc field value contained in ALTSVC frame.
   * This is not necessarily NULL-terminated.
   */
  uint8_t *field_value;
  /**
   * The length of the |field_value|.
   */
  size_t field_value_len;
} nghttp2_ext_altsvc;

/**
 * @function
 *
 * Submits ALTSVC frame.
 *
 * ALTSVC frame is a non-critical extension to HTTP/2, and defined in
 * `RFC 7383 <https://tools.ietf.org/html/rfc7838#section-4>`_.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * The |origin| points to the origin this alternative service is
 * associated with.  The |origin_len| is the length of the origin.  If
 * |stream_id| is 0, the origin must be specified.  If |stream_id| is
 * not zero, the origin must be empty (in other words, |origin_len|
 * must be 0).
 *
 * The ALTSVC frame is only usable from server side.  If this function
 * is invoked with client side session, this function returns
 * :enum:`NGHTTP2_ERR_INVALID_STATE`.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     The function is called from client side session
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     The sum of |origin_len| and |field_value_len| is larger than
 *     16382; or |origin_len| is 0 while |stream_id| is 0; or
 *     |origin_len| is not 0 while |stream_id| is not 0.
 */
NGHTTP2_EXTERN int nghttp2_submit_altsvc(nghttp2_session *session,
                                         uint8_t flags, int32_t stream_id,
                                         const uint8_t *origin,
                                         size_t origin_len,
                                         const uint8_t *field_value,
                                         size_t field_value_len);

/**
 * @struct
 *
 * The single entry of an origin.
 */
typedef struct {
  /**
   * The pointer to origin.  No validation is made against this field
   * by the library.  This is not necessarily NULL-terminated.
   */
  uint8_t *origin;
  /**
   * The length of the |origin|.
   */
  size_t origin_len;
} nghttp2_origin_entry;

/**
 * @struct
 *
 * The payload of ORIGIN frame.  ORIGIN frame is a non-critical
 * extension to HTTP/2 and defined by `RFC 8336
 * <https://tools.ietf.org/html/rfc8336>`_.
 *
 * If this frame is received, and
 * `nghttp2_option_set_user_recv_extension_type()` is not set, and
 * `nghttp2_option_set_builtin_recv_extension_type()` is set for
 * :enum:`NGHTTP2_ORIGIN`, ``nghttp2_extension.payload`` will point to
 * this struct.
 *
 * It has the following members:
 */
typedef struct {
  /**
   * The number of origins contained in |ov|.
   */
  size_t nov;
  /**
   * The pointer to the array of origins contained in ORIGIN frame.
   */
  nghttp2_origin_entry *ov;
} nghttp2_ext_origin;

/**
 * @function
 *
 * Submits ORIGIN frame.
 *
 * ORIGIN frame is a non-critical extension to HTTP/2 and defined by
 * `RFC 8336 <https://tools.ietf.org/html/rfc8336>`_.
 *
 * The |flags| is currently ignored and should be
 * :enum:`NGHTTP2_FLAG_NONE`.
 *
 * The |ov| points to the array of origins.  The |nov| specifies the
 * number of origins included in |ov|.  This function creates copies
 * of all elements in |ov|.
 *
 * The ORIGIN frame is only usable by a server.  If this function is
 * invoked with client side session, this function returns
 * :enum:`NGHTTP2_ERR_INVALID_STATE`.
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     The function is called from client side session.
 * :enum:`NGHTTP2_ERR_INVALID_ARGUMENT`
 *     There are too many origins, or an origin is too large to fit
 *     into a default frame payload.
 */
NGHTTP2_EXTERN int nghttp2_submit_origin(nghttp2_session *session,
                                         uint8_t flags,
                                         const nghttp2_origin_entry *ov,
                                         size_t nov);

/**
 * @function
 *
 * Compares ``lhs->name`` of length ``lhs->namelen`` bytes and
 * ``rhs->name`` of length ``rhs->namelen`` bytes.  Returns negative
 * integer if ``lhs->name`` is found to be less than ``rhs->name``; or
 * returns positive integer if ``lhs->name`` is found to be greater
 * than ``rhs->name``; or returns 0 otherwise.
 */
NGHTTP2_EXTERN int nghttp2_nv_compare_name(const nghttp2_nv *lhs,
                                           const nghttp2_nv *rhs);

/**
 * @function
 *
 * A helper function for dealing with NPN in client side or ALPN in
 * server side.  The |in| contains peer's protocol list in preferable
 * order.  The format of |in| is length-prefixed and not
 * null-terminated.  For example, ``h2`` and
 * ``http/1.1`` stored in |in| like this::
 *
 *     in[0] = 2
 *     in[1..2] = "h2"
 *     in[3] = 8
 *     in[4..11] = "http/1.1"
 *     inlen = 12
 *
 * The selection algorithm is as follows:
 *
 * 1. If peer's list contains HTTP/2 protocol the library supports,
 *    it is selected and returns 1. The following step is not taken.
 *
 * 2. If peer's list contains ``http/1.1``, this function selects
 *    ``http/1.1`` and returns 0.  The following step is not taken.
 *
 * 3. This function selects nothing and returns -1 (So called
 *    non-overlap case).  In this case, |out| and |outlen| are left
 *    untouched.
 *
 * Selecting ``h2`` means that ``h2`` is written into |*out| and its
 * length (which is 2) is assigned to |*outlen|.
 *
 * For ALPN, refer to https://tools.ietf.org/html/rfc7301
 *
 * See http://technotes.googlecode.com/git/nextprotoneg.html for more
 * details about NPN.
 *
 * For NPN, to use this method you should do something like::
 *
 *     static int select_next_proto_cb(SSL* ssl,
 *                                     unsigned char **out,
 *                                     unsigned char *outlen,
 *                                     const unsigned char *in,
 *                                     unsigned int inlen,
 *                                     void *arg)
 *     {
 *         int rv;
 *         rv = nghttp2_select_next_protocol(out, outlen, in, inlen);
 *         if (rv == -1) {
 *             return SSL_TLSEXT_ERR_NOACK;
 *         }
 *         if (rv == 1) {
 *             ((MyType*)arg)->http2_selected = 1;
 *         }
 *         return SSL_TLSEXT_ERR_OK;
 *     }
 *     ...
 *     SSL_CTX_set_next_proto_select_cb(ssl_ctx, select_next_proto_cb, my_obj);
 *
 */
NGHTTP2_EXTERN int nghttp2_select_next_protocol(unsigned char **out,
                                                unsigned char *outlen,
                                                const unsigned char *in,
                                                unsigned int inlen);

/**
 * @function
 *
 * Returns a pointer to a nghttp2_info struct with version information
 * about the run-time library in use.  The |least_version| argument
 * can be set to a 24 bit numerical value for the least accepted
 * version number and if the condition is not met, this function will
 * return a ``NULL``.  Pass in 0 to skip the version checking.
 */
NGHTTP2_EXTERN nghttp2_info *nghttp2_version(int least_version);

/**
 * @function
 *
 * Returns nonzero if the :type:`nghttp2_error` library error code
 * |lib_error| is fatal.
 */
NGHTTP2_EXTERN int nghttp2_is_fatal(int lib_error_code);

/**
 * @function
 *
 * Returns nonzero if HTTP header field name |name| of length |len| is
 * valid according to http://tools.ietf.org/html/rfc7230#section-3.2
 *
 * Because this is a header field name in HTTP2, the upper cased alphabet
 * is treated as error.
 */
NGHTTP2_EXTERN int nghttp2_check_header_name(const uint8_t *name, size_t len);

/**
 * @function
 *
 * Returns nonzero if HTTP header field value |value| of length |len|
 * is valid according to
 * http://tools.ietf.org/html/rfc7230#section-3.2
 */
NGHTTP2_EXTERN int nghttp2_check_header_value(const uint8_t *value, size_t len);

/* HPACK API */

struct nghttp2_hd_deflater;

/**
 * @struct
 *
 * HPACK deflater object.
 */
typedef struct nghttp2_hd_deflater nghttp2_hd_deflater;

/**
 * @function
 *
 * Initializes |*deflater_ptr| for deflating name/values pairs.
 *
 * The |max_deflate_dynamic_table_size| is the upper bound of header
 * table size the deflater will use.
 *
 * If this function fails, |*deflater_ptr| is left untouched.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int
nghttp2_hd_deflate_new(nghttp2_hd_deflater **deflater_ptr,
                       size_t max_deflate_dynamic_table_size);

/**
 * @function
 *
 * Like `nghttp2_hd_deflate_new()`, but with additional custom memory
 * allocator specified in the |mem|.
 *
 * The |mem| can be ``NULL`` and the call is equivalent to
 * `nghttp2_hd_deflate_new()`.
 *
 * This function does not take ownership |mem|.  The application is
 * responsible for freeing |mem|.
 *
 * The library code does not refer to |mem| pointer after this
 * function returns, so the application can safely free it.
 */
NGHTTP2_EXTERN int
nghttp2_hd_deflate_new2(nghttp2_hd_deflater **deflater_ptr,
                        size_t max_deflate_dynamic_table_size,
                        nghttp2_mem *mem);

/**
 * @function
 *
 * Deallocates any resources allocated for |deflater|.
 */
NGHTTP2_EXTERN void nghttp2_hd_deflate_del(nghttp2_hd_deflater *deflater);

/**
 * @function
 *
 * Changes header table size of the |deflater| to
 * |settings_max_dynamic_table_size| bytes.  This may trigger eviction
 * in the dynamic table.
 *
 * The |settings_max_dynamic_table_size| should be the value received
 * in SETTINGS_HEADER_TABLE_SIZE.
 *
 * The deflater never uses more memory than
 * ``max_deflate_dynamic_table_size`` bytes specified in
 * `nghttp2_hd_deflate_new()`.  Therefore, if
 * |settings_max_dynamic_table_size| >
 * ``max_deflate_dynamic_table_size``, resulting maximum table size
 * becomes ``max_deflate_dynamic_table_size``.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int
nghttp2_hd_deflate_change_table_size(nghttp2_hd_deflater *deflater,
                                     size_t settings_max_dynamic_table_size);

/**
 * @function
 *
 * Deflates the |nva|, which has the |nvlen| name/value pairs, into
 * the |buf| of length |buflen|.
 *
 * If |buf| is not large enough to store the deflated header block,
 * this function fails with :enum:`NGHTTP2_ERR_INSUFF_BUFSIZE`.  The
 * caller should use `nghttp2_hd_deflate_bound()` to know the upper
 * bound of buffer size required to deflate given header name/value
 * pairs.
 *
 * Once this function fails, subsequent call of this function always
 * returns :enum:`NGHTTP2_ERR_HEADER_COMP`.
 *
 * After this function returns, it is safe to delete the |nva|.
 *
 * This function returns the number of bytes written to |buf| if it
 * succeeds, or one of the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_HEADER_COMP`
 *     Deflation process has failed.
 * :enum:`NGHTTP2_ERR_INSUFF_BUFSIZE`
 *     The provided |buflen| size is too small to hold the output.
 */
NGHTTP2_EXTERN ssize_t nghttp2_hd_deflate_hd(nghttp2_hd_deflater *deflater,
                                             uint8_t *buf, size_t buflen,
                                             const nghttp2_nv *nva,
                                             size_t nvlen);

/**
 * @function
 *
 * Deflates the |nva|, which has the |nvlen| name/value pairs, into
 * the |veclen| size of buf vector |vec|.  The each size of buffer
 * must be set in len field of :type:`nghttp2_vec`.  If and only if
 * one chunk is filled up completely, next chunk will be used.  If
 * |vec| is not large enough to store the deflated header block, this
 * function fails with :enum:`NGHTTP2_ERR_INSUFF_BUFSIZE`.  The caller
 * should use `nghttp2_hd_deflate_bound()` to know the upper bound of
 * buffer size required to deflate given header name/value pairs.
 *
 * Once this function fails, subsequent call of this function always
 * returns :enum:`NGHTTP2_ERR_HEADER_COMP`.
 *
 * After this function returns, it is safe to delete the |nva|.
 *
 * This function returns the number of bytes written to |vec| if it
 * succeeds, or one of the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_HEADER_COMP`
 *     Deflation process has failed.
 * :enum:`NGHTTP2_ERR_INSUFF_BUFSIZE`
 *     The provided |buflen| size is too small to hold the output.
 */
NGHTTP2_EXTERN ssize_t nghttp2_hd_deflate_hd_vec(nghttp2_hd_deflater *deflater,
                                                 const nghttp2_vec *vec,
                                                 size_t veclen,
                                                 const nghttp2_nv *nva,
                                                 size_t nvlen);

/**
 * @function
 *
 * Returns an upper bound on the compressed size after deflation of
 * |nva| of length |nvlen|.
 */
NGHTTP2_EXTERN size_t nghttp2_hd_deflate_bound(nghttp2_hd_deflater *deflater,
                                               const nghttp2_nv *nva,
                                               size_t nvlen);

/**
 * @function
 *
 * Returns the number of entries that header table of |deflater|
 * contains.  This is the sum of the number of static table and
 * dynamic table, so the return value is at least 61.
 */
NGHTTP2_EXTERN
size_t nghttp2_hd_deflate_get_num_table_entries(nghttp2_hd_deflater *deflater);

/**
 * @function
 *
 * Returns the table entry denoted by |idx| from header table of
 * |deflater|.  The |idx| is 1-based, and idx=1 returns first entry of
 * static table.  idx=62 returns first entry of dynamic table if it
 * exists.  Specifying idx=0 is error, and this function returns NULL.
 * If |idx| is strictly greater than the number of entries the tables
 * contain, this function returns NULL.
 */
NGHTTP2_EXTERN
const nghttp2_nv *
nghttp2_hd_deflate_get_table_entry(nghttp2_hd_deflater *deflater, size_t idx);

/**
 * @function
 *
 * Returns the used dynamic table size, including the overhead 32
 * bytes per entry described in RFC 7541.
 */
NGHTTP2_EXTERN
size_t nghttp2_hd_deflate_get_dynamic_table_size(nghttp2_hd_deflater *deflater);

/**
 * @function
 *
 * Returns the maximum dynamic table size.
 */
NGHTTP2_EXTERN
size_t
nghttp2_hd_deflate_get_max_dynamic_table_size(nghttp2_hd_deflater *deflater);

struct nghttp2_hd_inflater;

/**
 * @struct
 *
 * HPACK inflater object.
 */
typedef struct nghttp2_hd_inflater nghttp2_hd_inflater;

/**
 * @function
 *
 * Initializes |*inflater_ptr| for inflating name/values pairs.
 *
 * If this function fails, |*inflater_ptr| is left untouched.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 */
NGHTTP2_EXTERN int nghttp2_hd_inflate_new(nghttp2_hd_inflater **inflater_ptr);

/**
 * @function
 *
 * Like `nghttp2_hd_inflate_new()`, but with additional custom memory
 * allocator specified in the |mem|.
 *
 * The |mem| can be ``NULL`` and the call is equivalent to
 * `nghttp2_hd_inflate_new()`.
 *
 * This function does not take ownership |mem|.  The application is
 * responsible for freeing |mem|.
 *
 * The library code does not refer to |mem| pointer after this
 * function returns, so the application can safely free it.
 */
NGHTTP2_EXTERN int nghttp2_hd_inflate_new2(nghttp2_hd_inflater **inflater_ptr,
                                           nghttp2_mem *mem);

/**
 * @function
 *
 * Deallocates any resources allocated for |inflater|.
 */
NGHTTP2_EXTERN void nghttp2_hd_inflate_del(nghttp2_hd_inflater *inflater);

/**
 * @function
 *
 * Changes header table size in the |inflater|.  This may trigger
 * eviction in the dynamic table.
 *
 * The |settings_max_dynamic_table_size| should be the value
 * transmitted in SETTINGS_HEADER_TABLE_SIZE.
 *
 * This function must not be called while header block is being
 * inflated.  In other words, this function must be called after
 * initialization of |inflater|, but before calling
 * `nghttp2_hd_inflate_hd2()`, or after
 * `nghttp2_hd_inflate_end_headers()`.  Otherwise,
 * `NGHTTP2_ERR_INVALID_STATE` was returned.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_INVALID_STATE`
 *     The function is called while header block is being inflated.
 *     Probably, application missed to call
 *     `nghttp2_hd_inflate_end_headers()`.
 */
NGHTTP2_EXTERN int
nghttp2_hd_inflate_change_table_size(nghttp2_hd_inflater *inflater,
                                     size_t settings_max_dynamic_table_size);

/**
 * @enum
 *
 * The flags for header inflation.
 */
typedef enum {
  /**
   * No flag set.
   */
  NGHTTP2_HD_INFLATE_NONE = 0,
  /**
   * Indicates all headers were inflated.
   */
  NGHTTP2_HD_INFLATE_FINAL = 0x01,
  /**
   * Indicates a header was emitted.
   */
  NGHTTP2_HD_INFLATE_EMIT = 0x02
} nghttp2_hd_inflate_flag;

/**
 * @function
 *
 * .. warning::
 *
 *   Deprecated.  Use `nghttp2_hd_inflate_hd2()` instead.
 *
 * Inflates name/value block stored in |in| with length |inlen|.  This
 * function performs decompression.  For each successful emission of
 * header name/value pair, :enum:`NGHTTP2_HD_INFLATE_EMIT` is set in
 * |*inflate_flags| and name/value pair is assigned to the |nv_out|
 * and the function returns.  The caller must not free the members of
 * |nv_out|.
 *
 * The |nv_out| may include pointers to the memory region in the |in|.
 * The caller must retain the |in| while the |nv_out| is used.
 *
 * The application should call this function repeatedly until the
 * ``(*inflate_flags) & NGHTTP2_HD_INFLATE_FINAL`` is nonzero and
 * return value is non-negative.  This means the all input values are
 * processed successfully.  Then the application must call
 * `nghttp2_hd_inflate_end_headers()` to prepare for the next header
 * block input.
 *
 * The caller can feed complete compressed header block.  It also can
 * feed it in several chunks.  The caller must set |in_final| to
 * nonzero if the given input is the last block of the compressed
 * header.
 *
 * This function returns the number of bytes processed if it succeeds,
 * or one of the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_HEADER_COMP`
 *     Inflation process has failed.
 * :enum:`NGHTTP2_ERR_BUFFER_ERROR`
 *     The header field name or value is too large.
 *
 * Example follows::
 *
 *     int inflate_header_block(nghttp2_hd_inflater *hd_inflater,
 *                              uint8_t *in, size_t inlen, int final)
 *     {
 *         ssize_t rv;
 *
 *         for(;;) {
 *             nghttp2_nv nv;
 *             int inflate_flags = 0;
 *
 *             rv = nghttp2_hd_inflate_hd(hd_inflater, &nv, &inflate_flags,
 *                                        in, inlen, final);
 *
 *             if(rv < 0) {
 *                 fprintf(stderr, "inflate failed with error code %zd", rv);
 *                 return -1;
 *             }
 *
 *             in += rv;
 *             inlen -= rv;
 *
 *             if(inflate_flags & NGHTTP2_HD_INFLATE_EMIT) {
 *                 fwrite(nv.name, nv.namelen, 1, stderr);
 *                 fprintf(stderr, ": ");
 *                 fwrite(nv.value, nv.valuelen, 1, stderr);
 *                 fprintf(stderr, "\n");
 *             }
 *             if(inflate_flags & NGHTTP2_HD_INFLATE_FINAL) {
 *                 nghttp2_hd_inflate_end_headers(hd_inflater);
 *                 break;
 *             }
 *             if((inflate_flags & NGHTTP2_HD_INFLATE_EMIT) == 0 &&
 *                inlen == 0) {
 *                break;
 *             }
 *         }
 *
 *         return 0;
 *     }
 *
 */
NGHTTP2_EXTERN ssize_t nghttp2_hd_inflate_hd(nghttp2_hd_inflater *inflater,
                                             nghttp2_nv *nv_out,
                                             int *inflate_flags, uint8_t *in,
                                             size_t inlen, int in_final);

/**
 * @function
 *
 * Inflates name/value block stored in |in| with length |inlen|.  This
 * function performs decompression.  For each successful emission of
 * header name/value pair, :enum:`NGHTTP2_HD_INFLATE_EMIT` is set in
 * |*inflate_flags| and name/value pair is assigned to the |nv_out|
 * and the function returns.  The caller must not free the members of
 * |nv_out|.
 *
 * The |nv_out| may include pointers to the memory region in the |in|.
 * The caller must retain the |in| while the |nv_out| is used.
 *
 * The application should call this function repeatedly until the
 * ``(*inflate_flags) & NGHTTP2_HD_INFLATE_FINAL`` is nonzero and
 * return value is non-negative.  If that happens, all given input
 * data (|inlen| bytes) are processed successfully.  Then the
 * application must call `nghttp2_hd_inflate_end_headers()` to prepare
 * for the next header block input.
 *
 * In other words, if |in_final| is nonzero, and this function returns
 * |inlen|, you can assert that :enum:`NGHTTP2_HD_INFLATE_FINAL` is
 * set in |*inflate_flags|.
 *
 * The caller can feed complete compressed header block.  It also can
 * feed it in several chunks.  The caller must set |in_final| to
 * nonzero if the given input is the last block of the compressed
 * header.
 *
 * This function returns the number of bytes processed if it succeeds,
 * or one of the following negative error codes:
 *
 * :enum:`NGHTTP2_ERR_NOMEM`
 *     Out of memory.
 * :enum:`NGHTTP2_ERR_HEADER_COMP`
 *     Inflation process has failed.
 * :enum:`NGHTTP2_ERR_BUFFER_ERROR`
 *     The header field name or value is too large.
 *
 * Example follows::
 *
 *     int inflate_header_block(nghttp2_hd_inflater *hd_inflater,
 *                              uint8_t *in, size_t inlen, int final)
 *     {
 *         ssize_t rv;
 *
 *         for(;;) {
 *             nghttp2_nv nv;
 *             int inflate_flags = 0;
 *
 *             rv = nghttp2_hd_inflate_hd2(hd_inflater, &nv, &inflate_flags,
 *                                         in, inlen, final);
 *
 *             if(rv < 0) {
 *                 fprintf(stderr, "inflate failed with error code %zd", rv);
 *                 return -1;
 *             }
 *
 *             in += rv;
 *             inlen -= rv;
 *
 *             if(inflate_flags & NGHTTP2_HD_INFLATE_EMIT) {
 *                 fwrite(nv.name, nv.namelen, 1, stderr);
 *                 fprintf(stderr, ": ");
 *                 fwrite(nv.value, nv.valuelen, 1, stderr);
 *                 fprintf(stderr, "\n");
 *             }
 *             if(inflate_flags & NGHTTP2_HD_INFLATE_FINAL) {
 *                 nghttp2_hd_inflate_end_headers(hd_inflater);
 *                 break;
 *             }
 *             if((inflate_flags & NGHTTP2_HD_INFLATE_EMIT) == 0 &&
 *                inlen == 0) {
 *                break;
 *             }
 *         }
 *
 *         return 0;
 *     }
 *
 */
NGHTTP2_EXTERN ssize_t nghttp2_hd_inflate_hd2(nghttp2_hd_inflater *inflater,
                                              nghttp2_nv *nv_out,
                                              int *inflate_flags,
                                              const uint8_t *in, size_t inlen,
                                              int in_final);

/**
 * @function
 *
 * Signals the end of decompression for one header block.
 *
 * This function returns 0 if it succeeds. Currently this function
 * always succeeds.
 */
NGHTTP2_EXTERN int
nghttp2_hd_inflate_end_headers(nghttp2_hd_inflater *inflater);

/**
 * @function
 *
 * Returns the number of entries that header table of |inflater|
 * contains.  This is the sum of the number of static table and
 * dynamic table, so the return value is at least 61.
 */
NGHTTP2_EXTERN
size_t nghttp2_hd_inflate_get_num_table_entries(nghttp2_hd_inflater *inflater);

/**
 * @function
 *
 * Returns the table entry denoted by |idx| from header table of
 * |inflater|.  The |idx| is 1-based, and idx=1 returns first entry of
 * static table.  idx=62 returns first entry of dynamic table if it
 * exists.  Specifying idx=0 is error, and this function returns NULL.
 * If |idx| is strictly greater than the number of entries the tables
 * contain, this function returns NULL.
 */
NGHTTP2_EXTERN
const nghttp2_nv *
nghttp2_hd_inflate_get_table_entry(nghttp2_hd_inflater *inflater, size_t idx);

/**
 * @function
 *
 * Returns the used dynamic table size, including the overhead 32
 * bytes per entry described in RFC 7541.
 */
NGHTTP2_EXTERN
size_t nghttp2_hd_inflate_get_dynamic_table_size(nghttp2_hd_inflater *inflater);

/**
 * @function
 *
 * Returns the maximum dynamic table size.
 */
NGHTTP2_EXTERN
size_t
nghttp2_hd_inflate_get_max_dynamic_table_size(nghttp2_hd_inflater *inflater);

struct nghttp2_stream;

/**
 * @struct
 *
 * The structure to represent HTTP/2 stream.  The details of this
 * structure are intentionally hidden from the public API.
 */
typedef struct nghttp2_stream nghttp2_stream;

/**
 * @function
 *
 * Returns pointer to :type:`nghttp2_stream` object denoted by
 * |stream_id|.  If stream was not found, returns NULL.
 *
 * Returns imaginary root stream (see
 * `nghttp2_session_get_root_stream()`) if 0 is given in |stream_id|.
 *
 * Unless |stream_id| == 0, the returned pointer is valid until next
 * call of `nghttp2_session_send()`, `nghttp2_session_mem_send()`,
 * `nghttp2_session_recv()`, and `nghttp2_session_mem_recv()`.
 */
NGHTTP2_EXTERN nghttp2_stream *
nghttp2_session_find_stream(nghttp2_session *session, int32_t stream_id);

/**
 * @enum
 *
 * State of stream as described in RFC 7540.
 */
typedef enum {
  /**
   * idle state.
   */
  NGHTTP2_STREAM_STATE_IDLE = 1,
  /**
   * open state.
   */
  NGHTTP2_STREAM_STATE_OPEN,
  /**
   * reserved (local) state.
   */
  NGHTTP2_STREAM_STATE_RESERVED_LOCAL,
  /**
   * reserved (remote) state.
   */
  NGHTTP2_STREAM_STATE_RESERVED_REMOTE,
  /**
   * half closed (local) state.
   */
  NGHTTP2_STREAM_STATE_HALF_CLOSED_LOCAL,
  /**
   * half closed (remote) state.
   */
  NGHTTP2_STREAM_STATE_HALF_CLOSED_REMOTE,
  /**
   * closed state.
   */
  NGHTTP2_STREAM_STATE_CLOSED
} nghttp2_stream_proto_state;

/**
 * @function
 *
 * Returns state of |stream|.  The root stream retrieved by
 * `nghttp2_session_get_root_stream()` will have stream state
 * :enum:`NGHTTP2_STREAM_STATE_IDLE`.
 */
NGHTTP2_EXTERN nghttp2_stream_proto_state
nghttp2_stream_get_state(nghttp2_stream *stream);

/**
 * @function
 *
 * Returns root of dependency tree, which is imaginary stream with
 * stream ID 0.  The returned pointer is valid until |session| is
 * freed by `nghttp2_session_del()`.
 */
NGHTTP2_EXTERN nghttp2_stream *
nghttp2_session_get_root_stream(nghttp2_session *session);

/**
 * @function
 *
 * Returns the parent stream of |stream| in dependency tree.  Returns
 * NULL if there is no such stream.
 */
NGHTTP2_EXTERN nghttp2_stream *
nghttp2_stream_get_parent(nghttp2_stream *stream);

NGHTTP2_EXTERN int32_t nghttp2_stream_get_stream_id(nghttp2_stream *stream);

/**
 * @function
 *
 * Returns the next sibling stream of |stream| in dependency tree.
 * Returns NULL if there is no such stream.
 */
NGHTTP2_EXTERN nghttp2_stream *
nghttp2_stream_get_next_sibling(nghttp2_stream *stream);

/**
 * @function
 *
 * Returns the previous sibling stream of |stream| in dependency tree.
 * Returns NULL if there is no such stream.
 */
NGHTTP2_EXTERN nghttp2_stream *
nghttp2_stream_get_previous_sibling(nghttp2_stream *stream);

/**
 * @function
 *
 * Returns the first child stream of |stream| in dependency tree.
 * Returns NULL if there is no such stream.
 */
NGHTTP2_EXTERN nghttp2_stream *
nghttp2_stream_get_first_child(nghttp2_stream *stream);

/**
 * @function
 *
 * Returns dependency weight to the parent stream of |stream|.
 */
NGHTTP2_EXTERN int32_t nghttp2_stream_get_weight(nghttp2_stream *stream);

/**
 * @function
 *
 * Returns the sum of the weight for |stream|'s children.
 */
NGHTTP2_EXTERN int32_t
nghttp2_stream_get_sum_dependency_weight(nghttp2_stream *stream);

/**
 * @functypedef
 *
 * Callback function invoked when the library outputs debug logging.
 * The function is called with arguments suitable for ``vfprintf(3)``
 *
 * The debug output is only enabled if the library is built with
 * ``DEBUGBUILD`` macro defined.
 */
typedef void (*nghttp2_debug_vprintf_callback)(const char *format,
                                               va_list args);

/**
 * @function
 *
 * Sets a debug output callback called by the library when built with
 * ``DEBUGBUILD`` macro defined.  If this option is not used, debug
 * log is written into standard error output.
 *
 * For builds without ``DEBUGBUILD`` macro defined, this function is
 * noop.
 *
 * Note that building with ``DEBUGBUILD`` may cause significant
 * performance penalty to libnghttp2 because of extra processing.  It
 * should be used for debugging purpose only.
 *
 * .. Warning::
 *
 *   Building with ``DEBUGBUILD`` may cause significant performance
 *   penalty to libnghttp2 because of extra processing.  It should be
 *   used for debugging purpose only.  We write this two times because
 *   this is important.
 */
NGHTTP2_EXTERN void nghttp2_set_debug_vprintf_callback(
    nghttp2_debug_vprintf_callback debug_vprintf_callback);

#ifdef __cplusplus
}
#endif

#endif /* NGHTTP2_H */