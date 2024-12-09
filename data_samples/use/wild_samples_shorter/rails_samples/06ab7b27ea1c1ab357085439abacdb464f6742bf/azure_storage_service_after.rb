      @container = container
    end

    def upload(key, io, checksum: nil, **)
      instrument :upload, key: key, checksum: checksum do
        handle_errors do
          blobs.create_block_blob(container, key, IO.try_convert(io) || io, content_md5: checksum)
        end