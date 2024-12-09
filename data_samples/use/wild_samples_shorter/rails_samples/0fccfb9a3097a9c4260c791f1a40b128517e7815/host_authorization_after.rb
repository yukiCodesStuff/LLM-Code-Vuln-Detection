
        def sanitize_string(host)
          if host.start_with?(".")
            /\A([a-z0-9-]+\.)?#{Regexp.escape(host[1..-1])}\z/i
          else
            /\A#{Regexp.escape host}\z/i
          end
        end
    end

    private
      def authorized?(request)
        origin_host = request.get_header("HTTP_HOST")
        forwarded_host = request.x_forwarded_host&.split(/,\s?/)&.last

        @permissions.allows?(origin_host) && (forwarded_host.blank? || @permissions.allows?(forwarded_host))
      end
