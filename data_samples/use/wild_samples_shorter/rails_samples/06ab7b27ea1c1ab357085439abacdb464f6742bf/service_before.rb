
    # Upload the +io+ to the +key+ specified. If a +checksum+ is provided, the service will
    # ensure a match when the upload has completed or raise an ActiveStorage::IntegrityError.
    def upload(key, io, checksum: nil)
      raise NotImplementedError
    end

    # Return the content of the file at the +key+.
    def download(key)
      raise NotImplementedError
    end