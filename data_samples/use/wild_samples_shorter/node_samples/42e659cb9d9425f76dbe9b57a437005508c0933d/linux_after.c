  use = atomic_load_explicit(&use_io_uring, memory_order_relaxed);

  if (use == 0) {
    /* Disable io_uring by default due to CVE-2024-22017. */
    use = -1;

    /* But users can still enable it if they so desire. */
    val = getenv("UV_USE_IO_URING");
    if (val != NULL)