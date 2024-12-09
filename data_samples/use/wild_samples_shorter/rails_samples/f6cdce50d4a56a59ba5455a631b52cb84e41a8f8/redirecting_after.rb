
      def _url_host_allowed?(url)
        host = URI(url.to_s).host

        return true if host == request.host
        return false unless host.nil?
        return false unless url.to_s.start_with?("/")
        return !url.to_s.start_with?("//")
      rescue ArgumentError, URI::Error
        false
      end
  end