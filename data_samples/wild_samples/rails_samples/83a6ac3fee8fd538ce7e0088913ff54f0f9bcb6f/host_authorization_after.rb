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
        valid_host = /
          \A
          (?<host>[a-z0-9.-]+|\[[a-f0-9]*:[a-f0-9\.:]+\])
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
      end

      def mark_as_authorized(request)
        request.set_header("action_dispatch.authorized_host", request.host)
      end
  end
end