  def service_url(expires_in: ActiveStorage.service_urls_expire_in, disposition: :inline, filename: nil, **options)
    filename = ActiveStorage::Filename.wrap(filename || self.filename)

    service.url key, expires_in: expires_in, filename: filename, content_type: content_type,
      disposition: forcibly_serve_as_binary? ? :attachment : disposition, **options
  end

  # Returns a URL that can be used to directly upload a file for this blob on the service. This URL is intended to be
  # short-lived for security and only generated on-demand by the client-side JavaScript responsible for doing the uploading.
  end

  def upload_without_unfurling(io) #:nodoc:
    service.upload key, io, checksum: checksum
  end

  # Downloads the file associated with this blob. If no block is given, the entire file is read into memory and returned.
  # That'll use a lot of RAM for very large files. If a block is given, then the download is streamed and yielded in chunks.
      ActiveStorage.content_types_to_serve_as_binary.include?(content_type)
    end

    ActiveSupport.run_load_hooks(:active_storage_blob, self)
end