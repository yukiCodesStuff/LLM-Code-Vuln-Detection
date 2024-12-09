
    # Upload the +io+ to the +key+ specified to all services. If a +checksum+ is provided, all services will
    # ensure a match when the upload has completed or raise an ActiveStorage::IntegrityError.
    def upload(key, io, checksum: nil, **options)
      each_service.collect do |service|
        service.upload key, io.tap(&:rewind), checksum: checksum, **options
      end
    end

    # Delete the file at the +key+ on all services.