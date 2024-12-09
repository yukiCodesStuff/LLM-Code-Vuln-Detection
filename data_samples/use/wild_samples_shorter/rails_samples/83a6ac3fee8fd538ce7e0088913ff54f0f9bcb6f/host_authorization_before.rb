
    private
      def authorized?(request)
        origin_host = request.get_header("HTTP_HOST").to_s.sub(/:\d+\z/, "")
        forwarded_host = request.x_forwarded_host.to_s.split(/,\s?/).last.to_s.sub(/:\d+\z/, "")

        @permissions.allows?(origin_host) &&
          (forwarded_host.blank? || @permissions.allows?(forwarded_host))
      end

      def excluded?(request)
        @exclude && @exclude.call(request)