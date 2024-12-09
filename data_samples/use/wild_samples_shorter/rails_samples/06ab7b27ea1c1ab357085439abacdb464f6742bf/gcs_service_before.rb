      @config = config
    end

    def upload(key, io, checksum: nil)
      instrument :upload, key: key, checksum: checksum do
        begin
          # The official GCS client library doesn't allow us to create a file with no Content-Type metadata.
          # We need the file we create to have no Content-Type so we can control it via the response-content-type
          # param in signed URLs. Workaround: let the GCS client create the file with an inferred
          # Content-Type (usually "application/octet-stream") then clear it.
          bucket.create_file(io, key, md5: checksum).update do |file|
            file.content_type = nil
          end
        rescue Google::Cloud::InvalidArgumentError
          raise ActiveStorage::IntegrityError
        end
      end
      end
    end

    def download_chunk(key, range)
      instrument :download_chunk, key: key, range: range do
        begin
          file_for(key).download(range: range).string