static int uv__use_io_uring(void) {
#if defined(__ANDROID_API__)
  return 0;  /* Possibly available but blocked by seccomp. */
#elif defined(__arm__) && __SIZEOF_POINTER__ == 4
  /* See https://github.com/libuv/libuv/issues/4158. */
  return 0;  /* All 32 bits kernels appear buggy. */
#else
  /* Ternary: unknown=0, yes=1, no=-1 */
  static _Atomic int use_io_uring;
  char* val;
  int use;

  use = atomic_load_explicit(&use_io_uring, memory_order_relaxed);

  if (use == 0) {
    /* Older kernels have a bug where the sqpoll thread uses 100% CPU. */
    use = uv__kernel_version() >= /* 5.10.186 */ 0x050ABA ? 1 : -1;

    /* But users can still enable it if they so desire. */
    val = getenv("UV_USE_IO_URING");
    if (val != NULL)
      use = atoi(val) ? 1 : -1;

    atomic_store_explicit(&use_io_uring, use, memory_order_relaxed);
  }