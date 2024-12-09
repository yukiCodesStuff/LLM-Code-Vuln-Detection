      @root = root
    end

    def upload(key, io, checksum: nil)
      instrument :upload, key: key, checksum: checksum do
        IO.copy_stream(io, make_path_for(key))
        ensure_integrity_of(key, checksum) if checksum
      end

    def url(key, expires_in:, filename:, disposition:, content_type:)
      instrument :url, key: key do |payload|
        verified_key_with_expiration = ActiveStorage.verifier.generate(key, expires_in: expires_in, purpose: :blob_key)

        generated_url =
          url_helpers.rails_disk_service_url(
            verified_key_with_expiration,
            host: current_host,
            filename: filename,
            disposition: content_disposition_with(type: disposition, filename: filename),
            content_type: content_type
          )

        payload[:url] = generated_url

        generated_url
      end