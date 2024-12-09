      [403, {
        "Content-Type" => "#{format}; charset=#{Response.default_charset}",
        "Content-Length" => body.bytesize.to_s,
      }, [body]]
    end

    def initialize(app, hosts, deprecated_response_app = nil, exclude: nil, response_app: nil)
      @app = app
      @permissions = Permissions.new(hosts)
      @exclude = exclude

      unless deprecated_response_app.nil?
        ActiveSupport::Deprecation.warn(<<-MSG.squish)
          `action_dispatch.hosts_response_app` is deprecated and will be ignored in Rails 7.0.
          Use the Host Authorization `response_app` setting instead.
        MSG

        response_app ||= deprecated_response_app
      end

      @response_app = response_app || DEFAULT_RESPONSE_APP
    end

    def call(env)
      return @app.call(env) if @permissions.empty?

      request = Request.new(env)

      if authorized?(request) || excluded?(request)
        mark_as_authorized(request)
        @app.call(env)
      else
        @response_app.call(env)
      end
    end

    private
      def authorized?(request)
        origin_host = request.get_header("HTTP_HOST").to_s.sub(/:\d+\z/, "")
        forwarded_host = request.x_forwarded_host.to_s.split(/,\s?/).last.to_s.sub(/:\d+\z/, "")

        @permissions.allows?(origin_host) &&
          (forwarded_host.blank? || @permissions.allows?(forwarded_host))
      end

      def excluded?(request)
        @exclude && @exclude.call(request)
      end

      def mark_as_authorized(request)
        request.set_header("action_dispatch.authorized_host", request.host)
      end
  end
end