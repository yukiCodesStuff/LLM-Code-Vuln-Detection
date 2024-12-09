
    // Write string
    offset = ROUND_UP(offset, WriteWrap::kAlignSize);
    CHECK_LE(offset, storage_size);
    char* str_storage = req_wrap->Extra(offset);
    size_t str_size = storage_size - offset;

    Local<String> string = chunk->ToString(env->isolate());