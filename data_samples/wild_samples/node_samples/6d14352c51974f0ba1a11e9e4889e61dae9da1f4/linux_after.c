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


UV_EXTERN int uv__node_patch_is_using_io_uring(void) {
  // This function exists only in the modified copy of libuv in the Node.js
  // repository. Node.js checks if this function exists and, if it does, uses it
  // to determine whether libuv is using io_uring or not.
  return uv__use_io_uring();
}