  use = atomic_load_explicit(&use_io_uring, memory_order_relaxed);

  if (use == 0) {
    /* Older kernels have a bug where the sqpoll thread uses 100% CPU. */
    use = uv__kernel_version() >= /* 5.10.186 */ 0x050ABA ? 1 : -1;

    /* But users can still enable it if they so desire. */
    val = getenv("UV_USE_IO_URING");
    if (val != NULL)