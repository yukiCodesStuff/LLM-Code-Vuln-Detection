      @config = config
    end

    def upload(key, io, checksum: nil, content_type: nil, disposition: nil, filename: nil)
      instrument :upload, key: key, checksum: checksum do
        begin
          # GCS's signed URLs don't include params such as response-content-type response-content_disposition
          # in the signature, which means an attacker can modify them and bypass our effort to force these to
          # binary and attachment when the file's content type requires it. The only way to force them is to
          # store them as object's metadata.
          content_disposition = content_disposition_with(type: disposition, filename: filename) if disposition && filename
          bucket.create_file(io, key, md5: checksum, content_type: content_type, content_disposition: content_disposition)
        rescue Google::Cloud::InvalidArgumentError
          raise ActiveStorage::IntegrityError
        end
      end
      end
    end

    def update_metadata(key, content_type:, disposition: nil, filename: nil)
      instrument :update_metadata, key: key, content_type: content_type, disposition: disposition do
        file_for(key).update do |file|
          file.content_type = content_type
          file.content_disposition = content_disposition_with(type: disposition, filename: filename) if disposition && filename
        end
      end
    end

    def download_chunk(key, range)
      instrument :download_chunk, key: key, range: range do
        begin
          file_for(key).download(range: range).string