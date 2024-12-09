  if (use == 0) {
    /* Disable io_uring by default due to CVE-2024-22017. */
    use = -1;

    /* But users can still enable it if they so desire. */
    val = getenv("UV_USE_IO_URING");
    if (val != NULL)
      use = atoi(val) ? 1 : -1;

    atomic_store_explicit(&use_io_uring, use, memory_order_relaxed);
  }

  return use > 0;
#endif
}


static void uv__iou_init(int epollfd,
                         struct uv__iou* iou,
                         uint32_t entries,
                         uint32_t flags) {
  struct uv__io_uring_params params;
  struct epoll_event e;
  size_t cqlen;
  size_t sqlen;
  size_t maxlen;
  size_t sqelen;
  uint32_t i;
  char* sq;
  char* sqe;
  int ringfd;

  sq = MAP_FAILED;
  sqe = MAP_FAILED;

  if (!uv__use_io_uring())
    return;

  /* SQPOLL required CAP_SYS_NICE until linux v5.12 relaxed that requirement.
   * Mostly academic because we check for a v5.13 kernel afterwards anyway.
   */
  memset(&params, 0, sizeof(params));
  params.flags = flags;

  if (flags & UV__IORING_SETUP_SQPOLL)
    params.sq_thread_idle = 10;  /* milliseconds */

  /* Kernel returns a file descriptor with O_CLOEXEC flag set. */
  ringfd = uv__io_uring_setup(entries, &params);
  if (ringfd == -1)
    return;

  /* IORING_FEAT_RSRC_TAGS is used to detect linux v5.13 but what we're
   * actually detecting is whether IORING_OP_STATX works with SQPOLL.
   */
  if (!(params.features & UV__IORING_FEAT_RSRC_TAGS))
    goto fail;

  /* Implied by IORING_FEAT_RSRC_TAGS but checked explicitly anyway. */
  if (!(params.features & UV__IORING_FEAT_SINGLE_MMAP))
    goto fail;

  /* Implied by IORING_FEAT_RSRC_TAGS but checked explicitly anyway. */
  if (!(params.features & UV__IORING_FEAT_NODROP))
    goto fail;

  sqlen = params.sq_off.array + params.sq_entries * sizeof(uint32_t);
  cqlen =
      params.cq_off.cqes + params.cq_entries * sizeof(struct uv__io_uring_cqe);
  maxlen = sqlen < cqlen ? cqlen : sqlen;
  sqelen = params.sq_entries * sizeof(struct uv__io_uring_sqe);

  sq = mmap(0,
            maxlen,
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_POPULATE,
            ringfd,
            0);  /* IORING_OFF_SQ_RING */

  sqe = mmap(0,
             sqelen,
             PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_POPULATE,
             ringfd,
             0x10000000ull);  /* IORING_OFF_SQES */

  if (sq == MAP_FAILED || sqe == MAP_FAILED)
    goto fail;

  if (flags & UV__IORING_SETUP_SQPOLL) {
    /* Only interested in completion events. To get notified when
     * the kernel pulls items from the submission ring, add POLLOUT.
     */
    memset(&e, 0, sizeof(e));
    e.events = POLLIN;
    e.data.fd = ringfd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, ringfd, &e))
      goto fail;
  }

  iou->sqhead = (uint32_t*) (sq + params.sq_off.head);
  iou->sqtail = (uint32_t*) (sq + params.sq_off.tail);
  iou->sqmask = *(uint32_t*) (sq + params.sq_off.ring_mask);
  iou->sqarray = (uint32_t*) (sq + params.sq_off.array);
  iou->sqflags = (uint32_t*) (sq + params.sq_off.flags);
  iou->cqhead = (uint32_t*) (sq + params.cq_off.head);
  iou->cqtail = (uint32_t*) (sq + params.cq_off.tail);
  iou->cqmask = *(uint32_t*) (sq + params.cq_off.ring_mask);
  iou->sq = sq;
  iou->cqe = sq + params.cq_off.cqes;
  iou->sqe = sqe;
  iou->sqlen = sqlen;
  iou->cqlen = cqlen;
  iou->maxlen = maxlen;
  iou->sqelen = sqelen;
  iou->ringfd = ringfd;
  iou->in_flight = 0;
  iou->flags = 0;

  if (uv__kernel_version() >= /* 5.15.0 */ 0x050F00)
    iou->flags |= UV__MKDIRAT_SYMLINKAT_LINKAT;

  for (i = 0; i <= iou->sqmask; i++)
    iou->sqarray[i] = i;  /* Slot -> sqe identity mapping. */

  return;

fail:
  if (sq != MAP_FAILED)
    munmap(sq, maxlen);

  if (sqe != MAP_FAILED)
    munmap(sqe, sqelen);

  uv__close(ringfd);
}