
      def _url_host_allowed?(url)
        host = URI(url.to_s).host
        host == request.host || host.nil? && url.to_s.start_with?("/")
      rescue ArgumentError, URI::Error
        false
      end
  end