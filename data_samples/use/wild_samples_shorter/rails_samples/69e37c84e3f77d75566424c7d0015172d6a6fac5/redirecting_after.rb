
    class UnsafeRedirectError < StandardError; end

    ILLEGAL_HEADER_VALUE_REGEX = /[\x00-\x08\x0A-\x1F]/.freeze

    included do
      mattr_accessor :raise_on_open_redirects, default: false
    end

      allow_other_host = response_options.delete(:allow_other_host) { _allow_other_host }

      self.status        = _extract_redirect_to_status(options, response_options)

      redirect_to_location = _compute_redirect_to_location(request, options)
      _ensure_url_is_http_header_safe(redirect_to_location)

      self.location      = _enforce_open_redirect_protection(redirect_to_location, allow_other_host: allow_other_host)
      self.response_body = ""
    end

    # Soft deprecated alias for #redirect_back_or_to where the +fallback_location+ location is supplied as a keyword argument instead
      rescue ArgumentError, URI::Error
        false
      end

      def _ensure_url_is_http_header_safe(url)
        # Attempt to comply with the set of valid token characters
        # defined for an HTTP header value in
        # https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6
        if url.match(ILLEGAL_HEADER_VALUE_REGEX)
          msg = "The redirect URL #{url} contains one or more illegal HTTP header field character. " \
            "Set of legal characters defined in https://datatracker.ietf.org/doc/html/rfc7230#section-3.2.6"
          raise UnsafeRedirectError, msg
        end
      end
  end
end