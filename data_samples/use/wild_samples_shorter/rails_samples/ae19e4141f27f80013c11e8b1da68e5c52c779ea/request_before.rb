    end

    def forgery_whitelisted?
      get? || xhr? || content_mime_type.nil? || !content_mime_type.verify_request?
    end

    def media_type
      content_mime_type.to_s
    end