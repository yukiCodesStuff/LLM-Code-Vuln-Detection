    end

    private
      def authorized?(request)
        valid_host = /
          \A
          (?<host>[a-z0-9.-]+|\[[a-f0-9]*:[a-f0-9.:]+\])
          (:\d+)?
          \z
        /x

        origin_host = valid_host.match(
          request.get_header("HTTP_HOST").to_s.downcase)
        forwarded_host = valid_host.match(
          request.x_forwarded_host.to_s.split(/,\s?/).last)

        origin_host && @permissions.allows?(origin_host[:host]) && (
          forwarded_host.nil? || @permissions.allows?(forwarded_host[:host]))
      end

      def excluded?(request)
        @exclude && @exclude.call(request)