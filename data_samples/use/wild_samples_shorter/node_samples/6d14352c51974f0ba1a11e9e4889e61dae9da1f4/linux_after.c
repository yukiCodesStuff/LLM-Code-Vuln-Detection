}


UV_EXTERN int uv__node_patch_is_using_io_uring(void) {
  // This function exists only in the modified copy of libuv in the Node.js
  // repository. Node.js checks if this function exists and, if it does, uses it
  // to determine whether libuv is using io_uring or not.
  return uv__use_io_uring();
}


static void uv__iou_init(int epollfd,
                         struct uv__iou* iou,
                         uint32_t entries,
                         uint32_t flags) {