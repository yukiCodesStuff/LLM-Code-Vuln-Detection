    end

    def forgery_whitelisted?
      get?
    end
    deprecate :forgery_whitelisted? => "it is just an alias for 'get?' now, update your code"

    def media_type
      content_mime_type.to_s
    end