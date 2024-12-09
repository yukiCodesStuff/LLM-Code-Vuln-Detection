
    class UnsafeRedirectError < StandardError; end

    included do
      mattr_accessor :raise_on_open_redirects, default: false
    end

      allow_other_host = response_options.delete(:allow_other_host) { _allow_other_host }

      self.status        = _extract_redirect_to_status(options, response_options)
      self.location      = _enforce_open_redirect_protection(_compute_redirect_to_location(request, options), allow_other_host: allow_other_host)
      self.response_body = ""
    end

    # Soft deprecated alias for #redirect_back_or_to where the +fallback_location+ location is supplied as a keyword argument instead
      rescue ArgumentError, URI::Error
        false
      end
  end
end